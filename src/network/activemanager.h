#ifndef MOD_ACTIVEMANAGER
#define MOD_ACTIVEMANAGER

#include <string>
#include <map>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h> 
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h> 
#include <cstring>
#include <errno.h>
#include <vector>
#include <arpa/inet.h>
#include <assert.h>
#include <functional>
#include "./common.h"

std::map<std::string, std::string> listServers();
void connectServer(std::string server);
void disconnectServer();

#endif