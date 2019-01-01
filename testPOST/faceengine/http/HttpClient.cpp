//https://raw.githubusercontent.com/wangmaopengV/FaceEngine/4f3cfcf4f8df375a244fd152cc338e7b0ea8564c/Adapter/ICE/Include/Http/HttpClient.cpp
#include "HttpClient.h"
#include "curl/curl.h"
#include <assert.h>

// #ifdef _DEBUG
// #pragma comment(lib, "libcurld.lib")
// #else
// #pragma comment(lib, "libcurl.lib")
// #endif


CHttpClient::CHttpClient(void)
{
	m_timeout = _HTTP_CLIENT_TIMEOUT_;
}


CHttpClient::~CHttpClient(void)
{
}

int CHttpClient::PostRequest(const std::string &url, 
							 const std::string &vcData, 
							 std::string &vcResponse, 
							 const std::vector<std::string> &head)
{
	vcResponse.clear();
	struct curl_slist *http_header = NULL;
	for(std::vector<std::string>::const_iterator it = head.begin(); it != head.end(); ++it)
	{
		http_header = curl_slist_append(http_header, it->c_str());
	}

	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, vcData.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, vcData.size());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &vcResponse);
	//这个是不是必须是1的
	curl_easy_setopt(curl, CURLOPT_POST, 1);
#ifdef _DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, m_timeout);
	if(http_header != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);
	}

	CURLcode res = curl_easy_perform(curl);
	if(http_header != NULL)
	{
		curl_slist_free_all(http_header);
	}
	curl_easy_cleanup(curl);
	if(res == CURLE_OK)
		return 0;

	return (HTTP_CLIENT_START-res);
}

int CHttpClient::GetRequest(const std::string &url, 
							std::string &vcResponse, 
							const std::vector<std::string> &head)
{
	struct curl_slist *http_header = NULL;
	for(std::vector<std::string>::const_iterator it = head.begin(); it != head.end(); ++it)
	{
		http_header = curl_slist_append(http_header, it->c_str());
	}
	vcResponse.clear();
	CURL *curl = curl_easy_init();		
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &vcResponse);
	curl_easy_setopt(curl, CURLOPT_POST, 0);
#ifdef _DEBUG
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, m_timeout);
	if(http_header != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);
	}
	CURLcode res = curl_easy_perform(curl);
	if(http_header != NULL)
	{
		curl_slist_free_all(http_header);
	}
	curl_easy_cleanup(curl);
	if(res == CURLE_OK)
		return 0;

	return (HTTP_CLIENT_START-res);
}


size_t CHttpClient::WriteData(void *buff, size_t size, size_t nmemb, void *userp)
{
	assert(userp != NULL);
	if(userp == NULL)
		return 0;
	std::string &str = *(std::string *)userp;
	str.append((char*)buff, size * nmemb);

	return size * nmemb;
}

void CHttpClient::SetTimeout(int _timeout, int _connecttimeout)
{
	if(_timeout > 0)
		m_timeout = _timeout;
	
}
int CHttpClient::PostFileRequest(const std::string & url, const char *buff, const int size, std::string &strResponse)
{
	struct curl_httppost * post = NULL;
	struct curl_httppost * last = NULL;

	CURLFORMcode err = CURL_FORMADD_OK;
	CURLcode res = CURLE_FAILED_INIT;

	err = curl_formadd(&post, &last,
		CURLFORM_COPYNAME, "imageData",
		CURLFORM_BUFFER, "data",
		CURLFORM_BUFFERPTR, buff,
		CURLFORM_BUFFERLENGTH, size,
		CURLFORM_END);

	if(err != CURL_FORMADD_OK)
		return HTTP_CLIENT_DATA_ADD;

	strResponse.clear();

	CURL *curl = curl_easy_init();
	if(curl != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1);
#ifdef _DEBUG
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#else
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
#endif
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, m_timeout);

		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	if(post != NULL)
	{
		curl_formfree(post);
	}

	if(res == CURLE_OK)
		return 0;

	return (HTTP_CLIENT_START-res);
}

int CHttpClient::Post2FileRequest(const std::string & url, const char *buff1, const int size1, const char *buff2, const int size2, std::string &strResponse)
{
	struct curl_httppost * post = NULL;
	struct curl_httppost * last = NULL;

	CURLFORMcode err = CURL_FORMADD_OK;
	CURLcode res = CURLE_FAILED_INIT;

	err = curl_formadd(&post, &last,
		CURLFORM_COPYNAME, "imageOne",
		CURLFORM_BUFFER, "data",
		CURLFORM_BUFFERPTR, buff1,
		CURLFORM_BUFFERLENGTH, size1,
		CURLFORM_END);

	if(err != CURL_FORMADD_OK)
		return HTTP_CLIENT_DATA_ADD;

	err = curl_formadd(&post, &last,
		CURLFORM_COPYNAME, "imageTwo",
		CURLFORM_BUFFER, "data",
		CURLFORM_BUFFERPTR, buff2,
		CURLFORM_BUFFERLENGTH, size2,
		CURLFORM_END);

	if(err != CURL_FORMADD_OK)
		return HTTP_CLIENT_DATA_ADD;

	strResponse.clear();

	CURL *curl = curl_easy_init();
	if(curl != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1);
#ifdef _DEBUG
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#else
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
#endif
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, m_timeout);

		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	if(post != NULL)
	{
		curl_formfree(post);
	}

	if(res == CURLE_OK)
		return 0;

	return (HTTP_CLIENT_START-res);
}


int CHttpClient::PostMultiFileRequest(const std::string & url, const CurlFileData *files, const int fileLen, std::string &strResponse)
{
	struct curl_httppost * post = NULL;
	struct curl_httppost * last = NULL;

	CURLFORMcode err = CURL_FORMADD_OK;
	CURLcode res = CURLE_FAILED_INIT;

	for (int i=0; i<fileLen; ++i)
	{
		err = curl_formadd(&post, &last,
			CURLFORM_COPYNAME, "imageDatas",
			CURLFORM_BUFFER, files[i].fileName,
			CURLFORM_BUFFERPTR, files[i].fileData,
			CURLFORM_BUFFERLENGTH, files[i].fileLen,
			CURLFORM_END);
		if(err != CURL_FORMADD_OK)
			return HTTP_CLIENT_DATA_ADD;
	}

	strResponse.clear();

	CURL *curl = curl_easy_init();
	if(curl != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_POST, 1);
#ifdef _DEBUG
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#else
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 0);
#endif
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteData);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, m_timeout);

		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
	}

	if(post != NULL)
	{
		curl_formfree(post);
	}

	if(res == CURLE_OK)
		return 0;

	return (HTTP_CLIENT_START-res);
}
