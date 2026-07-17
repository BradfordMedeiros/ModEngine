#ifndef MOD_OBJECTUTIL
#define MOD_OBJECTUTIL

#include "./scene/scene.h"
#include "./cscript/cscript_binding.h"
#include "./state.h"

void setGameObjectTexture(objid id, std::string texture);
void setGameObjectTextureOffset(objid id, glm::vec2 offset);
void setGameObjectTextureSize(objid id, glm::vec2 offset);
void setGameObjectFriction(objid id, float friction);
void setGameObjectGravity(objid id, glm::vec3 gravity);
glm::vec3 getGameObjectVelocity(objid id);
void setGameObjectVelocity(objid id, glm::vec3 velocity);

void setGameObjectTint(objid id, glm::vec4 tint);
glm::vec4 getGameObjectTint(objid id);

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

std::string getGameObjectShader(objid id);
void setGameObjectShader(objid id, std::string shader);

bool getGameObjectHasCollision(objid id);
void setGameObjectHasCollision(objid id, bool hasCollision);

std::string getGameObjectPhysicsShape(objid id);
void setGameObjectPhysicsShape(objid id, std::string shape);

ObjectType getObjectType(objid id);


bool isEditorDebug();
void setEditorDebug(bool show);
bool isShowCamera();
void setShowCamera(bool show);
bool isShowLights();
void setShowLights(bool show);
bool isShowEmitters();
void setShowEmitters(bool show);
bool isShowSound();
void setShowSound(bool show);

std::optional<objid> activeSceneId();
bool isMuted();
void setIsMuted(bool isMuted);
bool isDiffuseEnabled();
void setDiffuseEnabled(bool enabled);
bool isSpecularEnabled();
void setSpecularEnabled(bool enabled);
bool isBloomEnabled();
void setBloomEnabled(bool enabled);
bool isAttenuationEnabled();
void setAttenuationEnabled(bool enabled);
bool isShadowsEnabled();
void setShadowsEnabled(bool enabled);
bool isExposureEnabled();
void setExposureEnabled(bool enabled);
bool isGammaEnabled();
void setGammaEnabled(bool enabled);
bool isSkyboxEnabled();
void setSkyboxEnabled(bool enabled);
glm::vec3 skyboxColor();
void setSkyboxColor(glm::vec3 color);
bool isCullEnabled();
void setCullEnabled(bool enabled);
bool isFogEnabled();
void setFogEnabled(bool enabled);
float fogMinCutoff();
void setFogMinCutoff(float value);
float fogMaxCutoff();
void setFogMaxCutoff(float value);
glm::vec4 fogColor();
void setFogColor(glm::vec4 value);
glm::vec3 ambientLight();
void setAmbientLightColor(glm::vec3 amount);
void setManipulatorMode(ManipulatorMode mode);
ManipulatorMode getManipulatorMode();
bool isTranslateMirror();
void setTranslateMirror(bool isEnabled);
bool isUniformScale();
void setUniformScale(bool isEnabled);
Axis getManipulatorAxis();
void setManipulatorAxis(Axis axis);

#endif