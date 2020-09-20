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
  float mass;
  float maxspeed;
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
};

struct SerializationObject {
  bool hasId;
  int id;
  std::string name;
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
  std::vector<std::string> children;
  physicsOpts physics;
  std::string type;
  std::string lookat;
  std::string layer;
  std::string script;
  std::string fragshader;
  bool netsynchronize;
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
SerializationObject getDefaultObject(std::string name, std::vector<Field> additionalFields, std::string layer);

std::map<std::string, SerializationObject> deserializeSceneTokens(std::vector<Token> tokens, std::vector<Field> additionalFields);
std::string getType(std::string name, std::vector<Field> additionalFields);

physicsOpts defaultPhysicsOpts(std::map<std::string, std::string> stringAttributes, std::map<std::string, glm::vec3> vecAttributes);


#endif