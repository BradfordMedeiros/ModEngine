#include "./network.h"

#define MAX_LISTENING_QUEUE 10
#define NETWORK_BUFFER_SIZE 1024

void guard(int value, const char* runtimeErrorMessage){
  if (value < 0){
    throw std::runtime_error(runtimeErrorMessage);
  }
}

modsocket createServer(){
	int socketFd = socket(AF_INET, SOCK_STREAM, 0);	
	if (socketFd == 0){
		throw std::runtime_error("error creating socket");
	}

  struct sockaddr_in address = {
  	.sin_family = AF_INET,
  	.sin_port = htons(8000),
  	.sin_addr = in_addr {
  		.s_addr = htonl(INADDR_ANY),
  	},
  };
    
  int enable = 1;
	guard(setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)), "error setting reuseaddr");
	guard(setsockopt(socketFd, SOL_SOCKET, SO_REUSEPORT, &enable, sizeof(enable)), "error setting reuseport");
  guard(bind(socketFd, (struct sockaddr*) &address, sizeof(address)), "error binding socket");
  guard(listen(socketFd, MAX_LISTENING_QUEUE), "error listening socket");
  guard(fcntl(socketFd, F_SETFL, fcntl(socketFd, F_GETFL) | O_NONBLOCK), "error setting nonblocking mode for socketin");

  struct modsocket socketInfo  = {
    .socketin = address,
    .socketFd = socketFd,
    .maxFd = 1,
  };

  return socketInfo;
}

bool acceptSocketAndMarkNonBlocking(modsocket& socketInfo){
  int addrlen = sizeof(socketInfo.socketin);
  int newSocket =  accept(socketInfo.socketFd, (struct sockaddr *)&socketInfo.socketin,  (socklen_t*)&addrlen); 

  if (newSocket < 0){  // @todo what other failure modes here?
    if (errno != EWOULDBLOCK && errno != EAGAIN){
      throw std::runtime_error(std::string("error accepting socket ") + strerror(errno));
    }
  }else{
    guard(fcntl(newSocket, F_SETFL, fcntl(newSocket, F_GETFL) | O_NONBLOCK), "error setting nonblocking mode for socket");
   
    FD_SET(newSocket, &socketInfo.fds);     
    socketInfo.maxFd = newSocket;
    
    return true;
  }
  return false;
}

void sendDataAndCloseSocket (modsocket& socketInfo, int socketFd){
  FD_CLR(socketFd, &socketInfo.fds);
  char buffer[NETWORK_BUFFER_SIZE];
  int value = read(socketFd, buffer, NETWORK_BUFFER_SIZE);
  const char* okString = "ok";
  send(socketFd, okString, strlen(okString), 0);
  close(socketFd);
}

void getDataFromSocket(modsocket socketInfo){
  while (acceptSocketAndMarkNonBlocking(socketInfo));

  timeval timeout {
    .tv_sec = 0,
    .tv_usec = 0,
  };

  int hasData = select(socketInfo.maxFd + 1, &socketInfo.fds, NULL, NULL, &timeout);
  if (hasData <= 0){
    if (hasData == -1){
      std::cout << errno << std::endl;
    }
    return;
  }

  std::vector<int> readySocketFds;
  int selectedSocket = -1;
  for (int socketFd = 0; socketFd <= socketInfo.maxFd; socketFd++){
    if (FD_ISSET(socketFd, &socketInfo.fds)){
      readySocketFds.push_back(socketFd);
    }
  }
 
  for(int socketFd:readySocketFds){
    sendDataAndCloseSocket(socketInfo, socketFd);
  }
}

void cleanupSocket(modsocket socketInfo){
  close(socketInfo.socketFd);
}

