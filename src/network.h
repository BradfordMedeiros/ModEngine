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
#include <arpa/inet.h>

// Server
struct modsocket {
	sockaddr_in socketin;
	int socketFd;
	fd_set fds;
	int maxFd;
};
      
modsocket createServer();
void getDataFromSocket(modsocket socketInfo, void (*onData)(std::string));
void cleanupSocket(modsocket socketInfo);

// Client 
void sendMessage(char* networkBuffer);

/*
  void onData(std::string data){
    std::cout << "got data: " << data << std::endl;
  }
  void sendMoveObjectMessage(){
    sendMessage((char*)"hello world");
  }
*/

#endif 
