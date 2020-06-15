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

std::string getConnectionHash(std::string ipAddress, int port){
  return ipAddress + "\\" + std::to_string(port);
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
      auto connectionHash = getConnectionHash(connectionInfo.ipAddress, connectionInfo.port);
   
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

void sendUdpPacketUpdateToAllExcept(int socket, UdpPacket& packet, std::map<std::string, sockaddr_in>& udpConnections, std::string& excludeConnectionHash){
  for (auto [connectHash, addr] : udpConnections){
    if (connectHash != excludeConnectionHash){
      int numBytes = sendto(socket, (char*)&packet, sizeof(packet),  MSG_CONFIRM, (const struct sockaddr *) &addr,   sizeof(addr)); 
      if (numBytes == -1){
        throw std::runtime_error("error sending udp data");
      }     
      std::cout << "sending udp packet to: " << connectHash << std::endl;
    }
  }
}


void launchServers(){
  std::cout << "INFO: running in server bootstrapper mode" << std::endl;
  auto tserver = initTcpServer();
  auto udpmodSocket = createUdpServer(); 
  std::map<std::string, sockaddr_in> udpConnections;

  while(true){
    processTcpServer(tserver);
    getDataFromUdpSocket(udpmodSocket.socketFd, [&udpConnections, &udpmodSocket](UdpPacket data, sockaddr_in addr) -> void {
      auto hash = getConnectionHash(getIpAddressFromSocketIn(addr), getPortFromSocketIn(addr));
      udpConnections[hash] = addr;
      sendUdpPacketUpdateToAllExcept(udpmodSocket.socketFd, data, udpConnections, hash);
    });
  }
}

