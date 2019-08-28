
#include "./object_types.h"

std::map<short, Objects> getObjectMapping() {
	std::map<short, Objects> objectMapping;
	return objectMapping;
}

void addObject(short id, std::string objectType, std::map<short, Objects>& mapping){
	if (objectType == "mesh"){
      std::cout << "INFO: adding object: " << id << "  type is:  " << objectType << std::endl;
	}else{
      throw std::runtime_error("object type:  " +objectType + " is invalid");
	}
}
