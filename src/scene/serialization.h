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
  physicsOpts physics;
};

glm::vec3 parseVec(std::string positionRaw);
std::vector<Token> getTokens(std::string content);
std::vector<SerializationObject> deserializeScene(std::vector<Token> tokens);

#endif