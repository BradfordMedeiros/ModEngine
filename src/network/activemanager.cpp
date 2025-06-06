#include "./activemanager.h"

#define NETWORK_BUFFER_CLIENT_SIZE 1024

struct udpSetup {
  int udpSocket;
  sockaddr_in servaddr; 
};

static bool isConnected = false;  // static-state
static std::string currentServerIp = "";

static std::string bootstrapperServer = "127.0.0.1";
static int bootstrapperPort = 8000;

// TCP specific 
static int currentSocket = -1;

// UDP specific
static udpSetup setup{ .udpSocket = -1 };


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

std::unordered_map<std::string, std::string> parseListServerRequest(std::string response){
  std::unordered_map<std::string, std::string> serverNameToIp;
  auto servers = split(response, '\n');
  for (auto server : servers){
    auto parts = split(server, ' ');
    serverNameToIp[parts.at(0)] = parts.at(1);
  }
  return serverNameToIp;
}

std::unordered_map<std::string, std::string> listServers(){
  return parseListServerRequest(sendMessageNewConnection(bootstrapperServer, bootstrapperPort, "list-servers"));
}

void sendDataOnUdpSocket(NetworkPacket packet){
  assert(isConnected);
  int numBytes = sendto(setup.udpSocket, packet.packet, packet.packetSize,  MSG_CONFIRM, (const struct sockaddr *) &setup.servaddr, sizeof(setup.servaddr)); 
  if (numBytes == -1){
    throw std::runtime_error("error sending udp data");
  }
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

udpSetup setupUdp(){
  sockaddr_in servaddr {
    .sin_family = AF_INET,
    .sin_port = htons(8001),
    .sin_addr = in_addr{
      .s_addr = htonl(INADDR_ANY),
    }
  }; 
    
  int sockfd = socket(AF_INET, SOCK_DGRAM, 0);  
  if (sockfd == -1){
    throw std::runtime_error("error creating socket");
  }
  udpSetup setup {
    .udpSocket = sockfd,
    .servaddr = servaddr,
  };
  return setup;
}

std::string connectServer(std::string server, std::function<NetworkPacket(std::string)> getConnectPacket){
  assert(!isConnected);

  auto serverAddress = listServers().at(server);

  // TCP Connection 
  auto response = connectTcp(serverAddress);
  currentSocket = response.sockFd;
  std::cout << "connect hash is: " << response.connectionHash << std::endl;

  setup = setupUdp();

  // Statekeeping
  std::cout << "INFO: connection request succeeded" << std::endl;

  std::cout << "INFO: now connected" << std::endl;
  isConnected = true;
  currentServerIp = serverAddress;

  // This is hackey, but needed b/c the server only sends data if the client has first sent a udp message.  Probably should formalize this better.
  sendDataOnUdpSocket(getConnectPacket(response.connectionHash));
  return response.connectionHash;
}

void disconnectServer(){
  assert(isConnected);
  
  sendMessageWithConnection(currentSocket, "disconnect");
  auto response = readMessageWithConnection(currentSocket);
  assert(response == "ack");

  isConnected = false;
  currentServerIp = "";
  setup.udpSocket = -1;
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

bool maybeGetUdpClientMessage(void* _packet, unsigned int packetSize){
  if (setup.udpSocket != -1){
    auto udpData = getDataFromUdpSocket(setup.udpSocket, _packet, packetSize);
    if (udpData.hasData){
      return true;
    }
  }
  return false;
}