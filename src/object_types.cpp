#include "./scene/common/mesh.h"
#include "./object_types.h"

std::map<short, GameObjectObj> getObjectMapping() {
	std::map<short, GameObjectObj> objectMapping;
	return objectMapping;
}

GameObjectMesh createMesh(std::string field, std::string payload, std::map<std::string, Mesh>& meshes, std::string defaultMesh, std::function<void(std::string)> ensureMeshLoaded){
  std::cout << "Creating gameobject: mesh: " << payload << std::endl;

  std::string meshName;
  if (field == ""){
    meshName = defaultMesh;
  }else{
    meshName = payload;
  }
  
  ensureMeshLoaded(meshName);
  if (meshes.find(meshName) == meshes.end()){
    std::cout << "ERROR: loading mesh " << meshName << " does not exist" << std::endl; 
    throw std::runtime_error("mesh does not exist: " + meshName);
  }

  GameObjectMesh obj = {
    .meshName = meshName,
    .mesh = meshes[meshName],
  };
  return obj;
}
GameObjectCamera createCamera(std::string field, std::string payload){
  GameObjectCamera obj = {};
  return obj;
}

void renderMesh(GameObjectMesh& obj){
  drawMesh(obj.mesh);
}
void renderCamera(Mesh& cameraMesh){
  drawMesh(cameraMesh);
}

void addObject(short id, std::string objectType, std::string field, std::string payload, 
  std::map<short, GameObjectObj>& mapping, 
  std::map<std::string, Mesh>& meshes, std::string defaultMesh, std::function<void(std::string)> ensureMeshLoaded
){
  if (objectType == "default"){
  	mapping[id] = createMesh(field, payload, meshes, defaultMesh, ensureMeshLoaded);
  }else if(objectType == "camera"){
  	mapping[id] = createCamera(field, payload);
  }else{
    std::cout << "ERROR: error object type " << objectType << " invalid" << std::endl;
  }
}
void removeObject(std::map<short, GameObjectObj>& mapping, short id){
  mapping.erase(id);
}

void renderObject(short id, std::map<short, GameObjectObj>& mapping, Mesh& cameraMesh, bool showCameras){
	GameObjectObj toRender = mapping[id];

  auto meshObj = std::get_if<GameObjectMesh>(&toRender);
	if (meshObj != NULL){
    renderMesh(*meshObj);
    return;
	}

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL && showCameras){
    renderCamera(cameraMesh);
    return;
  }
}

std::vector<std::pair<std::string, std::string>> serializeMesh(GameObjectMesh obj){
  return { std::pair<std::string, std::string>("mesh", obj.meshName) };
}
std::vector<std::pair<std::string, std::string>> serializeCamera(){
  return {};    // no additional fields for now
}

std::vector<std::pair<std::string, std::string>> getAdditionalFields(short id, std::map<short, GameObjectObj>& mapping){
  GameObjectObj objectToSerialize = mapping[id];

  auto meshObject = std::get_if<GameObjectMesh>(&objectToSerialize);
  if (meshObject != NULL){
    return serializeMesh(*meshObject);
  }

  auto cameraObject = std::get_if<GameObjectCamera>(&objectToSerialize);
  if (cameraObject != NULL){
    return serializeCamera();
  }
  return { };   // probably should throw an exception (would be better to rewrite so this cant happen, same in render)
}