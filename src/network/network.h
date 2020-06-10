#ifndef MOD_NETWORK
#define MOD_NETWORK

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h> 
#include <fcntl.h>
#include <iostream>
#include <unistd.h> 
#include <cstring>
#include <errno.h>
#include <vector>
#include <map>
#include <arpa/inet.h>
#include <assert.h>
#include <functional>
#include "./common.h"

struct ConnectionInfo {
  std::string ipAddress;
  short unsigned int port;
  int socketFd;
};
ConnectionInfo getConnectionInfo(int sockfd);

// Server
struct modsocket {
	sockaddr_in socketin;
	int socketFd;
	fd_set fds;
	int maxFd;
};
      
modsocket createServer();

struct socketResponse {
  std::string response;
  bool shouldCloseSocket;
  bool shouldSendData;
};
void getDataFromSocket(modsocket& socketInfo, std::function<socketResponse(std::string, int)>);
void sendDataOnSocket(int socketFd, const char* data);
void cleanupSocket(modsocket& socketInfo);

#endif 
