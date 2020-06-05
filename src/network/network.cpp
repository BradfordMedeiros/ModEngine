#include "./network.h"

#define MAX_LISTENING_QUEUE 100
#define NETWORK_BUFFER_SIZE 1024

void guard(int value, const char* runtimeErrorMessage){
  if (value < 0){
    throw std::runtime_error(runtimeErrorMessage);
  }
}

modsocket createServer(){
	int socketFd = socket(AF_INET, SOCK_STREAM, 0);	
	if (socketFd == -1){
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

  fd_set set;
  FD_ZERO(&set);

  struct modsocket socketInfo  = {
    .socketin = address,
    .socketFd = socketFd,
    .fds = set,
    .maxFd = 1,
  };

  return socketInfo;
}

bool acceptSocketAndMarkNonBlocking(modsocket& socketInfo){
  int addrlen = sizeof(socketInfo.socketin);
  int newSocket =  accept(socketInfo.socketFd, (struct sockaddr *)&socketInfo.socketin,  (socklen_t*)&addrlen); 

  if (newSocket < 0){  // @todo what other failure modes here?
    if (errno == EWOULDBLOCK || errno == EAGAIN){
      return false;
    }
    throw std::runtime_error(std::string("error accepting socket ") + strerror(errno));
  }else{
    guard(fcntl(newSocket, F_SETFL, fcntl(newSocket, F_GETFL) | O_NONBLOCK), "error setting nonblocking mode for socket");

    std::cout << "socket accepted: " << newSocket << std::endl;

    std::cout << "network: maxFd is now: " << newSocket << std::endl;
    FD_SET(newSocket, &socketInfo.fds);     
    socketInfo.maxFd = socketInfo.maxFd > newSocket ? socketInfo.maxFd : newSocket;
    return true;
  }
  assert(false);
}

void sendDataAndCloseSocket (modsocket& socketInfo, int socketFd, std::function<std::string(std::string)> onData){
  char buffer[NETWORK_BUFFER_SIZE] = {0};

  std::cout << "network: reading socket data" << std::endl;
  int value = read(socketFd, buffer, NETWORK_BUFFER_SIZE);
  std::cout << "network: finished reading socket data" << std::endl;

  std::string serverResponse = onData(buffer);

  const char* responsep = serverResponse.c_str();

  std::cout << "network: sending socket data" << std::endl;
  send(socketFd, responsep, strlen(responsep), 0);
  std::cout << "network: finished sending socket data" << std::endl;

  std::cout << "network: closing socket" << std::endl;
  close(socketFd);
  FD_CLR(socketFd, &socketInfo.fds);
  std::cout << "network: closed socket" << std::endl;

}

void getDataFromSocket(modsocket& socketInfo, std::function<std::string(std::string)> onData){
  acceptSocketAndMarkNonBlocking(socketInfo);

  timeval timeout {
    .tv_sec = 0,
    .tv_usec = 0,
  };

  auto newFds = socketInfo.fds;

  int hasData = select(socketInfo.maxFd + 1, &newFds, NULL, NULL, &timeout);
  if (hasData <= 0){
    if (hasData == -1){
      std::cout << errno << std::endl;
      assert(false);
    }
  }

  std::vector<int> readySocketFds;
  int selectedSocket = -1;
  for (int socketFd = 0; socketFd <= socketInfo.maxFd; socketFd++){
    if (FD_ISSET(socketFd, &socketInfo.fds)){
      std::cout << "network: socket is set!" << std::endl;
      readySocketFds.push_back(socketFd);
    }
  }
 
  for(int socketFd : readySocketFds){
    sendDataAndCloseSocket(socketInfo, socketFd, onData);
  }
}

void cleanupSocket(modsocket& socketInfo){
  close(socketInfo.socketFd);
}

void sendMessage(char* networkBuffer){
  struct sockaddr_in address = {
    .sin_family = AF_INET,
    .sin_port = htons(8000),
    .sin_addr = in_addr {
      .s_addr = inet_addr("127.0.0.1"),
    },
  };

  int sockFd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFd == -1){
    throw std::runtime_error("error creating socket");
  }

  guard(connect(sockFd, (struct sockaddr*) &address, sizeof(address)), "error connecting socket");
  write(sockFd, networkBuffer, strlen(networkBuffer));
  close(sockFd);
}

