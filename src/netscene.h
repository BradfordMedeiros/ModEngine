#ifndef MOD_NETSCENE
#define MOD_NETSCENE

#include "./scene/scene.h"
#include "./network/servers.h"
#include "./network/activemanager.h"

enum PacketType { CREATE, DELETE, UPDATE, SETUP, LOAD };
struct SetupPacket {
  char connectionHash[4000];
};
struct CreatePacket {
  int32_t id;
  objid sceneId;
  char serialobj[3000];
};
struct DeletePacket {
  int32_t id;
};
struct UpdatePacket {
  int32_t id;
  Properties properties;
};

// @TODO this should optimize so only send the size necessary, not max size since scnee file needs to be larger
struct LoadPacket {
  objid sceneId;
  char sceneData[4000]; // this makes every message 4k, which is probably way bigger than each packet needs to be, optimize this
};
union PacketPayload {
  SetupPacket setuppacket;
  CreatePacket createpacket;
  DeletePacket deletepacket;
  UpdatePacket updatepacket;
  LoadPacket loadpacket;
};
struct UdpPacket {
  PacketType type;
  PacketPayload payload;
};

NetworkPacket toNetworkPacket(UdpPacket& packet);

void netObjectCreate(World& world, GameObject& obj, NetCode& netcode, bool bootstrapperMode);

#endif