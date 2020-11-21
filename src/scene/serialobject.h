#ifndef MOD_SERIALOBJ
#define MOD_SERIALOBJ

#include <string>
#include <vector>
#include <map>
#include <glm/gtx/quaternion.hpp>
#include "../common/util.h"
#include "./common/util/types.h"

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

struct SerializationObject {
  bool hasId;
  int id;
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
  std::vector<std::string> children;
  physicsOpts physics;
  std::string lookat;
  std::string layer;
  std::string script;
  std::string fragshader;
  bool netsynchronize;
  glm::vec3 tint;
  std::map<std::string, std::string> additionalFields;
};

void setSerialObjFromAttr(SerializationObject& object, GameobjAttributes& attributes);

#endif