#ifndef MOD_SCENEGRAPH
#define MOD_SCENEGRAPH

#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <functional>
#include <glm/gtc/quaternion.hpp>
#include "./common/mesh.h"
#include "../common/util.h"

struct GameObject {
  short id;
  std::string name;
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
};
struct GameObjectH {
  short id;
  short parentId;
  std::set<short> children;
};

struct Scene {
  short id;
  std::vector<short> rootGameObjectsH;
  std::map<short, GameObject> idToGameObjects;
  std::map<short, GameObjectH> idToGameObjectsH;
  std::map<std::string, short> nameToId;
};

struct Token {
  std::string target;
  std::string attribute;
  std::string payload;
};

struct Field {
  char prefix;
  std::string type;
  std::vector<std::string> additionalFields;
};


std::string serializeScene(Scene& scene, std::function<std::vector<std::pair<std::string, std::string>>(short)> getAdditionalFields);
Scene deserializeScene(std::string content, std::function<void(short, std::string, std::string, std::string)> addObject, std::vector<Field> fields);
  
void addObjectToScene(Scene& scene, std::string name, std::string mesh,  glm::vec3 position, std::function<void(short, std::string, std::string, std::string)> addObject);
void removeObjectFromScene(Scene& scene, short id);

#endif
