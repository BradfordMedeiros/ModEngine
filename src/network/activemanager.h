#ifndef MOD_ACTIVEMANAGER
#define MOD_ACTIVEMANAGER

#include <string>
#include <map>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
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
#include "../common/util.h"
#include "./network.h"
#include "./servers.h"

std::map<std::string, std::string> listServers();
void connectServer(std::string server);
void disconnectServer();
bool isConnectedToServer();

void sendMessageToActiveServer(std::string data);
void maybeGetClientMessage(std::function<void(std::string)> onClientMessage);

void sendDataOnUdpSocket(UdpPacket packet);
void sendDataOnUdpSocket(std::string data);

void maybeGetUdpClientMessage(std::function<void(UdpPacket)> onClientMessage);

#endif