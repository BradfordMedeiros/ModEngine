#include "./servers.h"

std::map<std::string, std::string> parseConfigData(std::string serverConfig){
  std::map<std::string, std::string> serverNameToIp;
  auto serverInfos = filterWhitespace(split(serverConfig, '\n'));

  for (auto serverInfo : serverInfos){
    auto infos = filterWhitespace(split(serverInfo, ' '));
    assert(infos.size() == 2);
    auto serverName = infos.at(0);
    auto serverIp = infos.at(1);
    serverNameToIp[serverName] = serverIp;      // validate server ip at some point
  }
  return serverNameToIp;
}

ServerBrowser loadServerInfo(){
  ServerBrowser browser {
    .serverNameToIp = parseConfigData(loadFile("./res/data/servers")),
  };
  return browser;
}

std::string handleListServer(ServerBrowser& browser){
  std::string servers = "";
  for (auto [serverName, serverIp] : browser.serverNameToIp){
    servers = servers + serverName + " " + serverIp + "\n";
  }
  return servers;
}

void launchServer(){
  auto browser = loadServerInfo();
  
  std::cout << "INFO: create server start" << std::endl;
  auto server = createServer();
  std::cout << "INFO: create server end" << std::endl;

  std::map<std::string, ConnectionInfo> connections;
  while(true){
    getDataFromSocket(server, [&browser, &connections](std::string request, int socketFd) -> socketResponse {      // @TODO probably use byte encoding for this instead of using string style comparisons
      auto requestLines = split(request, '\n');
      auto requestHeader = requestLines.at(0);

      std::string response = "ok";
      bool shouldCloseSocket = true;

      if (requestHeader == "list-servers"){
        response = handleListServer(browser);
      } else if (requestHeader == "connect"){
        auto connectionInfo = getConnectionInfo(socketFd);
        auto connectionHash = connectionInfo.ipAddress + "\\" + std::to_string(connectionInfo.port); 
        if (connections.find(connectionHash) == connections.end()){
          response = "ack";
          connections[connectionHash] = connectionInfo;
          shouldCloseSocket = false;
        }else{
          response = "nack";
        }
      }else if (requestHeader == "type:data"){
        auto data = request.substr(10);
        response = "ok";
      }

      std::cout << "response is: " << std::endl << response << std::endl;
      socketResponse serverResponse {
        .response = response,
        .shouldCloseSocket = shouldCloseSocket,
      };
      return serverResponse;
    });
  }
}