#include "zdmprivate.h"
#include <QDateTime>
#include <QStringList>
#include <QUuid>
#include <QEventLoop>
#include <QFileInfo>
#include <QTimer>
#include <QDomDocument>
#include <QDomElement>
#include <QTextStream>

#include "zdmhttp.h"
#include <curl/curl.h>
#include "zlog.h"

ZDMTask::ZDMTask(ZDM *dm) :
    ZDownloadTask(dm),
    isWriting(false){
    DBG("+ ZDMTask %p\n", this);
    status = STAT_PENDING;
    result = ERR_NOERROR;
    mtime = QDateTime::currentDateTime().toTime_t();
    progress = 0;
    size = 0;

    this->dm = dm;
    needsStop = false;
    nextStatus = STAT_PENDING;
    lastProgress = 0;

    threadPool = new ZDMThreadPool(this);
    file = NULL;

    connect(&speed_timer, SIGNAL(timeout()), SLOT(slot_calcSpeed()));
    connect(this, SIGNAL(signal_startThreads()), SLOT(slot_startThreads()));
    connect(this, SIGNAL(signal_stopSpeedTimer(qint64,int)), SLOT(slot_stopSpeedTimer(qint64,int)));
}

ZDMTask::~ZDMTask() {
    while(!parts.isEmpty()) {
        delete parts.takeFirst();
    }
    DBG("- ZDMTask %p\n", this);
}

bool ZDMTask::initParts() {
    // if failed last time, clear all progress
    if(status == ZDMTask::STAT_FAILED) {
        progress = 0;
        lastProgress = 0;
        foreach(ZDMPart *part, parts) {
            part->progress = 0;
        }
    }

    // fetch remote size
    char *uri = strdup(url.toEncoded().data());
    DBG("uri='%s'\n", uri);

    qint64 new_size = -1;
    long http_resp = -1;
    int r = ZDMHttp::getContentLength(uri, http_resp, new_size);
    free(uri);

    DBG("getContentLength http_code: %d, content_length: %d\n", http_resp, new_size);
    if(r != CURLE_OK) {
        result = ZDMTask::ERR_CONNECT;
        DBG("curl fetch size error, code %d\n", r);
        return false;
    }
    if(new_size != size) {
        DBG("size mismatch, re-split parts\n");
        progress = 0;
        lastProgress = 0;
        while(!parts.isEmpty()) {
            delete parts.takeFirst();
        }
    }
    size = new_size;

    // split parts
    if(parts.isEmpty()) {
        if(size < MIN_PART_BLKSIZE) {
            DBG("now left is less than MIN_PART_BLKSIZE!\n");
            ZDMPart *part = new ZDMPart();
            parts.append(part);

            part->begin = 0;
            part->end = size - 1; // always mark to -1 for single thread, no range needed
            part->progress = 0;
            DBG("creating part %lld -> %lld\n", part->begin, part->end);
        } else {
            DBG("now left is more than MIN_PART_BLKSIZE!\n");
            qint64 pos = 0;
            qint64 part_size = size/dm->maxParts -1;
            uint padding = size % dm->maxParts;

            while(pos < size) {
                ZDMPart *part = new ZDMPart();
                parts.append(part);

                part->begin = pos;
                part->progress = 0;
                if(pos == 0) {
                    pos += padding;
                    part->end = part_size + padding;
                } else {
                    part->end = pos + part_size;
                }
                DBG("creating part %lld -> %lld\n", part->begin, part->end);
                pos += part_size;
                pos ++;
            }
        }
    }
    return true;
}

bool ZDMTask::initFile() {
    QString tmpName = path + ".dm!";
    QFileInfo finfo(tmpName);
    if(finfo.exists() && finfo.size() == size) {
        DBG("%p using exist file\n", this);
        file = new QFile(tmpName);
        if(!file->open(QIODevice::ReadWrite)) {
            result = ZDMTask::ERR_WRITE_FILE;
            delete file;
            file = NULL;
            return false;
        }
    } else {
        DBG("%p create new file\n", this);
        QFile::remove(tmpName);
        progress = 0;
        lastProgress = 0;
        DBG("foreach(ZDMPart *part, parts)\n");
        foreach(ZDMPart *part, parts) {
            part->progress = 0;
        }

        file = new QFile(tmpName);
        DBG("if(file->open(QIODevice::WriteOnly))\n");
        if(file->open(QIODevice::WriteOnly)) {
            this->sub_status = ZDownloadTask::SUB_CREATFILE;
            DBG("eimit emit signal_status\n");
            emit signal_status(this);
            qint64 left = size;
            char dummy[4096] = {0};
            int n;
            while(left > 0) {
                n = file->write(dummy, left > sizeof(dummy) ? sizeof(dummy) : left);
                if(n < 0) {
                    break;
                }

                file->flush();
                left -= n;
            }
            if(left > 0) {
                result = ZDMTask::ERR_WRITE_FILE;
                file->close();
                delete file;
                file = NULL;
                return false;
            }
            this->sub_status = ZDownloadTask::SUB_NONE;
            DBG("eimit emit signal_status\n");
            emit signal_status(this);
        } else {
            result = ZDMTask::ERR_CREATE_FILE;
            delete file;
            file = NULL;
            return false;
        }
    }
    return true;
}

qint64 ZDMTask::writeFile(qint64 pos, const QByteArray& buffer) {
    if(file == NULL) {
        result = ZDMTask::ERR_WRITE_FILE;
        return -1;
    }
    DBG("write at %lld, size %d\n", pos, buffer.size());

    qint64 cur = 0;
    bool success = false;

    file_mutex.lock();
#ifdef WRITE_LOG
    //增加一个LOG文件保存一下任务的属性
    QFile f("ZDM_LOG/" + this->id);
    if(f.open(QIODevice::WriteOnly)) {
        QByteArray out;
        out.append("id:" + this->id.toUtf8()).append("\r\n");
        out.append("path:" + this->path.toUtf8()).append("\r\n");
        out.append("stat:" + QByteArray::number(this->status)).append("\r\n");
        out.append("sub stat:" + QByteArray::number(this->sub_status)).append("\r\n");
        out.append("progress:" + QByteArray::number(this->progress)).append("\r\n");
        f.write(out);
        f.close();
    }
#endif
    if((success = file->seek(pos)) == true) {
        success = buffer.size() == file->write(buffer);
    }
    file_mutex.unlock();

    if(!success) {
        result = ZDMTask::ERR_WRITE_FILE;
    }
    return cur;
}

void ZDMTask::addProgress(qint64 n) {
    progress_mutex.lock();
    progress += n;
    int elapsed = progress_timer.elapsed();
    if(elapsed > 1000) {
        emit signal_progress(this);
        emit signal_saveConfig();
        progress_timer.restart();
    }
    progress_mutex.unlock();
}

void ZDMTask::slot_startThreads() {
    lastProgress = progress;
    speed_timer.start(1000);

    progress_timer.restart();
    DBG("slot_startThreads for %d parts\n", parts.size());
    ZDMPartThread *t = new ZDMPartThread(this, dm, this, parts);
    t->start();
}

void ZDMTask::slot_calcSpeed() {
    emit signal_speed(this, progress - lastProgress);
    lastProgress = progress;
}

void ZDMTask::slot_stopSpeedTimer(qint64 gap, int elapsed) {
    speed_timer.stop();

    double average_speed = gap/(elapsed/1000.0f);
    emit signal_speed(this, (int)average_speed);
}

void ZDMTask::run() {
    bool success = false;
    status = ZDMTask::STAT_DOWNLOADING;
    DBG("eimit emit signal_status\n");
    emit signal_status(this);

    qint64 start_progress = progress;
    QTime start_time;
    start_time.start();
    DBG("this is a rum in ZDMTask\n");
    do {
        if(!initParts()) {
            break;
        }
        if(!initFile()) {
            break;
        }
        DBG("init finished!\n");
        DBG("after init\n");
        emit signal_startThreads();

        DBG("start to Loop!\n");
        QEventLoop e;
        connect(threadPool, SIGNAL(signalEmpty()), &e, SLOT(quit()));
        e.exec();
        DBG("threadpool is [%d] count\n", threadPool->threadCount());
        DBG("Loop end!\n");
        DBG("progress %lld / %lld\n", progress, size);
        success = size > 0 && progress >= size;
    } while(0);
    emit signal_stopSpeedTimer(progress - start_progress, start_time.elapsed());
    if(needsStop) {
        status = nextStatus;
    } else if(dm->needsStop) {
        status = ZDMTask::STAT_PENDING;
    } else {
        if(success) {
            status = ZDMTask::STAT_FINISHED;
        } else {
            status = ZDMTask::STAT_FAILED;
        }
    }

    if(file != NULL) {
        file->close();
        if(success) {
            QFile::remove(path);
            file->rename(path);
            mtime = QFileInfo(path).lastModified().toTime_t();
        } else {
            mtime = QDateTime::currentDateTime().toTime_t();
        }
        delete file;
        file = NULL;
    } else {
        mtime = QDateTime::currentDateTime().toTime_t();
    }
    emit signal_status(this);
    dm->saveConfig();
    DBG("ZDMTask %p finished.\n", this);
}

ZDMTaskThread::ZDMTaskThread(ZDMTask *task, ZDM *dm) :
    QThread(dm) {
    dm->threadPool->addThread(this);
    this->dm = dm;
    this->task = task;
    connect(this, SIGNAL(finished()), SLOT(deleteLater()));
}

ZDMTaskThread::~ZDMTaskThread() {
    dm->threadPool->removeThread(this);
}

void ZDMTaskThread::run() {
    task->run();
}
//curl multi fun
typedef size_t ZDMHTTP_WRITE_FUNC(const char *buf, size_t size, size_t nmemb, void *p);
typedef struct CURL_ARG {
    ZDMTask *task;
    ZDMPart *part;
}CURL_ARG;

typedef struct WRITE_ARG {
    ZDMTask *task;
    ZDMPart *part[DEF_MAX_PARTS];
} WRITE_ARG;

static int start_Perform_on(CURLM *multi_handle, ZDMTask *t){
    int running_handle_count;
    int try_count = 0;
    while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &running_handle_count))
    {
        DBG("this is first while perform!\n");
        DBG("handle_cout is %d\n", running_handle_count);
    }
    timeval tv;
    int max_fd;
    fd_set fd_read;
    fd_set fd_write;
    fd_set fd_except;
    qint64 lastprogress;
    while(running_handle_count) {
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        FD_ZERO(&fd_read);
        FD_ZERO(&fd_write);
        FD_ZERO(&fd_except);
        curl_multi_fdset(multi_handle, &fd_read, &fd_write, &fd_except, &max_fd);
        if(-1 == max_fd) {
            if(++try_count > 10) {
                break;
            }
            DBG("curl_multi_fdset return fd -1\n");
            Sleep(200);
        } else {
            int return_code = select(max_fd + 1, &fd_read, &fd_write, &fd_except, &tv);
            if (-1 == return_code)
            {
                DWORD error = ::GetLastError();
                DBG("error id <%ld>\n", error);
                DBG("select is finished! retrurn_cord is <%d>\n", return_code);
                if(++try_count > 10) {
                    break;
                }
            }
        }
        lastprogress = t->progress;
        while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(multi_handle, &running_handle_count))
        {
            DBG("handle_cout is %d\n", running_handle_count);
        }
        if(lastprogress == t->progress) {
            DBG("progress is not move!!!\n");
            if(++try_count > 1000) {
                break;
            }
        } else if(try_count > 0){
            try_count = 0;
        }
    }
    return running_handle_count;
}

static int getDownloadData(char *url, qint64 from[], qint64 to[],
                           const int count, ZDMHTTP_WRITE_FUNC func,
                           void *p) {
    CURLM *multi_handle = NULL;
    CURL *curl[DEF_MAX_PARTS], *var;
    CURL_ARG *arg = new CURL_ARG[count];
    char range[128];
    WRITE_ARG *_getarg = (WRITE_ARG *)p;
    multi_handle = curl_multi_init();

    for(int i = 0; i < count; i++) {
        curl[i] = curl_easy_init();
        var = curl[i];
        if(to[i] > from[i]) {
            sprintf(range, "%lld-%lld", from[i], to[i]);
            curl_easy_setopt(var, CURLOPT_RANGE, range);
        } else {
            continue;
        }

        DBG("http get range %lld -> %lld\n", from[i], to[i]);
        DBG("from[%d] is <%lld>, to[%d] is <%lld>\n", i, from[i], i, to[i]);

        arg[i].task = _getarg->task;
        arg[i].part = _getarg->part[i];
        curl_easy_setopt(var, CURLOPT_URL, url);
        curl_easy_setopt(var, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(var, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(var, CURLOPT_CONNECTTIMEOUT, 10);
        curl_easy_setopt(var, CURLOPT_NOBODY, 0);
        curl_easy_setopt(var, CURLOPT_READFUNCTION, NULL);
        curl_easy_setopt(var, CURLOPT_WRITEFUNCTION, func);
        curl_easy_setopt(var, CURLOPT_WRITEDATA, &(arg[i]));

        curl_multi_add_handle(multi_handle, var);
    }
    int ret = start_Perform_on(multi_handle, _getarg->task);
    for(int i = 0; i < count; i++) {
        curl_multi_remove_handle(multi_handle, curl[i]);
        curl_easy_cleanup(curl[i]);
    }
    curl_multi_cleanup(multi_handle);
    delete []arg;
    return ret;
}

static size_t curl_write_func(const char *buf, size_t size, size_t nmemb, void *p) {
    CURL_ARG *arg = (CURL_ARG *)p;
    size_t n = size * nmemb;

    arg->part->buffer.append(buf, n);
    arg->task->addProgress(n);

    if(arg->part->buffer.size() >= MAX_BUFF_BLKSIZE) {
        arg->task->writeFile(arg->part->begin + arg->part->progress, arg->part->buffer);
        arg->part->progress += arg->part->buffer.size();
        arg->part->buffer.clear();
    }

    if(arg->task->needsStop || arg->task->dm->needsStop) {
        return 0;
    }
    return n;
}

ZDMPartThread::ZDMPartThread(QObject *parent,
                             ZDM *dm,
                             ZDMTask *task,
                             QList<ZDMPart *> part) :
    QThread(parent)
{
    this->c_dm = dm;
    this->c_parts = part;
    this->c_task = task;
    c_task->threadPool->addThread(this);
    connect(this, SIGNAL(finished()), SLOT(deleteLater()));
}

ZDMPartThread::~ZDMPartThread() {
    c_task->threadPool->removeThread(this);
}

void ZDMPartThread::run() {
    WRITE_ARG arg;
    const int _count = c_parts.size();
    DBG("_count is %d\n", _count);
    qint64 *from = new qint64[_count];
    qint64 *to = new qint64[_count];
    arg.task = c_task;
    ZDMPart *var;
    for(int i = 0; i < _count; i++) {
        var = c_parts.at(i);
        arg.part[i] = var;
        DBG("var[%d], begin %lld, end %lld, progress %lld\n", i, var->begin, var->end, var->progress);
        from[i] = var->begin + var->progress;
        to[i] = var->end;
    }

    char *url = strdup(c_task->url.toEncoded().data());
    int r = getDownloadData(url, from, to, _count, curl_write_func,(void*)&arg);
    DBG("getDownloadData is ret %d\n", r);
    free(url);

    for(int i = 0; i < _count; i++) {
        var = c_parts.at(i);
        DBG("part[%d],begin <%lld>, progress<%lld>,end<%lld>\n", i, var->begin, var->progress, var->end);
        DBG("var[%d] size of buffer is %lld, progress is %lld\n", i, var->buffer.size(), var->progress);
        if(var->buffer.size() > 0) {
            c_task->writeFile(var->begin + var->progress, var->buffer);
            var->progress += var->buffer.size();
            var->buffer.clear();
        }
    }
    if(r = CURLE_OK && c_task->size < 0) {
        c_task->size = c_task->progress;
    }
    delete []from;
    delete []to;
}

ZDownloadTask::ZDownloadTask(QObject *parent) :
    QObject(parent) ,
    sub_status(SUB_NONE),
    isUser(false){
}

ZDownloadManager::ZDownloadManager(QObject *parent) :
    QObject(parent) {

}

bool ZDownloadManager::global_init() {
    return ZDMHttp::global_init();
}

ZDownloadManager *ZDownloadManager::newDownloadManager(const QString &prefix, QObject *parent) {
    ZDM *dm = new ZDM(prefix, parent);
    return dm;
}

void ZDownloadManager::delDownloadManager(ZDownloadManager *obj) {
    ZDM *dm = (ZDM*) obj;
    delete dm;
}

ZDM::ZDM(const QString &prefix, QObject *parent) :
    ZDownloadManager(parent) {
    DBG("+ ZDM %p\n", this);
    needsStop = false;
    threadPool = new ZDMThreadPool(this);

    dbPrefix = prefix + ".xml";

    maxTasks = DEF_MAX_TASKS;
    maxParts = DEF_MAX_PARTS;
    maxRetry = DEF_MAX_RETRY;

    connect(this, SIGNAL(signal_newTask(ZDMTask*)), SLOT(slot_newTask(ZDMTask*)));
    connect(this, SIGNAL(signal_checkAndStart()), SLOT(slot_checkAndStart()));
    connect(threadPool, SIGNAL(signalEmpty()), SIGNAL(signal_idle()));
    loadConfig();
}

ZDM::~ZDM() {
    while(!tasks.isEmpty()) {
        delete tasks.takeFirst();
    }
    DBG("- ZDM %p\n", this);
}

void ZDM::setMaxTasks(int max) {
    maxTasks = max;
}

void ZDM::setMaxParts(int max) {
    maxParts = max;
}

void ZDM::setMaxRetry(int max) {
    maxRetry = max;
}
#include <QDir>
void ZDM::startWork() {
#ifdef WRITE_LOG
    //增加一个LOG文件夹
    QDir dir;
    dir.mkdir("ZDM_LOG");
#endif
    emit signal_checkAndStart();
}

void ZDM::stopWork() {
    needsStop = true;
    saveConfig();
}

int ZDM::getUnfinishedCount() {
    int ret = 0;
    tasks_mutex.lock();
    foreach(ZDMTask *t, tasks) {
        if(t->status != ZDMTask::STAT_FINISHED) {
            ret ++;
        }
    }
    tasks_mutex.unlock();
    return ret;
}

int ZDM::getRunningCount() {
    int ret = 0;
    tasks_mutex.lock();
    foreach(ZDMTask *t, tasks) {
        if(t->status == ZDMTask::STAT_DOWNLOADING) {
            ret ++;
        }
    }
    tasks_mutex.unlock();
    return ret;
}

ZDownloadTask *ZDM::addTaskFront(const QString&id, const QString &url, const QString &path) {
    ZDMTask *task = new ZDMTask(this);
    task->id = id;
    task->url = url;
    task->path = path;
    tasks_mutex.lock();
    tasks.prepend(task);
    tasks_mutex.unlock();

    emit signal_newTask(task);
    emit signal_checkAndStart();
    emit signal_status(task);
    return task;
}

ZDownloadTask *ZDM::addTask(const QString &url, const QString &path) {
    return addTask(QUuid::createUuid().toString(), url, path);
}

ZDownloadTask *ZDM::addTask(const QString& id, const QString &url, const QString &path) {
    ZDMTask *task = new ZDMTask(this);
    task->id = id;
    task->url = url;
    task->path = path;

    tasks_mutex.lock();
    tasks.append(task);
    tasks_mutex.unlock();

    emit signal_newTask(task);
    emit signal_checkAndStart();
    emit signal_status(task);
    return task;
}

ZDownloadTask *ZDM::insertTask(const QString &id, const QString &url, const QString &path, bool isUser) {
    ZDMTask *task = (ZDMTask *)getTask(id);
    if(task == NULL) {
        //一个新的任务
        task = new ZDMTask(this);
        task->isUser = isUser;
        task->id = id;
        task->url = url;
        task->path = path;
        tasks_mutex.lock();
        tasks.append(task);
        tasks_mutex.unlock();
        emit signal_newTask(task);
    } else {
        //把旧的非用户任务改变成用户任务
        //为了方便 暂时不修改任务的路径
        task->isUser = isUser;
        task->needsStop = false;
        task->status = ZDMTask::STAT_PENDING;
        task->needsStop = false;
        task->nextStatus = ZDMTask::STAT_PENDING;
    }
    emit signal_checkAndStart();
    emit signal_status(task);
    return task;
}

ZDownloadTask *ZDM::getTask(const QString &id) {
    ZDownloadTask *ret = NULL;
    tasks_mutex.lock();
    foreach(ZDMTask *t, tasks) {
        if(t->id == id) {
            ret = (ZDownloadTask *)t;
            break;
        }
    }
    tasks_mutex.unlock();
    return ret;
}

QList<ZDownloadTask *> ZDM::getTasks() {
    QList<ZDownloadTask *> list;
    tasks_mutex.lock();
    foreach (ZDMTask *t, tasks) {
        list.append(t);
    }
    tasks_mutex.unlock();
    return list;
}

QList<ZDownloadTask *> ZDM::getTasks(const QStringList &ids) {
    QList<ZDownloadTask *> list;
    tasks_mutex.lock();
    foreach (const QString &id, ids) {
        foreach (ZDMTask *t, tasks) {
            if(t->id == id) {
                list.append(t);
                break;
            }
        }
    }
    tasks_mutex.unlock();
    return list;
}

void ZDM::startTask(ZDownloadTask *t) {
    ZDMTask *task = (ZDMTask *)t;
    if(task->status == ZDMTask::STAT_STOPPED
            || task->status == ZDMTask::STAT_FAILED) {
        task->status = ZDMTask::STAT_PENDING;
        emit signal_status(task);
    }
    emit signal_checkAndStart();
}

void ZDM::stopTask(ZDownloadTask *t) {
    ZDMTask *task = (ZDMTask *)t;
    task->needsStop = true;
    if(task->status == ZDMTask::STAT_DOWNLOADING) {
        task->nextStatus = ZDMTask::STAT_STOPPED;
    } else if(task->status == ZDMTask::STAT_PENDING) {
        task->status = ZDMTask::STAT_STOPPED;
        emit signal_status(task);
    }
}

void ZDM::removeTask(ZDownloadTask *t) {
    ZDMTask *task = (ZDMTask *)t;
    if(task->status != ZDMTask::STAT_DOWNLOADING) {
        tasks_mutex.lock();
        DBG("removed task '%s'\n", t->id.toLocal8Bit().data());
        tasks.removeOne(task);
        tasks_mutex.unlock();
        delete task;
    }
}

void ZDM::removeTask(const QString &id) {
    tasks_mutex.lock();
    for(int i=0; i<tasks.size(); i++) {
        if(tasks[i]->status != ZDMTask::STAT_DOWNLOADING && tasks[i]->id == id) {
            DBG("removed task '%s'\n", id.toLocal8Bit().data());
            delete tasks[i];
            tasks.removeAt(i);
            break;
        }
    }
    tasks_mutex.unlock();
}

void ZDM::removeTasks(const QStringList &ids) {
    tasks_mutex.lock();
    foreach(const QString& id, ids) {
        for(int i=0; i<tasks.size(); i++) {
            if(tasks[i]->status != ZDMTask::STAT_DOWNLOADING && tasks[i]->id == id) {
                DBG("removed task '%s'\n", id.toLocal8Bit().data());
                delete tasks[i];
                tasks.removeAt(i);
                break;
            }
        }
    }
    tasks_mutex.unlock();
}

void ZDM::slot_newTask(ZDMTask *t) {
    DBG("slot_newTask %p\n", t);
    connect(t, SIGNAL(signal_progress(ZDownloadTask*)), this, SIGNAL(signal_progress(ZDownloadTask*)));
    connect(t, SIGNAL(signal_speed(ZDownloadTask*,int)), this, SIGNAL(signal_speed(ZDownloadTask*,int)));
    connect(t, SIGNAL(signal_status(ZDownloadTask*)), this, SIGNAL(signal_status(ZDownloadTask*)));
    connect(t, SIGNAL(signal_saveConfig()), this, SLOT(slot_saveConfig()));
}

void ZDM::slot_saveConfig() {
    if(dbTimer.elapsed() > 2000) {
        dbTimer.restart();
        saveConfig();
    }
}

void ZDM::loadConfig() {
    DBG("load config\n");

    QFile file(dbPrefix);
    if(!file.open(QIODevice::ReadOnly)) {
        DBG("ERROR read config file\n");
        return;
    }

    QDomDocument doc;
    if(!doc.setContent(&file)) {
        DBG("ERROR parse config file\n");
        file.close();
        file.remove();
        return;
    }

    QDomNodeList nodes = doc.elementsByTagName("task");
    int count = nodes.count();
    tasks_mutex.lock();
    for(int i=0; i<count; i++) {
        QDomElement e = nodes.at(i).toElement();

        ZDMTask *task = new ZDMTask(this);
        task->id = e.attribute("id");
        task->url = QUrl::fromEncoded(e.attribute("url").toLocal8Bit());
        task->path = e.attribute("path");
        task->status = e.attribute("status").toInt();
        task->mtime = e.attribute("mtime").toUInt();
        task->progress = e.attribute("progress").toLongLong();
        task->size = e.attribute("size").toLongLong();
        task->isUser = e.attribute("isUser").toInt();
        if(task->status != ZDMTask::STAT_STOPPED) {
            task->status = ZDMTask::STAT_PENDING;
        }

        DBG("found exist task '%s'\n", task->id.toLocal8Bit().data());
        QDomNodeList subs = e.elementsByTagName("part");
        for(int j = 0; j < subs.count(); j++) {
            QDomElement sub = subs.at(j).toElement();
            ZDMPart *part = new ZDMPart();
            part->begin = sub.attribute("begin").toLongLong();
            part->end = sub.attribute("end").toLongLong();
            part->progress = sub.attribute("progress").toLongLong();
            task->parts.append(part);
            DBG("found exist part %lld-%lld, %lld\n", part->begin, part->end, part->progress);
        }

        emit signal_newTask(task);
        emit signal_status(task);
        tasks.append(task);
    }
    tasks_mutex.unlock();

    file.close();
}

void ZDM::saveConfig() {
    dbMutex.lock();
    DBG("save config\n");
    do {
        QFile file(dbPrefix);
        file.remove();
        if(!file.open(QIODevice::WriteOnly)) {
            DBG("ERROR create config file\n");
            break;
        }

        QTextStream out(&file);
        out.setCodec("UTF-8");

        QDomDocument doc;
        QDomProcessingInstruction instruction = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
        doc.appendChild(instruction);

        QDomElement root = doc.createElement("tasks");
        doc.appendChild(root);

        tasks_mutex.lock();
        foreach(ZDMTask *task, tasks) {
            if(task->status == ZDMTask::STAT_FINISHED) {
                DBG("skipped finished task %p\n", task);
                continue;
            }

            QDomElement e = doc.createElement("task");
            e.setAttribute("id", task->id);
            e.setAttribute("url", task->url.toEncoded().constData());
            e.setAttribute("path", task->path);
            e.setAttribute("status", task->status);
            e.setAttribute("mtime", task->mtime);
            e.setAttribute("progress", task->progress);
            e.setAttribute("size", task->size);
            e.setAttribute("isUser",task->isUser);

            foreach(ZDMPart *part, task->parts) {
                QDomElement sub = doc.createElement("part");
                sub.setAttribute("begin", part->begin);
                sub.setAttribute("end", part->end);
                sub.setAttribute("progress", part->progress);
                e.appendChild(sub);
            }
            root.appendChild(e);
        }

        tasks_mutex.unlock();

        doc.save(out, 4, QDomNode::EncodingFromTextStream);
        file.close();
    } while(0);
    dbMutex.unlock();
}

void ZDM::slot_checkAndStart() {
    int total_user_count = 0;
    if(needsStop) {
        return;
    }
    tasks_mutex.lock();
    // get running count
    ZDMTask *var;
    QList<ZDMTask *> user_list;
    QList<ZDMTask *> nuser_list;
    //得到总运行的任务数目
    //得到正在运行总的用户任务数目和非用户任务数目
    for(int i = 0; i < tasks.size(); i++) {
        var = tasks.at(i);
        if(var->status == ZDMTask::STAT_DOWNLOADING) {
            if(var->isUser) {
                user_list.append(var);
            } else {
                nuser_list.append(var);
            }
        } else if(var->status == ZDMTask::STAT_FINISHED) {//|| var->status == ZDMTask::STAT_STOPPED
            tasks.removeOne(var);
            delete var;
            var = NULL;
            i--;
        }
        if(var != NULL && var->isUser) {
            total_user_count++;
        }
    }
    //得到用户任务非启动的任务数目
    int vacancy = total_user_count - user_list.size();
    //开始停止非用户任务来 给用户任务留出任务空位
    while(vacancy > 0 && nuser_list.size() > 0) {
        nuser_list.last()->needsStop = true;
        nuser_list.last()->nextStatus = ZDMTask::STAT_PENDING;
        nuser_list.removeLast();
        vacancy--;
    }
    //如果总的运行任务数目大于最大任务设置，停止一部分任务
    while(user_list.size() + nuser_list.size() > maxTasks) {
        if(nuser_list.size() > 0) {
            nuser_list.last()->needsStop = true;
            nuser_list.last()->nextStatus = ZDMTask::STAT_PENDING;
            nuser_list.removeLast();
        } else if(user_list.size() > 0){
            user_list.last()->needsStop = true;
            user_list.last()->nextStatus = ZDMTask::STAT_PENDING;
            user_list.removeLast();
        }
    }
    DBG("running count %d\n", user_list.size() + nuser_list.size());
    //如果运行的任务数目小于最大任务设置，开始一部分任务
    if(user_list.size() + nuser_list.size() < maxTasks) {
        ZDMTask *t;
        for(int i = 0; i < tasks.size(); i++) {
            t = tasks.at(i);
            if(t->isUser && t->status == ZDMTask::STAT_PENDING) {
                t->needsStop = false;
                t->status = ZDMTask::STAT_DOWNLOADING;
                ZDMTaskThread *thread = new ZDMTaskThread(t, this);
                connect(thread, SIGNAL(destroyed()), this, SIGNAL(signal_checkAndStart()));
                thread->start();
                user_list.append(t);
            }
            if(user_list.size() + nuser_list.size() == maxTasks) {
                break;
            }
        }
    }
    if(user_list.size() + nuser_list.size() < maxTasks) {
        ZDMTask *t;
        for(int i = 0; i < tasks.size(); i++) {
            t = tasks.at(i);
            if(t->status == ZDMTask::STAT_PENDING) {
                t->needsStop = false;
                t->status = ZDMTask::STAT_DOWNLOADING;
                ZDMTaskThread *thread = new ZDMTaskThread(t, this);
                connect(thread, SIGNAL(destroyed()), this, SIGNAL(signal_checkAndStart()));
                thread->start();
                nuser_list.append(t);
            }
            if(user_list.size() + nuser_list.size() == maxTasks) {
                break;
            }
        }
    }
    tasks_mutex.unlock();
}

ZDMThreadPool::ZDMThreadPool(QObject *parent) :
    QObject(parent){
    DBG("+ ZDMThreadPool %p\n", this);
}

ZDMThreadPool::~ZDMThreadPool() {
    DBG("- ZDMThreadPool %p\n", this);
}

void ZDMThreadPool::addThread(QThread *t) {
    mutex.lock();
    list.append(t);
    mutex.unlock();
}

void ZDMThreadPool::removeThread(QThread *t) {
    int size;
    mutex.lock();
    list.removeOne(t);
    size = list.size();
    mutex.unlock();
    if(size == 0) {
        emit signalEmpty();
    }
}

int ZDMThreadPool::threadCount() {
    int size;
    mutex.lock();
    size = list.size();
    mutex.unlock();
    return size;
}
