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

void netObjectCreate(World& world, GameObject& obj, NetCode& netcode, bool bootstrapperMode);
void netObjectUpdate(World& world, GameObject& obj, NetCode& netcode, bool bootstrapperMode);
void netObjectDelete(objid id, bool isNet, NetCode& netcode, bool bootstrapperMode);

void onNetCode(World& world, SysInterface& interface, NetCode& netcode, std::function<void(std::string)> onClientMessage, bool bootstrapperMode);

void sendDataUdp(std::string data);
std::string connectServer(std::string data);

void sendLoadScene(World& world, NetCode& netcode, bool bootStrapperMode, int32_t id);

#endif