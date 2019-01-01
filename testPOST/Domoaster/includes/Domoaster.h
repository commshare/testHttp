

#ifndef DOMOASTER_H
#define DOMOASTER_H

#include <memory>
#include <string>

#include "IhmCommunicationThread.h"
#include "LogDestination.h"
#include "RestLogDestination.h"
#include "SettingsParser.h"
#include "WebServer.h"

namespace domoaster {

class Domoaster
{
  public:
    Domoaster (int argc, char **argv)  ;
    void	Start() ;
    //~ NodesManager * NodesMgr () {return &_nodesManager ; }
    //~ ScriptsManager * ScriptsMgr () {return &_scriptsManager ; }

  private:
    void	InitConfig (int argc, char **argv)  ;
    void	InitRestServer () ;

    void	InitLog () ;
    void	InitRestLog();

    void	InitPlugins () ;

    void	InitConnectors () ;
    void	InitProtocols () ;
    void	InitNodes () ;
    void	InitFeatures () ;
    void	InitManagers () ;

    void	StartComm() ;
    void	StartFeatures () ;

  private:
    StdErrorLogDestination *stderr_log ;
    RestLogDestination *restLog ;

    SettingsParser *	_pSettings ;

    Kernel _kernel ;

    std::unique_ptr<IhmCommunicationThread> _pCom;

    WebServer::HTTPServer * _server;

    //~ NodesManager _nodesManager ;
    //~ ScriptsManager _scriptsManager ;
} ;

} ; // namespace domoaster

#endif // DOMOASTER_H