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
#include <optional>

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
      
struct SocketResponse {
  std::string response;
  bool shouldCloseSocket;
  bool shouldSendData;
};

struct tcpServer {
  modsocket server;
};

struct ClientConnection {
  bool isConnected = false;
  int currentSocket = -1;
};
struct NetCode {
  tcpServer tServer;
};

NetCode initNetCode(bool bootstrapperMode, std::function<std::string(std::string)> readFile);

void onNetCode(NetCode& netcode, std::function<void(std::string)> onClientMessage, bool bootstrapperMode);

void connectServer(std::string data);
void disconnectServer();
std::string sendMessage(std::string dataToSend);


std::unordered_map<std::string, std::string> listServers();


#endif