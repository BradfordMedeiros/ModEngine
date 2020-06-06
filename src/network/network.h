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
#include <arpa/inet.h>
#include <assert.h>
#include <functional>
#include "./common.h"

// Server
struct modsocket {
	sockaddr_in socketin;
	int socketFd;
	fd_set fds;
	int maxFd;
};
      
modsocket createServer();
void getDataFromSocket(modsocket& socketInfo, std::function<std::string(std::string)>);
void cleanupSocket(modsocket& socketInfo);

#endif 
