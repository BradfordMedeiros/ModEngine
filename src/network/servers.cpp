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
    getDataFromSocket(server, [&browser, &connections, &server](std::string request, int socketFd) -> socketResponse {      // @TODO probably use byte encoding for this instead of using string style comparisons
      auto requestLines = split(request, '\n');
      auto requestHeader = requestLines.size() > 0 ? requestLines.at(0) : "";

      std::string response = "ok";
      bool shouldCloseSocket = false;
      bool shouldSendData = false;

      if (requestHeader == "list-servers"){
        response = handleListServer(browser);
        shouldCloseSocket = true;
        shouldSendData = true;
      } else if (requestHeader == "connect"){
        auto connectionInfo = getConnectionInfo(server, socketFd);
        auto connectionHash = connectionInfo.ipAddress + "\\" + std::to_string(connectionInfo.port); 
   
        if (connections.find(connectionHash) == connections.end()){
          std::cout << "INFO: connection hash: " << connectionHash << std::endl;
          response = "ack";
          connections[connectionHash] = connectionInfo;
        }else{
          response = "nack";
          shouldCloseSocket = true;
        }
        shouldSendData = true;
      }else if (requestHeader == "type:data"){
        auto data = request.substr(10);
        response = "ok";

        for (auto [_, connection] : connections){
          sendDataOnSocket(connection.socketFd, data.c_str());
        }
        shouldSendData = true;
      }

      socketResponse serverResponse {
        .response = response,
        .shouldCloseSocket = shouldCloseSocket,
        .shouldSendData = shouldSendData,
      };
      return serverResponse;
    });
  }
}

void launchUdpServer(){
  std::cout << "INFO: create udp server" << std::endl;

  auto udpmodSocket = createUdpServer(); 
  while(true){
    getDataFromUdpSocket(udpmodSocket, [](std::string data) -> void {
      std::cout << "message from udp socket: " << data << std::endl;
    });
  }
}