#ifndef MOD_OBJECTTYPES
#define MOD_OBJECTTYPES

#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>
#include <variant>
#include "./scene/common/mesh.h"
#include "./scene/scene.h"

// idea is to have a union of the types we want + active type in struct, and then a data structure which will map ids (objects that we add to the scene)
// to the proper types 
// this way we can decouple, for example, the camera type, or the concept of a mesh, from our scene graph 

typedef std::variant<Mesh, int> GameObjectObj;

static Field obj = {
  .prefix = '@', 
  .type = "default",
  .additionalFields = { "mesh" }
};

static Field camera = {
  .prefix = '>',
  .type = "camera",
  .additionalFields = { },
};
static std::vector fields = { obj, camera };

std::map<short, GameObjectObj> getObjectMapping();

void addObject(short id, std::string objectType, std::string field, std::string payload, std::map<short, GameObjectObj>& mapping, std::map<std::string, Mesh>& meshes, std::string defaultMesh);
void renderObject(short id, std::map<short, GameObjectObj>& mapping, Mesh& cameraMesh, bool showCameras);

std::vector<std::pair<std::string, std::string>> getAdditionalFields(short id, std::map<short, GameObjectObj>& mapping);

std::map<short, int> getCameras();
std::map<short, Mesh> getMeshObjects();


#endif 