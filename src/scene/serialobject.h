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
  glm::vec3 velocity;
  glm::vec3 angularVelocity;
  float friction;
  float restitution;
  float mass;
  float maxspeed;
  float layer;
};

struct GameObject {
  objid id;
  std::string name;
  Transformation transformation;
  physicsOpts physicsOptions;  // Should remove this and just get it from the physics system (probably)
  std::string lookat;
  std::string layer;
  std::string script;
  std::string fragshader;
  bool netsynchronize;
  GameobjAttributes attr;
};

GameObject gameObjectFromFields(std::string name, objid id, GameobjAttributes attributes);
void setAttribute(GameObject& gameobj, std::string field, AttributeValue attr);
void setAllAttributes(GameObject& gameobj, GameobjAttributes& attr);
void getAllAttributes(GameObject& gameobj, GameobjAttributes& _attr);
AttributeValue parsePropertySuffix(std::string key, std::string value);

#endif