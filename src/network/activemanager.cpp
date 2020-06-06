#include "./activemanager.h"

static bool isConnected = false;
static std::string currentServer = "";
static std::string bootstrapperServer = "127.0.0.1";
static int bootstrapperPort = 8000;

void sendMessage(std::string ip, int port, const char* networkBuffer){
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
  close(sockFd);
}


std::map<std::string, std::string> listServers(){
  std::map<std::string, std::string> serverNameToIp;
//  sendMessage(bootstrapperServer, bootstrapperPort, "list-servers");

  serverNameToIp["no-mans-world"] = "1";
  serverNameToIp["brads-server"] = "2";
  return serverNameToIp;
}

void connectServer(){


}
void disconnectServer(){
  isConnected = false;
  currentServer = "";
}