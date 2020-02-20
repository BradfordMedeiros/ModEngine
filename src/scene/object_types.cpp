#include "./object_types.h"

std::map<short, GameObjectObj> getObjectMapping() {
	std::map<short, GameObjectObj> objectMapping;
	return objectMapping;
}

GameObjectMesh createMesh(std::map<std::string, std::string> additionalFields, std::map<std::string, Mesh>& meshes, std::string defaultMesh, std::function<void(std::string)> ensureMeshLoaded){
  bool usesMultipleMeshes = additionalFields.find("meshes") != additionalFields.end();
  bool isDisabled = additionalFields.find("disabled") != additionalFields.end() ; 

  std::vector<std::string> meshNames;
  std::vector<Mesh> meshesToRender;

  if (usesMultipleMeshes){
    auto meshStrings = split(additionalFields.at("meshes"), ',');
    for (auto meshName : meshStrings){
      ensureMeshLoaded(meshName);
      meshesToRender.push_back(meshes.at(meshName));
    }
  }else{
    auto meshName = (additionalFields.find("mesh") != additionalFields.end()) ? additionalFields.at("mesh") : defaultMesh;
    meshName = (meshName == "") ? defaultMesh : meshName;
    ensureMeshLoaded(meshName);
    meshNames.push_back(meshName);
    meshesToRender.push_back(meshes.at(meshName));
  }

  GameObjectMesh obj {
    .meshNames = meshNames,
    .meshesToRender = meshesToRender,
    .isDisabled = isDisabled,
  };

  return obj;
}
GameObjectCamera createCamera(){
  GameObjectCamera obj {};
  return obj;
}
GameObjectLight createLight(){
  GameObjectLight obj {};
  return obj;
}

GameObjectVoxel createVoxel(std::map<std::string, std::string> additionalFields, std::function<void()> onVoxelBoundInfoChanged){
  auto voxel = createVoxels(parseVoxelState(additionalFields.at("from")), onVoxelBoundInfoChanged);
  GameObjectVoxel obj {
    .voxel = voxel,
  };
  return obj;
}

void addObject(
  short id, 
  std::string objectType, 
  std::map<std::string, std::string> additionalFields,
  std::map<short, GameObjectObj>& mapping, 
  std::map<std::string, Mesh>& meshes, 
  std::string defaultMesh, 
  std::function<void(std::string)> ensureMeshLoaded,
  std::function<void()> onVoxelBoundInfoChanged
){
  if (objectType == "default"){
    mapping[id] = createMesh(additionalFields, meshes, defaultMesh, ensureMeshLoaded);
  }else if(objectType == "camera"){
    mapping[id] = createCamera();
  }else if (objectType == "light"){
    mapping[id] = createLight();
  }else if (objectType == "voxel"){
    mapping[id] = createVoxel(additionalFields, onVoxelBoundInfoChanged);
  }else{
    std::cout << "ERROR: error object type " << objectType << " invalid" << std::endl;
  }
}
void removeObject(std::map<short, GameObjectObj>& mapping, short id){
  mapping.erase(id);
}

void renderObject(GLint shaderProgram, short id, std::map<short, GameObjectObj>& mapping, Mesh& cameraMesh, bool showBoundingBoxForMesh, Mesh& boundingBoxMesh, bool showCameras){
  GameObjectObj& toRender = mapping.at(id);

  auto meshObj = std::get_if<GameObjectMesh>(&toRender);
  if (meshObj != NULL && !meshObj->isDisabled){
    for (auto meshToRender : meshObj -> meshesToRender){
      if (meshToRender.bones.size() > 0){
        auto boneMatrix = glm::mat4(1.f);
        //meshToRender.bones.at
        for (int i = 0; i < 20; i++){ // 20 = hardcoded number of bones
          glUniformMatrix4fv(glGetUniformLocation(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str()), 1, GL_FALSE, glm::value_ptr(boneMatrix));
        }
        glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), true);
      }else{
        glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), false);
      }

      drawMesh(meshToRender);    
    }
    return;
  }

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL && showCameras){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), cameraMesh.bones.size() > 0);
    drawMesh(cameraMesh);
    return;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL && showCameras){   // @TODO SH0W CAMERAS SHOULD BE SHOW DEBUG, AND WE SHOULD HAVE SEPERATE MESH TYPE FOR LIGHTS AND NOT REUSE THE CAMERA
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), cameraMesh.bones.size() > 0);
    drawMesh(cameraMesh);
    return;
  }

  auto voxelObj = std::get_if<GameObjectVoxel>(&toRender);
  if (voxelObj != NULL){
    glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), voxelObj -> voxel.mesh.bones.size() > 0);
    drawMesh(voxelObj -> voxel.mesh);
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
  GameObjectObj objectToSerialize = mapping[id];
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
