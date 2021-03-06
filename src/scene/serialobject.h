#ifndef MOD_SERIALOBJ
#define MOD_SERIALOBJ

#include <string>
#include <vector>
#include <map>
#include <glm/gtx/quaternion.hpp>
#include "../common/util.h"
#include "./common/util/types.h"

enum ColliderShape { BOX, SPHERE, CAPSULE, CYLINDER, CONVEXHULL, SHAPE_EXACT, AUTOSHAPE };

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
  float layer;
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
  GameobjAttributes additionalFields;
};

struct GameObject {
  objid id;
  std::string name;
  Transformation transformation;
  physicsOpts physicsOptions;
  std::string lookat;
  std::string layer;
  std::string script;
  std::string fragshader;
  bool netsynchronize;
};

GameObject gameObjectFromFields(std::string name, objid id, GameobjAttributes attributes);
void setAttribute(GameObject& gameobj, std::string field, AttributeValue attr);
void setAllAttributes(GameObject& gameobj, GameobjAttributes& attr);
void getAllAttributes(GameObject& gameobj, GameobjAttributes& _attr);
void applyAttributeDelta(GameObject& gameobj, std::string field, AttributeValue delta);
AttributeValue parsePropertySuffix(std::string key, std::string value);

#endif