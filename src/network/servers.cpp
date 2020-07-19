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
void processTcpServer(tcpServer& tserver, std::map<std::string, sockaddr_in>& udpConnections, std::function<void(std::string)> onPlayerDisconnected){
  getDataFromSocket(tserver.server, [&tserver, &udpConnections, &onPlayerDisconnected](std::string request, int socketFd) -> socketResponse {      
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
        response = connectionHash;
        tserver.connections[connectionHash] = connectionInfo;
      }else{
        response = "nack";
        shouldCloseSocket = true;
      }
      shouldSendData = true;
    }else if (requestHeader == "disconnect"){
      auto connectionInfo = getConnectionInfo(tserver.server, socketFd);
      auto connectionHash = getConnectionHash(connectionInfo.ipAddress, connectionInfo.port);
      if (tserver.connections.find(connectionHash) == tserver.connections.end()){
        response = "nack";
      }else{
        tserver.connections.erase(connectionHash);      
        udpConnections.erase(connectionHash);
        response = "ack";
        onPlayerDisconnected(connectionHash);  
      }
      shouldCloseSocket = true;
      shouldSendData = true;
    }
    else if (requestHeader == "type:data"){
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

void sendNetworkPacketUpdateToAllExcept(int socket, NetworkPacket packet, std::map<std::string, sockaddr_in>& udpConnections, std::string& excludeConnectionHash){
  std::cout << "INFO: NETWORK: sending data to all except: " << excludeConnectionHash << " num connections: " << udpConnections.size() << std::endl;
  for (auto [connectHash, addr] : udpConnections){
    std::cout << "INFO: NETWORKING: sending udp datagram to " << connectHash << std::endl;
    if (connectHash != excludeConnectionHash){
      int numBytes = sendto(socket, (char*)packet.packet, packet.packetSize,  MSG_CONFIRM, (const struct sockaddr *) &addr,   sizeof(addr)); 
      if (numBytes == -1){
        throw std::runtime_error("error sending udp data");
      }     
    }else{
      std::cout << "INFO: NETWORKING: sending udp datagram skipping" << std::endl;
    }
  }
}

NetCode initNetCode(std::function<void(std::string)> onPlayerConnected, std::function<void(std::string)> onPlayerDisconnected){
  std::cout << "INFO: running in server bootstrapper mode" << std::endl;
  std::map<std::string, sockaddr_in> udpConnections;
  NetCode netcode {
    .tServer = initTcpServer(),
    .udpModsocket = createUdpServer(),
    .udpConnections = udpConnections,
    .onPlayerConnected = onPlayerConnected,
    .onPlayerDisconnected = onPlayerDisconnected,
  };
  return netcode;
}
bool tickNetCode(NetCode& netcode, NetworkPacket& packet, std::function<std::string()> maybeGetSetupConnectionHash){
  processTcpServer(netcode.tServer, netcode.udpConnections, netcode.onPlayerDisconnected);
  UdpSocketData response = getDataFromUdpSocket(netcode.udpModsocket.socketFd, packet.packet, packet.packetSize);
  if (response.hasData){
    std::cout << "INFO: TICK NETCODE: got data" << std::endl;
    auto setupConnectionHash = maybeGetSetupConnectionHash();
    if (setupConnectionHash != ""){
      assert(netcode.udpConnections.find(setupConnectionHash) == netcode.udpConnections.end());
      netcode.udpConnections[setupConnectionHash] = response.socketin;
      netcode.onPlayerConnected(setupConnectionHash);   // need to relate the mapping between the udp and tcp connection
    }
  }
  return response.hasData;
}

void sendUdpPacketToAllUdpClients(NetCode& netcode, NetworkPacket data){
  std::string value = "";
  sendNetworkPacketUpdateToAllExcept(netcode.udpModsocket.socketFd, data, netcode.udpConnections, value);
}