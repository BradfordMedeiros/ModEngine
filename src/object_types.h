#ifndef MOD_OBJECTTYPES
#define MOD_OBJECTTYPES

#include <iostream>
#include <map>
#include <stdexcept>
#include "./scene/common/mesh.h"

// idea is to have a union of the types we want + active type in struct, and then a data structure which will map ids (objects that we add to the scene)
// to the proper types 
// this way we can decouple, for example, the camera type, or the concept of a mesh, from our scene graph 

enum ObjectType { ERROR = 0, MESH = 1, CAMERA = 2 } ;

union Objects {
  Mesh object;
  int cameraPlaceholder;
};
struct Object {
  Objects obj;
  ObjectType activeType;
};

std::map<short, Object> getObjectMapping();
void addObject(short id, std::string objectName,  std::map<short, Object>& mapping, std::map<std::string, Mesh>& meshes);
void renderObject(short id, std::map<short, Object>& mapping);

#endif 