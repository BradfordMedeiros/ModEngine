#include "./scene/common/mesh.h"
#include "./object_types.h"

std::map<short, GameObjectObj> getObjectMapping() {
	std::map<short, GameObjectObj> objectMapping;
	return objectMapping;
}

Mesh createMesh(std::string field, std::string payload, std::map<std::string, Mesh>& meshes, std::string defaultMesh){
  if (field == ""){
    return  meshes[defaultMesh];
  }
  return meshes[payload];
}

int createCamera(std::string field, std::string payload){
  return 0;
}

void renderMesh(Mesh& mesh){
  drawMesh(mesh);
}
void renderCamera(Mesh& cameraMesh){
  drawMesh(cameraMesh);
}

void addObject(short id, std::string objectType, std::string field, std::string payload, 
  std::map<short, GameObjectObj>& mapping, 
  std::map<std::string, Mesh>& meshes, std::string defaultMesh
){
  if (objectType == "default"){
  	mapping[id] = createMesh(field, payload, meshes, defaultMesh);
  }else if(objectType == "camera"){
    std::cout << "ADDING CAMERA" << std::endl;
  	mapping[id] = createCamera(field, payload);
  }else{
    std::cout << "ERROR: error object type " << objectType << " invalid" << std::endl;
  }
}

void renderObject(short id, std::map<short, GameObjectObj>& mapping, Mesh& cameraMesh, bool showCameras){
	GameObjectObj toRender = mapping[id];

  auto meshObj = std::get_if<Mesh>(&toRender);
	if (meshObj != NULL){
    renderMesh(*meshObj);
    return;
	}

  auto cameraObj = std::get_if<int>(&toRender);
  if (cameraObj != NULL && showCameras){
    renderCamera(cameraMesh);
    return;
  }
}

std::vector<std::pair<std::string, std::string>> serializeMesh(){
  return { std::pair<std::string, std::string>("mesh", "placeholder") };
}
std::vector<std::pair<std::string, std::string>> serializeCamera(){
  return {};    // no additional fields for now
}

std::vector<std::pair<std::string, std::string>> getAdditionalFields(short id, std::map<short, GameObjectObj>& mapping){
  GameObjectObj objectToSerialize = mapping[id];

  auto meshObject = std::get_if<Mesh>(&objectToSerialize);
  if (meshObject != NULL){
    return serializeMesh();
  }

  auto cameraObject = std::get_if<int>(&objectToSerialize);
  if (cameraObject != NULL){
    return serializeCamera();
  }
  return { };   // probably should throw an exception (would be better to rewrite so this cant happen, same in render)
}