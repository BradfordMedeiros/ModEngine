#ifndef MOD_SERIALIZATION
#define MOD_SERIALIZATION

#include <vector>
#include <map>
#include <iostream>
#include <sstream>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../common/util.h"

enum ColliderShape { BOX, SPHERE, AUTOSHAPE };

struct physicsOpts {
  bool enabled;
  bool isStatic;
  bool hasCollisions;
  ColliderShape shape;
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

struct SerializationObject {
  std::string name;
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
  bool hasParent;
  std::string parentName;
  physicsOpts physics;
  std::string type;
  std::map<std::string, std::string> additionalFields;
};

glm::quat parseQuat(std::string payload);
glm::vec3 parseVec(std::string positionRaw);
std::vector<Token> getTokens(std::string content);
std::map<std::string, SerializationObject> deserializeScene(std::vector<Token> tokens, std::vector<Field> additionalFields);
std::string serializationObjectToString(std::vector<SerializationObject> object);

#endif