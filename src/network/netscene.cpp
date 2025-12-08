#include "./netscene.h"

#define MAX_LISTENING_QUEUE 100
#define NETWORK_BUFFER_SIZE 1024
#define NETWORK_BUFFER_CLIENT_SIZE 1024

ClientConnection client{};

void guard(int value, const char* runtimeErrorMessage){
  if (value < 0){
    throw std::runtime_error(runtimeErrorMessage);
  }
}

// Server code ////////////////////////////////////////////////////////////////////////

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

short unsigned int getPortFromSocketIn(sockaddr_in& socketin){
  short unsigned int inboundPort = ntohs(socketin.sin_port);
  return inboundPort;
}
std::string getIpAddressFromSocketIn(sockaddr_in& socketin){
  char ipAddress[16] = {0};
  inet_ntop(AF_INET, &socketin.sin_addr, ipAddress, sizeof(ipAddress));
  return ipAddress;
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

    ConnectionInfo info {
      .port = getPortFromSocketIn(socketin),
      .socketFd = newSocket,
      .ipAddress = getIpAddressFromSocketIn(socketin),
    };


    std::cout << "network: maxFd is now: " << newSocket << std::endl;
    FD_SET(newSocket, &socketInfo.fds);     
    socketInfo.maxFd = socketInfo.maxFd > newSocket ? socketInfo.maxFd : newSocket;
    return info;
  }
  assert(false);
  return std::nullopt;
}

void closeServerSocket(modsocket& socketInfo, int socketFd){
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

typedef std::function<void(char*, size_t, int, std::function<void(char* data, size_t size, bool closeSocket)>)> OnDataFn;

void sendDataToSocket(modsocket& socketInfo, int socketFd, OnDataFn onData){
  char buffer[NETWORK_BUFFER_SIZE] = {0};
  size_t bytesRead = read(socketFd, buffer, NETWORK_BUFFER_SIZE);      // @TODO -> read might still have more data?
  //assert(value != -1);
  if (bytesRead == -1){
    return;
  }

  bool responseHandled = false;
  onData(buffer, bytesRead, socketFd, [&socketInfo, socketFd, &responseHandled](char* data, size_t size, bool closeSocket) -> void {
    if (data){
      send(socketFd, data, size, 0);
    }
    if (closeSocket){
      closeServerSocket(socketInfo, socketFd);
    }
    responseHandled = true;
  });

  if (!responseHandled){
    std::cout << "response not handled" << std::endl;
    closeServerSocket(socketInfo, socketFd);
  }
}

void getDataFromServerSocket(modsocket& socketInfo, OnDataFn onData){
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
      readySocketFds.push_back(socketFd);
    }
  }
 
  for(int socketFd : readySocketFds){
    sendDataToSocket(socketInfo, socketFd, onData);
  }
}

tcpServer initTcpServer(std::function<std::string(std::string)> readFile){
  std::cout << "INFO: create server start" << std::endl;
  auto server = createServer();
  std::cout << "INFO: create server end" << std::endl;
  tcpServer tserver {
    .server = server,
  };
  return tserver;
}

NetCode initNetCode(bool bootstrapperMode, std::function<std::string(std::string)> readFile){
  std::cout << "INFO: running in server bootstrapper mode" << std::endl;
  NetCode netcode {};
  if (bootstrapperMode){
    netcode.tServer = initTcpServer(readFile);
  }
  return netcode;
}

// Client code ////////////////////////////////////////////////////////////////////////
void sendMessageWithClientConnection(int sockFd, const char* networkBuffer, size_t size){
  write(sockFd, networkBuffer, size);
}

std::vector<uint8_t> readMessageWithConnectionAsBytes(int sockFd) {
    uint8_t buffer[NETWORK_BUFFER_CLIENT_SIZE];
    size_t bytesRead = read(sockFd, buffer, NETWORK_BUFFER_CLIENT_SIZE);
    guard(bytesRead, "error reading message");
    return std::vector<uint8_t>(buffer, buffer + bytesRead);
}

std::string readMessageWithConnection(int sockFd){
  auto response = readMessageWithConnectionAsBytes(sockFd);
  return std::string(reinterpret_cast<const char*>(response.data()), response.size());
}


int socketClientConnection(std::string ip, int port){
  struct sockaddr_in address = {
    .sin_family = AF_INET,
    .sin_port = htons(port),
    .sin_addr = in_addr {
      .s_addr = inet_addr(ip.c_str()),
    },
  };
  int sockFd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockFd == -1){
    throw std::runtime_error("error creating socket");
  }
  guard(connect(sockFd, (struct sockaddr*) &address, sizeof(address)), "error connecting socket");
  return sockFd;
}

std::string sendMessageNewConnection(std::string ip, int port, const char* networkBuffer, size_t size){
  int sockFd = socketClientConnection(ip, port);
  sendMessageWithClientConnection(sockFd, networkBuffer, size);
  auto buffer = readMessageWithConnection(sockFd);
  close(sockFd);
  return buffer;
}

std::vector<uint8_t> sendMessageNewConnectionAsBytes(std::string ip, int port, const char* networkBuffer, size_t size){
  int sockFd = socketClientConnection(ip, port);
  sendMessageWithClientConnection(sockFd, networkBuffer, size);
  auto buffer = readMessageWithConnectionAsBytes(sockFd);
  close(sockFd);
  return buffer;
}



void sendMessageToActiveServer(std::string data){
  assert(client.isConnected);
  std::string content = "type:data\n" + data;
  std::cout << "Sending message to active server starting: " << data << std::endl;
  sendMessageWithClientConnection(client.currentSocket, content.c_str(), strlen(content.c_str()));
  std::cout << "Sending message to active server complete: " << data << std::endl;
}

struct connectResponse {
  int sockFd;
};
connectResponse connectTcp(std::string serverAddress){
  auto sockFd = socketClientConnection(serverAddress, 8000);
  connectResponse resp {
    .sockFd = sockFd,
  };
  return resp;
}

bool socketHasDataToRead(int socketFd){
  int count;
  ioctl(client.currentSocket, FIONREAD, &count);
  bool hasData = count > 0;
  return hasData;
}
void maybeReadClientMessage(ClientConnection& client, std::function<void(std::string)> onClientMessage){
  if (client.isConnected){
    if (socketHasDataToRead(client.currentSocket)){
      char buffer[NETWORK_BUFFER_CLIENT_SIZE] = {0};
      guard(read(client.currentSocket, buffer, NETWORK_BUFFER_CLIENT_SIZE), "error get client message");
      std::cout << "reading client message: end" << std::endl;
      std::string message = buffer;
      if (message != ""){
        std::cout << "length: " << message.size() << std::endl;
        onClientMessage(message);
      }
    }
  }
}

void connectServer(std::string server){
  assert(!client.isConnected);
  // TCP Connection 
  auto response = connectTcp("localhost:8000");
  client.currentSocket = response.sockFd;

  // Statekeeping
  std::cout << "INFO: connection request succeeded" << std::endl;

  std::cout << "INFO: now connected" << std::endl;
  client.isConnected = true;
}

void disconnectServer(){
  assert(client.isConnected);
  client.isConnected = false;
  close(client.currentSocket);
  client.currentSocket = -1;
}

std::string sendMessage(std::string dataToSend){
  auto response = sendMessageNewConnection("127.0.0.1", 8000, dataToSend.c_str(), strlen(dataToSend.c_str()));
  return response;
}

std::vector<uint8_t> sendMessageAsBytes(const char* data, size_t size){
  auto response = sendMessageNewConnectionAsBytes("127.0.0.1", 8000, data, size);
  return response;
}


// Core per frame tick ////////////
void onNetCode(NetCode& netcode, std::function<void(std::string)> onClientMessage, bool bootstrapperMode){
  maybeReadClientMessage(client, onClientMessage);
  if (bootstrapperMode){
    tcpServer& tserver = netcode.tServer;
    getDataFromServerSocket(tserver.server, [&tserver](char* data, size_t size, int socketFd, std::function<void(char* data, size_t size, bool closeSocket)> sendData) -> void {      

      // echo
      sendData(data, size, false);
      return;

      ////
      MessageToSend message{
        .value = 10002,
      };
      //sendData((char*)&message, sizeof(MessageToSend), false);

      sendData((char*)&message, sizeof(MessageToSend), false);
    });
  }
}