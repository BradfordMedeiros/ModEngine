
#include "./object_types.h"

std::map<short, Object> getObjectMapping() {
	std::map<short, Object> objectMapping;
	return objectMapping;
}

void addObject(short id, std::string objectType, std::map<short, Object>& mapping, std::map<std::string, Mesh>& meshes){
  std::cout << "INFO: adding object: " << id << "  type is:  " << objectType << std::endl;  
  ObjectType activeType = ERROR;
  if (objectType == "mesh"){
  	activeType = MESH;
  }else if(objectType == "camera"){
  	activeType == CAMERA;
  }

  mapping[id] = Object {
  	.activeType = activeType,
  };
}

void renderObject(short id, std::map<short, Object>& mapping){
	Object toRender = mapping[id];
	if (toRender.activeType == MESH){
		std::cout << "render mesh placeholder" << std::endl;
	}
	if (toRender.activeType == CAMERA){
		std::cout << "render camera placeholder" << std::endl;
	}
}
/*
fields {
	.additionalFields = { "name", "string" },
	.prefix = '',
	.type = "mesh",
}

.fields {
	.additionalFields = { 'active', 'bool' },
	.prefix = '>',
	.type = 'camera',
}
*/
/*
{
	additionalFields: [{ name: 'mesh', type: 'string' }, { name: 'color', type: 'vec3' }],
}

{	
	prefix: '>'
	additionalFields: [{ name: 'active', type: 'bool' }]
}
*/
