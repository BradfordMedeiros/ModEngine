#include "./activemanager.h"

static bool isConnected = false;
static std::string currentServerIp = "";
static int currentSocket = -1;

static std::string bootstrapperServer = "127.0.0.1";
static int bootstrapperPort = 8000;

#define NETWORK_BUFFER_CLIENT_SIZE 1024

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

std::map<std::string, std::string> parseListServerRequest(std::string response){
  std::map<std::string, std::string> serverNameToIp;
  auto servers = split(response, '\n');
  for (auto server : servers){
    auto parts = split(server, ' ');
    serverNameToIp[parts.at(0)] = parts.at(1);
  }
  return serverNameToIp;
}

std::map<std::string, std::string> listServers(){
  return parseListServerRequest(sendMessageNewConnection(bootstrapperServer, bootstrapperPort, "list-servers"));
}
void connectServer(std::string server){
  assert(!isConnected);
  auto serverAddress = listServers().at(server);
  auto sockFd = socketConnection(serverAddress, 8000);
  sendMessageWithConnection(sockFd, "connect");
  auto response = readMessageWithConnection(sockFd);
  assert(response == "ack");
  std::cout << "INFO: connection request succeeded" << std::endl;
  isConnected = true;
  currentServerIp = serverAddress;
  currentSocket = sockFd;
}
void disconnectServer(){
  assert(isConnected);
  isConnected = false;
  currentServerIp = "";
}
void sendMessageToActiveServer(std::string data){
  assert(isConnected);
  std::string content = "type:data\n" + data;
  std::cout << "Sending message to active server starting: " << data << std::endl;
  sendMessageWithConnection(currentSocket, content.c_str());
  std::cout << "Sending message to active server complete: " << data << std::endl;
}

bool socketHasDataToRead(int socketFd){
  int count;
  ioctl(currentSocket, FIONREAD, &count);
  return count > 0;
}

void maybeGetClientMessage(std::function<void(std::string)> onClientMessage){
  if (isConnected){
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