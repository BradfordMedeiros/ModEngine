#include "./object_types.h"

std::map<short, GameObjectObj> getObjectMapping() {
	std::map<short, GameObjectObj> objectMapping;
	return objectMapping;
}

GameObjectMesh createMesh(std::map<std::string, std::string> additionalFields, std::map<std::string, Mesh>& meshes, std::string defaultMesh, std::function<bool(std::string)> ensureMeshLoaded){
  bool usesMultipleMeshes = additionalFields.find("meshes") != additionalFields.end();

  std::vector<std::string> meshNames;
  std::vector<Mesh> meshesToRender;

  if (usesMultipleMeshes){
    auto meshStrings = split(additionalFields.at("meshes"), ',');
    for (auto meshName : meshStrings){
      bool loadedMesh = ensureMeshLoaded(meshName);
      if (loadedMesh){
        meshNames.push_back(meshName);
        meshesToRender.push_back(meshes.at(meshName));  
      }
    }
  }else{
    auto meshName = (additionalFields.find("mesh") != additionalFields.end()) ? additionalFields.at("mesh") : defaultMesh;
    meshName = (meshName == "") ? defaultMesh : meshName;
    bool loadedMesh = ensureMeshLoaded(meshName);
    if (loadedMesh){
      meshNames.push_back(meshName);
      meshesToRender.push_back(meshes.at(meshName));   
    }
  }

  GameObjectMesh obj {
    .meshNames = meshNames,
    .meshesToRender = meshesToRender,
    .isDisabled = additionalFields.find("disabled") != additionalFields.end(),
    .nodeOnly = meshNames.size() == 0
  };
  return obj;
}
GameObjectCamera createCamera(){
  GameObjectCamera obj {};
  return obj;
}
GameObjectSound createSound(std::map<std::string, std::string> additionalFields, std::function<void(std::string)> loadClip){
  GameObjectSound obj {
    .clip = additionalFields.at("clip")
  };
  loadClip(obj.clip);
  return obj;
}
GameObjectLight createLight(std::map<std::string, std::string> additionalFields){
  auto color = additionalFields.find("color") == additionalFields.end() ? glm::vec3(1.f, 1.f, 1.f) : parseVec(additionalFields.at("color"));
  GameObjectLight obj {
    .color = color,
  };
  return obj;
}

GameObjectVoxel createVoxel(std::map<std::string, std::string> additionalFields, std::function<void()> onVoxelBoundInfoChanged){
  auto voxel = createVoxels(parseVoxelState(additionalFields.at("from")), onVoxelBoundInfoChanged);
  GameObjectVoxel obj {
    .voxel = voxel,
  };
  return obj;
}

GameObjectChannel createChannel(std::map<std::string, std::string> additionalFields){
  bool hasFrom = additionalFields.find("from") != additionalFields.end();
  bool hasTo = additionalFields.find("to") != additionalFields.end();

  GameObjectChannel obj {
    .from = hasFrom ? additionalFields.at("from") : "",
    .to = hasTo ? additionalFields.at("to") : "",
    .complete = hasFrom && hasTo,
  };
  return obj;
}

GameObjectRail createRail(short id, std::map<std::string, std::string> additionalFields, std::function<void(short id, std::string from, std::string to)> createRail){
  RailConnection connection {
    .from = additionalFields.at("from"),
    .to = additionalFields.at("to")
  };
  GameObjectRail obj {
    .id = id,
    .connection = connection,
  };
  createRail(id, connection.from, connection.to);
  return obj;
}

void addObject(
  short id, 
  std::string objectType, 
  std::map<std::string, std::string> additionalFields,
  std::map<short, GameObjectObj>& mapping, 
  std::map<std::string, Mesh>& meshes, 
  std::string defaultMesh, 
  std::function<void(std::string)> loadClip,
  std::function<bool(std::string)> ensureMeshLoaded,
  std::function<void()> onVoxelBoundInfoChanged,
  std::function<void(short id, std::string from, std::string to)> addRail
){
  if (objectType == "default"){
    mapping[id] = createMesh(additionalFields, meshes, defaultMesh, ensureMeshLoaded);
  }else if(objectType == "camera"){
    mapping[id] = createCamera();
  }else if(objectType == "sound"){
    mapping[id] = createSound(additionalFields, loadClip);
  }else if(objectType == "light"){
    mapping[id] = createLight(additionalFields);
  }else if(objectType == "voxel"){
    mapping[id] = createVoxel(additionalFields, onVoxelBoundInfoChanged);
  }else if(objectType == "channel"){
    mapping[id] = createChannel(additionalFields);
  }else if(objectType == "rail"){
    mapping[id] = createRail(id, additionalFields, addRail);
  }else{
    std::cout << "ERROR: error object type " << objectType << " invalid" << std::endl;
    assert(false);
  }
}
void removeObject(std::map<short, GameObjectObj>& mapping, short id, std::function<void(std::string)> unloadClip, std::function<void()> removeRail){
  // @TODO - handle resource cleanup better here eg unload meshes
  auto Object = mapping.at(id); 
  auto soundObj = std::get_if<GameObjectSound>(&Object);
  if (soundObj != NULL){
    unloadClip(soundObj -> clip); 
  }
  auto railObj = std::get_if<GameObjectRail>(&Object);
  if (railObj != NULL){
    removeRail();
  }
  mapping.erase(id);
}

void renderObject(
  GLint shaderProgram, 
  short id, 
  std::map<short, GameObjectObj>& mapping, 
  Mesh& nodeMesh,
  Mesh& cameraMesh, 
  bool showDebug, 
  bool showBoneWeight,
  bool useBoneTransform
){
  GameObjectObj& toRender = mapping.at(id);
  auto meshObj = std::get_if<GameObjectMesh>(&toRender);
  if (meshObj != NULL && !meshObj -> isDisabled && !meshObj ->nodeOnly){
    for (auto meshToRender : meshObj -> meshesToRender){
      bool hasBones = false;
      if (meshToRender.bones.size() > 0){
        for (int i = 0; i < 100; i++){
          auto boneUniformLocation = glGetUniformLocation(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str());
          if (i >= meshToRender.bones.size()){
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
          }else{
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(meshToRender.bones.at(i).offsetMatrix));
          }
        }
        hasBones = true;
      }

      glUniform1i(glGetUniformLocation(shaderProgram, "showBoneWeight"), showBoneWeight);
      glUniform1i(glGetUniformLocation(shaderProgram, "useBoneTransform"), useBoneTransform);
      glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), hasBones);     
      drawMesh(meshToRender, shaderProgram);    
    }
    return;
  }

  if (meshObj != NULL && meshObj -> nodeOnly && showDebug) {
    glUniform1i(glGetUniformLocation(shaderProgram, "showBoneWeight"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "useBoneTransform"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), false);     
    drawMesh(nodeMesh, shaderProgram);    
  }

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL && showDebug){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), cameraMesh.bones.size() > 0);
    drawMesh(cameraMesh, shaderProgram);
    return;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL && showDebug){   // @TODO SH0W CAMERAS SHOULD BE SHOW DEBUG, AND WE SHOULD HAVE SEPERATE MESH TYPE FOR LIGHTS AND NOT REUSE THE CAMERA
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    drawMesh(nodeMesh, shaderProgram);
    return;
  }

  auto voxelObj = std::get_if<GameObjectVoxel>(&toRender);
  if (voxelObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), voxelObj -> voxel.mesh.bones.size() > 0);
    drawMesh(voxelObj -> voxel.mesh, shaderProgram);
    return;
  }

  auto channelObj = std::get_if<GameObjectChannel>(&toRender);
  if (channelObj != NULL && showDebug){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    drawMesh(nodeMesh, shaderProgram);
    return;
  }

  auto railObj = std::get_if<GameObjectRail>(&toRender);
  if (railObj != NULL && showDebug){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), nodeMesh.bones.size() > 0);
    drawMesh(nodeMesh, shaderProgram);
    return; 
  }
}

std::map<std::string, std::string> objectAttributes(std::map<short, GameObjectObj>& mapping, short id){
  std::map<std::string, std::string> attributes;

  GameObjectObj& toRender = mapping.at(id);
  auto meshObj = std::get_if<GameObjectMesh>(&toRender);
  if (meshObj != NULL){
    if (meshObj -> meshNames.size() > 0){
      attributes["mesh"] = meshObj -> meshNames.at(0);
    }
    attributes["isDisabled"] = meshObj -> isDisabled ? "true": "false";
    return attributes;
  }

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL){
    return attributes;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL){   
    attributes["color"] = print(lightObj -> color);
    return attributes;
  }

  auto voxelObj = std::get_if<GameObjectVoxel>(&toRender);
  if (voxelObj != NULL){
    // not yet implemented
    return attributes;
  }

  auto soundObj = std::get_if<GameObjectSound>(&toRender);
  if (soundObj != NULL){
    attributes["clip"] = soundObj -> clip;
    return attributes;
  }

  auto channelObj = std::get_if<GameObjectChannel>(&toRender);
  if (channelObj != NULL){
    attributes["from"] = channelObj -> from;
    attributes["to"] = channelObj -> to;
    return attributes;
  }
  return attributes;
}
void setObjectAttributes(std::map<short, GameObjectObj>& mapping, short id, std::map<std::string, std::string> attributes){
 GameObjectObj& toRender = mapping.at(id);
  auto meshObj = std::get_if<GameObjectMesh>(&toRender);
  if (meshObj != NULL){
    if (attributes.find("isDisabled") != attributes.end()){
      meshObj -> isDisabled = attributes.at("isDisabled") == "true";;
    }
    return;
  }

  auto cameraObj = std::get_if<GameObjectChannel>(&toRender);
  if (cameraObj != NULL){
    if (attributes.find("to") != attributes.end()){
      cameraObj -> to = attributes.at("to");
    }
    if (attributes.find("from") != attributes.end()){
      cameraObj -> from = attributes.at("from");
    }
    return;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL){   
    lightObj -> color = parseVec(attributes.at("color"));
    return;
  }

}

std::vector<std::pair<std::string, std::string>> serializeMesh(GameObjectMesh obj){
  // @TODO fix scene serialization, only serializing first mesh right now, which is wrong
  return { std::pair<std::string, std::string>("mesh", obj.meshNames.at(0)) };
}
std::vector<std::pair<std::string, std::string>> defaultSerialization(){
  return {}; 
}   

std::vector<std::pair<std::string, std::string>> getAdditionalFields(short id, std::map<short, GameObjectObj>& mapping){
  GameObjectObj objectToSerialize = mapping.at(id);
  auto meshObject = std::get_if<GameObjectMesh>(&objectToSerialize);
  if (meshObject != NULL){
    return serializeMesh(*meshObject);
  }
  return defaultSerialization();
}

std::vector<short> getGameObjectsIndex(std::map<short, GameObjectObj>& mapping){
  std::vector<short> indicies;
  for (auto [id, _]: mapping){    
      indicies.push_back(id);
  }
  return indicies;
}

NameAndMesh getMeshesForId(std::map<short, GameObjectObj>& mapping, short id){  
  std::vector<std::reference_wrapper<std::string>> meshNames;
  std::vector<std::reference_wrapper<Mesh>> meshes;

  GameObjectObj& gameObj = mapping.at(id);
  auto meshObject = std::get_if<GameObjectMesh>(&gameObj);

  if (meshObject != NULL){
    for (int i = 0; i < meshObject -> meshesToRender.size(); i++){
      meshNames.push_back(meshObject -> meshNames.at(i));
      meshes.push_back(meshObject -> meshesToRender.at(i));
    }
  }

  NameAndMesh meshData {
    .meshNames = meshNames,
    .meshes = meshes
  };

  assert(meshNames.size() == meshes.size());
  return meshData;
}

std::vector<std::string> getMeshNames(std::map<short, GameObjectObj>& mapping, short id){
  std::vector<std::string> names;
  if (id == -1){
    return names;
  }
  for (auto name : getMeshesForId(mapping, id).meshNames){
    names.push_back(name);
  }

  return names;
}

std::map<std::string, std::vector<std::string>> getChannelMapping(std::map<short, GameObjectObj>& mapping){
  std::map<std::string, std::vector<std::string>> channelMapping;
  for (auto &[_, obj] : mapping){
    auto channelObj = std::get_if<GameObjectChannel>(&obj);
    if (channelObj != NULL && channelObj -> complete){
      std::vector<std::string> toChannels;
      if (channelMapping.find(channelObj -> from) == channelMapping.end()){
        channelMapping[channelObj -> from] = toChannels;
      }
      channelMapping[channelObj -> from].push_back(channelObj -> to);   
    } 
  }
  return channelMapping;
}


std::map<short, RailConnection> getRails(std::map<short, GameObjectObj>& mapping){
  std::map<short, RailConnection> connections;
  for (auto [_, obj] : mapping){
    auto railObj = std::get_if<GameObjectRail>(&obj);
    if (railObj != NULL){
      connections[railObj -> id] = railObj -> connection;
    }
  }
  return connections;
}