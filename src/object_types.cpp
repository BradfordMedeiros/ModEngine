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

void addObject(short id, std::string objectType, std::string field, std::string payload, std::map<short, Object>& mapping, std::map<std::string, Mesh>& meshes, std::string defaultMesh){
  if (objectType == "default"){
  	mapping[id] = createMesh(field, payload, meshes, defaultMesh);
  }else if(objectType == "camera"){
  	mapping[id] = createCamera(field, payload);
  }else{
    std::cout << "ERROR: error object type " << objectType << " invalid" << std::endl;
  }
}

void renderObject(short id, std::map<short, Object>& mapping){
	Object toRender = mapping[id];
	if (toRender.activeType == MESH){
    renderMesh(toRender.obj);
	}
}
