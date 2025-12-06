#include "./netscene.h"


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
  send(socketFd, data, strlen(data), 0);
}
void sendDataToSocket(modsocket& socketInfo, int socketFd, std::function<socketResponse(std::string, int)> onData){
  char buffer[NETWORK_BUFFER_SIZE] = {0};
  int value = read(socketFd, buffer, NETWORK_BUFFER_SIZE);      // @TODO -> read might still have more data?
  //assert(value != -1);
  if (value == -1){
    return;
  }

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
  std::unordered_map<std::string, ConnectionInfo> connections;
  tcpServer tserver {
    .server = server,
    .connections = connections,
  };
  return tserver;
}

std::string getConnectionHash(std::string ipAddress, int port){
  return ipAddress + "\\" + std::to_string(port);
}

NetCode initNetCode(std::function<std::string(std::string)> readFile){
  std::cout << "INFO: running in server bootstrapper mode" << std::endl;
  NetCode netcode {
    .tServer = initTcpServer(readFile),
  };
  return netcode;
}


#define NETWORK_BUFFER_CLIENT_SIZE 1024

static bool isConnected = false;  // static-state
static std::string currentServerIp = "";

static std::string bootstrapperServer = "127.0.0.1";
static int bootstrapperPort = 8000;

// TCP specific 
static int currentSocket = -1;


void sendMessageWithConnection(int sockFd, const char* networkBuffer){
  write(sockFd, networkBuffer, strlen(networkBuffer));
}
std::string readMessageWithConnection(int sockFd){
  char buffer[NETWORK_BUFFER_CLIENT_SIZE] = {0};
  guard(read(sockFd, buffer, NETWORK_BUFFER_CLIENT_SIZE), "error reading message");   // @TODO -> read should just be threaded off and pump to an event loop
  return buffer;
}

int socketConnection(std::string ip, int port){
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

std::string sendMessageNewConnection(std::string ip, int port, const char* networkBuffer){
  int sockFd = socketConnection(ip, port);
  sendMessageWithConnection(sockFd, networkBuffer);
  auto buffer = readMessageWithConnection(sockFd);
  close(sockFd);
  return buffer;
}

std::unordered_map<std::string, std::string> listServers(){
  return { { "localhost", "8000" }};
}

struct connectResponse {
  int sockFd;
  std::string connectionHash;
};
connectResponse connectTcp(std::string serverAddress){
  auto sockFd = socketConnection(serverAddress, 8000);
  sendMessageWithConnection(sockFd, "connect");
  auto response = readMessageWithConnection(sockFd);
  assert(response != "nack");
  connectResponse resp {
    .sockFd = sockFd,
    .connectionHash = response,
  };
  return resp;
}

std::string connectServerVal(std::string server){
  assert(!isConnected);

  auto serverAddress = listServers().at(server);

  // TCP Connection 
  auto response = connectTcp(serverAddress);
  currentSocket = response.sockFd;
  std::cout << "connect hash is: " << response.connectionHash << std::endl;

  // Statekeeping
  std::cout << "INFO: connection request succeeded" << std::endl;

  std::cout << "INFO: now connected" << std::endl;
  isConnected = true;
  currentServerIp = serverAddress;

  return response.connectionHash;
}

void disconnectServer(){
  assert(isConnected);
  
  sendMessageWithConnection(currentSocket, "disconnect");
  auto response = readMessageWithConnection(currentSocket);
  assert(response == "ack");

  isConnected = false;
  currentServerIp = "";
  currentSocket = -1;
}
bool isConnectedToServer(){
  return isConnected;
}

void sendMessageToActiveServer(std::string data){
  assert(isConnected);
  std::string content = "type:data\n" + data;
  std::cout << "Sending message to active server starting: " << data << std::endl;
  sendMessageWithConnection(currentSocket, content.c_str());
  std::cout << "Sending message to active server complete: " << data << std::endl;
}


NetworkPacket toNetworkPacket(UdpPacket& packet){
  NetworkPacket netpacket {
    .packet = &packet,
    .packetSize = sizeof(packet),
  };
  return netpacket;
}


void tickNetCode(NetCode& netcode){
  tcpServer& tserver = netcode.tServer;
  getDataFromSocket(tserver.server, [&tserver](std::string request, int socketFd) -> socketResponse {      
    auto requestLines = split(request, '\n');
    auto requestHeader = requestLines.size() > 0 ? requestLines.at(0) : "";

    std::string response = "ok";
    bool shouldCloseSocket = false;
    bool shouldSendData = false;

    if (requestHeader == "list-servers"){
      response = "server browser no longer a thing";
      shouldCloseSocket = true;
      shouldSendData = true;
    } else if (requestHeader == "connect"){
      auto connectionInfo = getConnectionInfo(tserver.server, socketFd);
      auto connectionHash = getConnectionHash(connectionInfo.ipAddress, connectionInfo.port);
   
      if (tserver.connections.find(connectionHash) == tserver.connections.end()){
        std::cout << "INFO: connection hash: " << connectionHash << std::endl;
        response = connectionHash;
        tserver.connections[connectionHash] = connectionInfo;
      }else{
        response = "nack";
        shouldCloseSocket = true;
      }
      shouldSendData = true;
    }else if (requestHeader == "disconnect"){
      auto connectionInfo = getConnectionInfo(tserver.server, socketFd);
      auto connectionHash = getConnectionHash(connectionInfo.ipAddress, connectionInfo.port);
      if (tserver.connections.find(connectionHash) == tserver.connections.end()){
        response = "nack";
      }else{
        tserver.connections.erase(connectionHash);      
        response = "ack";
      }
      shouldCloseSocket = true;
      shouldSendData = true;
    }
    else if (requestHeader == "type:data"){
      auto data = request.substr(10);
      response = "ok";

      for (auto [_, connection] : tserver.connections){
        sendDataOnSocket(connection.socketFd, data.c_str());
      }
      shouldSendData = true;
    }

    socketResponse serverResponse {
      .response = response,
      .shouldCloseSocket = shouldCloseSocket,
      .shouldSendData = shouldSendData,
    };
    return serverResponse;
  });
}

bool socketHasDataToRead(int socketFd){
  int count;
  ioctl(currentSocket, FIONREAD, &count);
  return count > 0;
}

void maybeGetClientMessage(std::function<void(std::string)> onClientMessage){
  if (isConnectedToServer()){
    if (socketHasDataToRead(currentSocket)){
      char buffer[NETWORK_BUFFER_CLIENT_SIZE] = {0};
      guard(read(currentSocket, buffer, NETWORK_BUFFER_CLIENT_SIZE), "error get client message");
      std::cout << "reading client message: end" << std::endl;
      std::string message = buffer;
      if (message != ""){
        std::cout << "length: " << message.size() << std::endl;
        onClientMessage(message);
      }
    }
  }
}

void onNetCode(NetCode& netcode, std::function<void(std::string)> onClientMessage, bool bootstrapperMode){
  maybeGetClientMessage(onClientMessage);
  if (bootstrapperMode){
    tickNetCode(netcode);
  }
}

std::string connectServer(std::string data){
  UdpPacket setup = {
    .type = SETUP,
  };  

  SetupPacket setupPacket {};
  auto packet = toNetworkPacket(setup);

  return connectServerVal(data);
}

