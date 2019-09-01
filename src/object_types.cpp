#include "./scene/common/mesh.h"
#include "./object_types.h"

std::map<short, Object> getObjectMapping() {
	std::map<short, Object> objectMapping;
	return objectMapping;
}

Object createMesh(std::string field, std::string payload, std::map<std::string, Mesh>& meshes, std::string defaultMesh){
  Objects obj { };
  if (field == ""){
    obj.mesh = meshes[defaultMesh];
  }else{
    obj.mesh = meshes[payload];
  }
  
  Object objData = {
  	.activeType = MESH,
  	.obj = obj, 
  };
  return objData;
}

Object createCamera(std::string field, std::string payload){
  Objects obj = {
  	.cameraPlaceholder = 4,
  };
  Object objData = {
  	.activeType = CAMERA,
  	.obj = obj, 
  };
  return objData;
}

void renderMesh(Objects &obj){
  drawMesh(obj.mesh);
}
void renderCamera(Mesh& cameraMesh){
  drawMesh(cameraMesh);
}

void addObject(short id, std::string objectType, std::string field, std::string payload, 
  std::map<short, Object>& mapping, 
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

void renderObject(short id, std::map<short, Object>& mapping, Mesh& cameraMesh, bool showCameras){
	Object toRender = mapping[id];
	if (toRender.activeType == MESH){
    renderMesh(toRender.obj);
	}else if (showCameras && toRender.activeType == CAMERA){
    renderCamera(cameraMesh);
  }
}

std::vector<std::pair<std::string, std::string>> serializeMesh(){
  return { std::pair<std::string, std::string>("mesh", "placeholder") };
}
std::vector<std::pair<std::string, std::string>> serializeCamera(){
  return {};    // no additional fields for now
}

std::vector<std::pair<std::string, std::string>> getAdditionalFields(short id, std::map<short, Object>& mapping){
  Object objectToSerialize = mapping[id];
  if (objectToSerialize.activeType == MESH){
    return serializeMesh();
  }else if (objectToSerialize.activeType == CAMERA){
    return serializeCamera();
  }
  return { };   // probably should throw an exception (would be better to rewrite so this cant happen, same in render)
}