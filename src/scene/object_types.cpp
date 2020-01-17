#include "./object_types.h"

std::map<short, GameObjectObj> getObjectMapping() {
	std::map<short, GameObjectObj> objectMapping;
	return objectMapping;
}

GameObjectMesh createMesh(std::string field, std::string payload, std::map<std::string, Mesh>& meshes, std::string defaultMesh, 
  std::function<void(std::string)> ensureMeshLoaded, GameObjectMesh& gameobj){
  std::cout << "Creating gameobject: mesh: " << payload << std::endl;

  std::string meshName = (field == "mesh") ? payload : gameobj.meshName;
  meshName = (meshName == "") ? defaultMesh : meshName;
  bool isDisabled = gameobj.isDisabled || (field == "disabled");
  
  ensureMeshLoaded(meshName);
  if (meshes.find(meshName) == meshes.end()){
    std::cout << "ERROR: loading mesh " << meshName << " does not exist" << std::endl; 
    throw std::runtime_error("mesh does not exist: " + meshName);
  }

  GameObjectMesh obj {
    .meshName = meshName,
    .mesh = meshes.at(meshName),
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
GameObjectVoxel createVoxel(){
  GameObjectVoxel obj {};
  return obj;
}

void addObject(
  short id, 
  std::string objectType, 
  std::string field, 
  std::string payload, 
  std::map<short, GameObjectObj>& mapping, 
  std::map<std::string, Mesh>& meshes, 
  std::string defaultMesh, 
  std::function<void(std::string)> ensureMeshLoaded
){
  if (objectType == "default"){
    GameObjectObj existingObject = mapping[id];
    GameObjectMesh* meshObject = std::get_if<GameObjectMesh>(&existingObject);
    mapping[id] = createMesh(field, payload, meshes, defaultMesh, ensureMeshLoaded, *meshObject);
  }else if(objectType == "camera"){
    mapping[id] = createCamera();
  }else if (objectType == "light"){
    mapping[id] = createLight();
  }else if (objectType == "voxel"){
    mapping[id] = createVoxel();
  }

  else{
    std::cout << "ERROR: error object type " << objectType << " invalid" << std::endl;
  }
}
void removeObject(std::map<short, GameObjectObj>& mapping, short id){
  mapping.erase(id);
}

void renderObject(short id, std::map<short, GameObjectObj>& mapping, Mesh& cameraMesh, bool showBoundingBoxForMesh, Mesh& boundingBoxMesh, bool showCameras){
  GameObjectObj toRender = mapping.at(id);

  auto meshObj = std::get_if<GameObjectMesh>(&toRender);
  if (meshObj != NULL && !meshObj->isDisabled){
    drawMesh(meshObj->mesh);
    return;
  }

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL && showCameras){
    drawMesh(cameraMesh);
    return;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL && showCameras){   // @TODO SH0W CAMERAS SHOULD BE SHOW DEBUG, AND WE SHOULD HAVE SEPERATE MESH TYPE FOR LIGHTS AND NOT REUSE THE CAMERA
    drawMesh(cameraMesh);
    return;
  }

  auto voxelObj = std::get_if<GameObjectVoxel>(&toRender);
  if (voxelObj != NULL){
    std::cout << "render voxel placeholder" << std::endl;
    return;
  }
}

std::vector<std::pair<std::string, std::string>> serializeMesh(GameObjectMesh obj){
  return { std::pair<std::string, std::string>("mesh", obj.meshName) };
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
