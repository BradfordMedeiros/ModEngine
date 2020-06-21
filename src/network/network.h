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
#include "../common/util.h"

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
      
modsocket createServer();
ConnectionInfo getConnectionInfo(modsocket& modsocket, int sockfd);


struct socketResponse {
  std::string response;
  bool shouldCloseSocket;
  bool shouldSendData;
};
void getDataFromSocket(modsocket& socketInfo, std::function<socketResponse(std::string, int)>);
void sendDataOnSocket(int socketFd, const char* data);

struct udpmodsocket {
  int socketFd;
};

enum PacketType { };
struct UdpPacket {
  short id;
  glm::vec3 position;
  glm::vec3 scale;
};

udpmodsocket createUdpServer();
void getDataFromUdpSocket(int socket, std::function<void(UdpPacket, sockaddr_in)> onData);
void sendDataOnUdpSocket(const char* networkBuffer);
short unsigned int getPortFromSocketIn(sockaddr_in& socketin);
std::string getIpAddressFromSocketIn(sockaddr_in& socketin);

#endif 
