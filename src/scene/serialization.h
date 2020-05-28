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
  glm::vec3 linearFactor;
  glm::vec3 angularFactor;
  glm::vec3 gravity;
  float friction;
  float restitution;
};

struct Token {
  std::string target;
  std::string attribute;
  std::string payload;
  std::string layer;
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
  std::string lookat;
  std::string layer;
  std::string script;
  std::map<std::string, std::string> additionalFields;
};

struct LayerInfo {
  std::string name;
  int zIndex;
  bool orthographic;
};
struct ParsedContent {
  std::vector<Token> tokens; 
  std::vector<LayerInfo> layers;
};
ParsedContent parseFormat(std::string content);

std::map<std::string, SerializationObject> deserializeSceneTokens(std::vector<Token> tokens, std::vector<Field> additionalFields);
std::string getType(std::string name, std::vector<Field> additionalFields);

#endif