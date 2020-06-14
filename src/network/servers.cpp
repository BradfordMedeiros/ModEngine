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

tcpServer initTcpServer(){
  auto browser = loadServerInfo();
  
  std::cout << "INFO: create server start" << std::endl;
  auto server = createServer();
  std::cout << "INFO: create server end" << std::endl;
  std::map<std::string, ConnectionInfo> connections;
  tcpServer tserver {
    .browser = browser,
    .server = server,
    .connections = connections,
  };
  return tserver;
}
void processTcpServer(tcpServer& tserver){
  getDataFromSocket(tserver.server, [&tserver](std::string request, int socketFd) -> socketResponse {      // @TODO probably use byte encoding for this instead of using string style comparisons
    auto requestLines = split(request, '\n');
    auto requestHeader = requestLines.size() > 0 ? requestLines.at(0) : "";

    std::string response = "ok";
    bool shouldCloseSocket = false;
    bool shouldSendData = false;

    if (requestHeader == "list-servers"){
      response = handleListServer(tserver.browser);
      shouldCloseSocket = true;
      shouldSendData = true;
    } else if (requestHeader == "connect"){
      auto connectionInfo = getConnectionInfo(tserver.server, socketFd);
      auto connectionHash = connectionInfo.ipAddress + "\\" + std::to_string(connectionInfo.port); 
   
      if (tserver.connections.find(connectionHash) == tserver.connections.end()){
        std::cout << "INFO: connection hash: " << connectionHash << std::endl;
        response = "ack";
        tserver.connections[connectionHash] = connectionInfo;
      }else{
        response = "nack";
        shouldCloseSocket = true;
      }
      shouldSendData = true;
    }else if (requestHeader == "type:data"){
      auto data = request.substr(10);
      response = "ok";

      for (auto [_, connection] : tserver.connections){
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

void launchServers(){
  std::cout << "INFO: running in server bootstrapper mode" << std::endl;
  auto tserver = initTcpServer();
  auto udpmodSocket = createUdpServer(); 

  while(true){
    processTcpServer(tserver);
    getDataFromUdpSocket(udpmodSocket.socketFd, [](UdpPacket data) -> void {
      //std::cout << "message from udp socket: " << data << std::endl;
      std::cout << "id is: " << data.id << std::endl;
      std::cout << "position is: " << print(data.position) << std::endl;
      std::cout << "scale is: " << print(data.scale) << std::endl;
    });
  }
}

