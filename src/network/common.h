#ifndef MOD_NETCOMMON
#define MOD_NETCOMMON

#include <stdexcept>

void guard(int value, const char* runtimeErrorMessage);

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

struct NetworkPacket {
  void* packet;
  unsigned int packetSize;
};

NetworkPacket toNetworkPacket(UdpPacket& packet);


#endif