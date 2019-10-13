#ifndef MOD_NETWORK
#define MOD_NETWORK

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h> 
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h> 
#include <cstring>
#include <errno.h>
#include <vector>

struct modsocket {
	sockaddr_in socketin;
	int socketFd;
	fd_set fds;
	int maxFd;
};
      
modsocket createServer();
void getDataFromSocket(modsocket socketInfo, void (*onData)(std::string));
void cleanupSocket(modsocket socketInfo);

void createClient();
void sendMessage();

#endif 
