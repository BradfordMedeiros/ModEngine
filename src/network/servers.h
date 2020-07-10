#ifndef MOD_SERVERS
#define MOD_SERVERS

#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include "../common/util.h"
#include "./network.h"


// Move this out of the networking code, it doesnt belong here
enum PacketType { CREATE, DELETE, UPDATE, SETUP, LOAD };
struct CreatePacket {
  short id;
};
struct DeletePacket {
  short id;
};
struct UpdatePacket {
  short id;
};

// @TODO this should optimize so only send the size necessary, not max size since scnee file needs to be larger
struct LoadPacket {
  char sceneData[4000]; // this makes every message 4k, which is probably way bigger than each packet needs to be, optimize this
};
union PacketPayload {
  CreatePacket createpacket;
  DeletePacket deletepacket;
  UpdatePacket updatepacket;
  LoadPacket loadpacket;
};
struct UdpPacket {
  PacketType type;
  PacketPayload payload;
};

//////////////////////

struct ServerBrowser {
  std::map<std::string, std::string> serverNameToIp;
};  

struct tcpServer {
  ServerBrowser browser;
  modsocket server;
  std::map<std::string, ConnectionInfo> connections;
};

struct NetCode {
  tcpServer tServer;
  udpmodsocket udpModsocket;
  std::map<std::string, sockaddr_in> udpConnections;
  std::function<void(std::string)> onPlayerConnected;
  std::function<void(std::string)> onPlayerDisconnected;
};
NetCode initNetCode(std::function<void(std::string)> onPlayerConnected, std::function<void(std::string)> onPlayerDisconnected);
void tickNetCode(NetCode& netcode);
void sendUdpPacketToAllUdpClients(NetCode& netcode, UdpPacket data);

#endif