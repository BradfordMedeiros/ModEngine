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
  std::function<void()> onPlayerConnected;
  std::function<void()> onPlayerDisconnected;
};
NetCode initNetCode(std::function<void()> onPlayerConnected, std::function<void()> onPlayerDisconnected);
void tickNetCode(NetCode& netcode);

#endif