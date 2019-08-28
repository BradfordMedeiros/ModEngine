#ifndef MOD_OBJECTTYPES
#define MOD_OBJECTTYPES

#include <iostream>
#include <map>
#include <stdexcept>
#include "./scene/common/mesh.h"

// idea is to have a union of the types we want + active type in struct, and then a data structure which will map ids (objects that we add to the scene)
// to the proper types 
// this way we can decouple, for example, the camera type, or the concept of a mesh, from our scene graph 

enum ObjectType { MESH = 1 } ;

union Objects {
  Mesh object;
};
struct Object {
  Objects obj;
  ObjectType activeType;
};

std::map<short, Objects> getObjectMapping();
void addObject(short id, std::string objectName, std::map<short, Objects>& mapping);


#endif 