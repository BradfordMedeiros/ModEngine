#ifndef MOD_SERVERS
#define MOD_SERVERS

#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include "../common/util.h"
#include "./network.h"

struct ServerBrowser {
  std::map<std::string, std::string> serverNameToIp;
  std::vector<std::string> activeConnections;
};  

void launchServer(std::function<void(std::string)> onTcpData);

#endif