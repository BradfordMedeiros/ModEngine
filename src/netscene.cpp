#include "./netscene.h"

void copyStr(std::string& data, char* copyTo, int size){
  auto strdata = data.c_str();
  assert((sizeof(strdata) + 1 ) < size);
  strncpy(copyTo, strdata, size);
  assert(copyTo[size -1] == '\0');
}

NetworkPacket toNetworkPacket(UdpPacket& packet){
  NetworkPacket netpacket {
    .packet = &packet,
    .packetSize = sizeof(packet),
  };
  return netpacket;
}

void netObjectCreate(World& world, GameObject& obj, NetCode& netcode, bool bootstrapperMode){
  if (!obj.netsynchronize){
    return;
  }

  std::cout << "created obj id: " << obj.id << std::endl;
  UdpPacket packet { .type = CREATE };

  packet.payload.createpacket = CreatePacket { 
    .id = obj.id,
    .sceneId = getGameObjectH(world.sandbox, obj.id).sceneId,
  };
  auto serialobj = serializeObject(world, obj.id);
  if (serialobj == ""){
    return; // "" is sentinal, that specifies that the group id != the id, which we do not send over a network.  This needs to be more explicit
  }

  copyStr(serialobj, packet.payload.createpacket.serialobj, sizeof(packet.payload.createpacket.serialobj));

  if (bootstrapperMode){
    sendUdpPacketToAllUdpClients(netcode, toNetworkPacket(packet));
  }else if (isConnectedToServer()){
    sendDataOnUdpSocket(toNetworkPacket(packet));
  } 
}