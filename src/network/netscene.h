#ifndef MOD_NETSCENE
#define MOD_NETSCENE

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h> 
#include <string>
#include <functional>
#include <iostream>
#include <functional>
#include <cstring>
#include <errno.h>
#include <assert.h>
#include "../common/util.h"


void guard(int value, const char* runtimeErrorMessage);

struct NetworkPacket {
  void* packet;
  unsigned int packetSize;
};

struct ConnectionInfo {
  short unsigned int port;
  int socketFd;
  std::string ipAddress;
};

// Server
struct modsocket {
  int socketFd;
  fd_set fds;
  int maxFd;
  std::vector<ConnectionInfo> infos;
};
      
struct socketResponse {
  std::string response;
  bool shouldCloseSocket;
  bool shouldSendData;
};

struct udpmodsocket {
  int socketFd;
};

struct UdpSocketData {
  bool hasData;
  sockaddr_in socketin;
};

short unsigned int getPortFromSocketIn(sockaddr_in& socketin);
std::string getIpAddressFromSocketIn(sockaddr_in& socketin);

struct tcpServer {
  modsocket server;
  std::unordered_map<std::string, ConnectionInfo> connections;
};

struct NetCode {
  tcpServer tServer;
  std::function<void(std::string&)> onPlayerConnected;
  std::function<void(std::string&)> onPlayerDisconnected;
};
NetCode initNetCode(std::function<std::string(std::string)> readFile);



//////////////
enum PacketType { SETUP };
struct SetupPacket {
  char connectionHash[4000];
};
union PacketPayload {
  SetupPacket setuppacket;
};
struct UdpPacket {
  PacketType type;
  PacketPayload payload;
};

void onNetCode(NetCode& netcode, std::function<void(std::string)> onClientMessage, bool bootstrapperMode);

std::string connectServer(std::string data);
void disconnectServer();
void sendMessageToActiveServer(std::string data);

std::unordered_map<std::string, std::string> listServers();


#endif