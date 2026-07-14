#ifndef MOD_OBJECTUTIL
#define MOD_OBJECTUTIL

#include "./scene/scene.h"
#include "./cscript/cscript_binding.h"

void setGameObjectTexture(objid id, std::string texture);
void setGameObjectTextureOffset(objid id, glm::vec2 offset);
void setGameObjectTextureSize(objid id, glm::vec2 offset);
void setGameObjectFriction(objid id, float friction);
void setGameObjectGravity(objid id, glm::vec3 gravity);
glm::vec3 getGameObjectVelocity(objid id);
void setGameObjectVelocity(objid id, glm::vec3 velocity);
void setGameObjectTint(objid id, glm::vec4 tint);
void setGameObjectStateEnabled(objid id, bool enable);
void setGameObjectMeshEnabled(objid id, bool enable);

bool isGameObjectPhysicsDynamic(objid id);
void setGameObjectPhysicsDynamic(objid id, bool isDynamic);

bool isGameObjectPhysicsEnabled(objid id);
void setGameObjectPhysicsEnable(objid id, bool enable);

void setGameObjectPhysicsMass(objid id, float mass);
void setGameObjectPhysics(objid id, float mass, float restitution, float friction, glm::vec3 gravity);
void setGameObjectPhysicsOptions(objid id, glm::vec3 angle, glm::vec3 linear, glm::vec3 gravity);
void setGameObjectEmission(objid id, std::optional<glm::vec3> emission);
void setGameObjectEmitterEffectTint(objid id, glm::vec4 tint);

std::string getGameObjectLayer(objid id);
void setGameObjectLayer(objid id, std::string layer);

#endif