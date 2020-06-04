#ifndef MOD_SERVERS
#define MOD_SERVERS

#include <map>
#include <string>
#include <iostream>
#include "../common/util.h"
#include "./network.h"

struct ServerBrowser {
  std::map<std::string, std::string> serverNameToIp;
};  

void launchServer();

#endif