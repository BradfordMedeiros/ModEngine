#ifndef MOD_SERVERS
#define MOD_SERVERS

#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include "../common/util.h"
#include "./network.h"

struct ServerBrowser {
  std::map<std::string, std::string> serverNameToIp;
};  

struct tcpServer {
  ServerBrowser browser;
  modsocket server;
  std::map<std::string, ConnectionInfo> connections;
};

struct NetCode {
  tcpServer tServer;
  udpmodsocket udpModsocket;
  std::map<std::string, sockaddr_in> udpConnections;
  std::function<void(std::string&)> onPlayerConnected;
  std::function<void(std::string&)> onPlayerDisconnected;
};
NetCode initNetCode(std::function<void(std::string&)> onPlayerConnected, std::function<void(std::string&)> onPlayerDisconnected, std::function<std::string(std::string)> readFile);
bool tickNetCode(NetCode& netcode, NetworkPacket& packet, std::function<std::string()> maybeGetSetupConnectionHash);
void sendUdpPacketToAllUdpClients(NetCode& netcode, NetworkPacket data);

#endif