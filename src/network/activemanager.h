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

std::unordered_map<std::string, std::string> listServers();
std::string connectServer(std::string server, std::function<NetworkPacket(std::string)> getConnectPacket);
void disconnectServer();
bool isConnectedToServer();

void sendMessageToActiveServer(std::string data);
void maybeGetClientMessage(std::function<void(std::string)> onClientMessage);

void sendDataOnUdpSocket(NetworkPacket packet);

bool maybeGetUdpClientMessage(void* _packet, unsigned int packetSize);

#endif