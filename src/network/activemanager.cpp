#include "./activemanager.h"

static bool isConnected = false;
static std::string currentServer = "";
static std::string bootstrapperServer = "127.0.0.1";
static int bootstrapperPort = 8000;

#define NETWORK_BUFFER_CLIENT_SIZE 1024

std::string sendMessage(std::string ip, int port, const char* networkBuffer){
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
  write(sockFd, networkBuffer, strlen(networkBuffer));

  char buffer[NETWORK_BUFFER_CLIENT_SIZE] = {0};
  guard(read(sockFd, buffer, NETWORK_BUFFER_CLIENT_SIZE), "error reading message");
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
  return parseListServerRequest(sendMessage(bootstrapperServer, bootstrapperPort, "list-servers"));
}

void connectServer(std::string server){
  std::cout << "connect server placeholder" << std::endl;
}
void disconnectServer(){
  isConnected = false;
  currentServer = "";
}