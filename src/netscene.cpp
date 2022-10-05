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
  auto serialobj = serializeObject(world, obj.id, true);
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

void netObjectUpdate(World& world, GameObject& obj, NetCode& netcode, bool bootstrapperMode){
  if (!obj.netsynchronize){   
    return;
  }
  UdpPacket packet { .type = UPDATE };
  packet.payload.updatepacket = UpdatePacket { 
    .id = obj.id,
    .properties = getProperties(world, obj.id),
  };
  if (bootstrapperMode){
    sendUdpPacketToAllUdpClients(netcode, toNetworkPacket(packet));
  }else if (isConnectedToServer()){
    sendDataOnUdpSocket(toNetworkPacket(packet));
  }   
}

void netObjectDelete(objid id, bool isNet, NetCode& netcode, bool bootstrapperMode) {
  if (!isNet){
    return;
  }

  UdpPacket packet { .type = DELETE };
  packet.payload.deletepacket =  DeletePacket { .id = id };
  if (bootstrapperMode){
    sendUdpPacketToAllUdpClients(netcode, toNetworkPacket(packet));
  }else if (isConnectedToServer()){
    sendDataOnUdpSocket(toNetworkPacket(packet));
  }
}

int32_t makeObject(World& world, std::string serializedobj, objid id, bool useObjId, objid sceneId, bool useSceneId){
  auto firstSceneId = allSceneIds(world.sandbox, std::nullopt).at(0);
  return addObjectToScene(world, useSceneId ? sceneId : firstSceneId, serializedobj, id, useObjId);
}

 // @TODO --  this needs to makeObject in the right scene
void handleCreate(World& world, UdpPacket& packet){
  auto create = packet.payload.createpacket;
  if (!sceneExists(world.sandbox, packet.payload.createpacket.sceneId)){
    return;
  }

  auto id = create.id;   
  if (idExists(world.sandbox, id)){     // could conceptually do a comparison to see if it changed, but probably not
    std::cout << "INFO: id already exits: " << id << std::endl;
    return;
  }
  std::string serialobj = create.serialobj;
  assert(serialobj.size() > 0);
  auto newObjId = makeObject(world, serialobj, create.id, true, packet.payload.createpacket.sceneId, true);                        
  assert(newObjId == id);
}

void handleUpdate(World& world, UdpPacket& packet){
  auto update = packet.payload.updatepacket;

  if (idExists(world.sandbox, update.id)){
    setProperties(world, update.id, update.properties);
  }else{
    std::cout << "WARNING: Udp client update: does not exist " << update.id << std::endl;
  }
}

void handleDelete(World& world, UdpPacket& packet){
  auto deletep = packet.payload.deletepacket;
  if (idExists(world.sandbox, deletep.id)){
    std::cout << "UDP CLIENT MESSAGE: DELETING: " << deletep.id << std::endl;
    removeObjectFromScene(world, deletep.id);
  }else{
    std::cout << "UDP CLIENT MESSAGE: ID NOT EXIST: " << deletep.id << std::endl;
  }
}

void onUdpServerMessage(World& world, UdpPacket& packet){
  if (packet.type == SETUP){
    std::cout << "INFO: SETUP PACKET HANDLED IN SERVER CODE" << packet.payload.setuppacket.connectionHash << std::endl;
  }else if (packet.type == LOAD){
    std::cout << "WARNING: LOAD message server, ignoring" << std::endl;
  }else if (packet.type == UPDATE){
    handleUpdate(world, packet);
  }else if (packet.type == CREATE){
    handleCreate(world, packet);
  }else if (packet.type == DELETE){
    handleDelete(world, packet);
  }else {
    std::cout << "ERROR: unknown packet type" << std::endl;
  }
}

void onUdpClientMessage(World& world, UdpPacket& packet){
  std::cout << "INFO: GOT UDP CLIENT MESSAGE" << std::endl;
  if (packet.type == SETUP){
    std::cout << "WARNING: should not get setup packet type" << std::endl;
  }else if (packet.type == LOAD){
    modassert(false, "should be preserving the names, tags of the scene");
    addSceneToWorldFromData(world, "", packet.payload.loadpacket.sceneId, packet.payload.loadpacket.sceneData, std::nullopt, std::nullopt); 
  }else if (packet.type == UPDATE){
    handleUpdate(world, packet);
  }else if (packet.type == CREATE){
    handleCreate(world, packet);
  }else if (packet.type == DELETE){
    handleDelete(world, packet);
  }
  //schemeBindings.onUdpMessage(message);
}

void onNetCode(World& world, NetCode& netcode, std::function<void(std::string)> onClientMessage, bool bootstrapperMode){
  maybeGetClientMessage(onClientMessage);
  UdpPacket udpPacket { };
  auto hasClientMessage = maybeGetUdpClientMessage(&udpPacket, sizeof(udpPacket));
  if (hasClientMessage){
    onUdpClientMessage(world, udpPacket);
  }
  if (bootstrapperMode){
    UdpPacket packet { };
    auto networkPacket = toNetworkPacket(packet);
    bool udpPacketHasData = tickNetCode(netcode, networkPacket, [&packet]() -> std::string {
      if (packet.type == SETUP){
        return packet.payload.setuppacket.connectionHash;          
      }
      return "";
    });
    if (udpPacketHasData){
      onUdpServerMessage(world, packet);
    }
  }
}

void sendDataUdp(std::string data){
  UdpPacket packet {
    .type = CREATE,
  };
  sendDataOnUdpSocket(toNetworkPacket(packet));
}

std::string connectServer(std::string data){
  UdpPacket setup = {
    .type = SETUP,
  };  

  SetupPacket setupPacket {};
  auto packet = toNetworkPacket(setup);

  return connectServer(data, [&setup, &setupPacket, &packet](std::string connectionHash) -> NetworkPacket {
    auto data = connectionHash.c_str();
    assert((sizeof(data) + 1 ) < sizeof(setupPacket.connectionHash));
    strncpy(setupPacket.connectionHash, data, sizeof(setupPacket.connectionHash));
    assert(setupPacket.connectionHash[sizeof(setupPacket.connectionHash) -1] == '\0');
    setup.payload.setuppacket = setupPacket;
    return packet;
  });
}

void sendLoadScene(World& world, NetCode& netcode, bool bootStrapperMode, int32_t id){
  if (!bootStrapperMode){
    std::cout << "ERROR: cannot send load scene in not-server mode" << std::endl;
    assert(false);
  }

  std::string sceneData = serializeScene(world, id, true);
  UdpPacket packet { .type = LOAD };
  auto data = sceneData.c_str();
  LoadPacket loadpacket {
    .sceneId = id,
  };
  assert((sizeof(data) + 1 ) < sizeof(loadpacket.sceneData));
  strncpy(loadpacket.sceneData, data, sizeof(loadpacket.sceneData));
  assert(loadpacket.sceneData[sizeof(loadpacket.sceneData) -1] == '\0');
  packet.payload.loadpacket = loadpacket; 
  sendUdpPacketToAllUdpClients(netcode, toNetworkPacket(packet));
}