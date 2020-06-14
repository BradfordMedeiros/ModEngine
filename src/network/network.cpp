#include "./network.h"

#define MAX_LISTENING_QUEUE 100
#define NETWORK_BUFFER_SIZE 1024

modsocket createServer(){
	int socketFd = socket(AF_INET, SOCK_STREAM, 0);	
	if (socketFd == -1){
		throw std::runtime_error("error creating socket");
	}

  sockaddr_in address = {
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

  modsocket socketInfo {
    .socketFd = socketFd,
    .fds = set,
    .maxFd = 1,
  };

  return socketInfo;
}

ConnectionInfo getConnectionInfo(modsocket& modsocket, int sockfd){
  for (auto connectionInfo : modsocket.infos){
    if (connectionInfo.socketFd == sockfd){
      return connectionInfo;
    }
  }
  assert(false);
}

std::optional<ConnectionInfo> acceptSocketAndMarkNonBlocking(modsocket& socketInfo){
  sockaddr_in socketin;
  int addrlen = sizeof(socketin);
  int newSocket =  accept(socketInfo.socketFd, (struct sockaddr *)&socketin,  (socklen_t*)&addrlen); 

  if (newSocket < 0){  // @todo what other failure modes here?
    if (errno == EWOULDBLOCK || errno == EAGAIN){
      return std::nullopt;
    }
    throw std::runtime_error(std::string("error accepting socket ") + strerror(errno));
  }else{
    guard(fcntl(newSocket, F_SETFL, fcntl(newSocket, F_GETFL) | O_NONBLOCK), "error setting nonblocking mode for socket");

    std::cout << "socket accepted: " << newSocket << std::endl;

    short unsigned int inboundPort = ntohs(socketin.sin_port);
    char ipAddress[16] = {0};
    inet_ntop(AF_INET, &socketin.sin_addr, ipAddress, sizeof(ipAddress));

    ConnectionInfo info {
      .port = inboundPort,
      .socketFd = newSocket,
      .ipAddress = ipAddress,
    };

    std::cout << "network: maxFd is now: " << newSocket << std::endl;
    FD_SET(newSocket, &socketInfo.fds);     
    socketInfo.maxFd = socketInfo.maxFd > newSocket ? socketInfo.maxFd : newSocket;
    return info;
  }
  assert(false);
}

void closeSocket(modsocket& socketInfo, int socketFd){
  std::cout << "network: closing socket" << std::endl;
  close(socketFd);
  FD_CLR(socketFd, &socketInfo.fds);
  std::cout << "network: closed socket" << std::endl;

  std::vector<ConnectionInfo> infos;
  for (auto info : socketInfo.infos){
    if (info.socketFd != socketFd){
      infos.push_back(info);
    }
  }
  socketInfo.infos = infos;
}
void sendDataOnSocket(int socketFd, const char* data){
  std::cout << "network: sending socket data" << std::endl;
  send(socketFd, data, strlen(data), 0);
  std::cout << "network: finished sending socket data" << std::endl;
}
void sendDataToSocket(modsocket& socketInfo, int socketFd, std::function<socketResponse(std::string, int)> onData){
  char buffer[NETWORK_BUFFER_SIZE] = {0};

  std::cout << "network: reading socket data" << std::endl;
  int value = read(socketFd, buffer, NETWORK_BUFFER_SIZE);      // @TODO -> read might still have more data?
  std::cout << "network: finished reading socket data" << std::endl;
  assert(value != -1);

  auto socketResponse = onData(buffer, socketFd);

  if (socketResponse.shouldSendData){
    const char* responsep = socketResponse.response.c_str();
    sendDataOnSocket(socketFd, responsep);
  }
  if (socketResponse.shouldCloseSocket){
    closeSocket(socketInfo, socketFd);
  }
}

void getDataFromSocket(modsocket& socketInfo, std::function<socketResponse(std::string, int)> onData){
  auto optConnectionInfo = acceptSocketAndMarkNonBlocking(socketInfo);
  if (optConnectionInfo.has_value()){
    socketInfo.infos.push_back(optConnectionInfo.value());
  }

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
  for (int socketFd = 0; socketFd <= socketInfo.maxFd; socketFd++){
    if (FD_ISSET(socketFd, &socketInfo.fds)){
      std::cout << "network: socket is ready!" << std::endl;
      readySocketFds.push_back(socketFd);
    }
  }
 
  for(int socketFd : readySocketFds){
    sendDataToSocket(socketInfo, socketFd, onData);
  }
}


udpmodsocket createUdpServer(){
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

  sockaddr_in address {
    .sin_family = AF_INET,
    .sin_port = htons(8001),
    .sin_addr = in_addr{
      .s_addr = htonl(INADDR_ANY),
    }
  };
  guard(bind(sockfd, (struct sockaddr*)&address, sizeof(address)), "error binding socket");

  if (sockfd == -1){
    throw std::runtime_error("error creating socket");
  }
  udpmodsocket socketInfo {
    .socketFd = sockfd,
  };
  return socketInfo;
}

void getDataFromUdpSocket(int socket, std::function<void(std::string)> onData){
  char buffer[NETWORK_BUFFER_SIZE] = {0};
  sockaddr_in socketin;
  int len = sizeof(socketin);
  int value = recvfrom(socket, buffer, NETWORK_BUFFER_SIZE,  MSG_DONTWAIT, ( struct sockaddr *) &socketin, (socklen_t*)&len); 
  
  if (value > 0){
    onData(buffer);
  }
}