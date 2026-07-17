#include "./object_util.h"

extern CustomApiBindings* mainApi;
extern World world;
extern engineState state;

void setGameObjectTexture(objid id, std::string texture){
  mainApi -> setSingleGameObjectAttr(id, "texture", texture);
}
void setGameObjectTextureOffset(objid id, glm::vec2 offset){
  mainApi -> setSingleGameObjectAttr(id, "textureoffset", offset);
}
void setGameObjectTextureSize(objid id, glm::vec2 offset){
  mainApi -> setSingleGameObjectAttr(id, "texturesize", offset);
}
void setGameObjectFriction(objid id, float friction){
  mainApi -> setSingleGameObjectAttr(id, "physics_friction", friction);
}
void setGameObjectGravity(objid id, glm::vec3 gravity){
  mainApi -> setSingleGameObjectAttr(id, "physics_gravity", gravity);
}

glm::vec3 getGameObjectVelocity(objid id){
  return mainApi -> getPhysicsVelocity(id);
}
void setGameObjectVelocity(objid id, glm::vec3 velocity){
  mainApi -> setPhysicsVelocity(id, velocity);
}

void setGameObjectTint(objid id, glm::vec4 tint){
  mainApi -> setSingleGameObjectAttr(id, "tint", tint);
}
glm::vec4 getGameObjectTint(objid id){
  std::optional<glm::vec4*> value = getTypeFromAttr<glm::vec4>(getObjectAttributePtr(world, id, "tint"));
  if (value.has_value()){
    return *(value.value());
  }
  modassert(false, "getGameObjectTint no tint");
  return glm::vec4(1.f, 1.f, 1.f, 1.f);
}

void setGameObjectStateEnabled(objid id, bool enable){
  mainApi -> setSingleGameObjectAttr(id, "state", enable ? std::string("enabled") : std::string("disabled"));
}
void setGameObjectMeshEnabled(objid id, bool enable){
  mainApi -> setSingleGameObjectAttr(id, "disabled", enable ? "false" : "true");
}


bool isGameObjectPhysicsDynamic(objid id){
  std::optional<bool*> value = getTypeFromAttr<bool>(getObjectAttributePtr(world, id, "physics_type"));
  if (value.has_value()){
    auto isStatic = *(value.value());
    return !isStatic;
  }
  modassert(false, "no physics isGameObjectPhysicsDynamic - unexpected");
  return false; 
}
void setGameObjectPhysicsDynamic(objid id, bool isDynamic){
   mainApi -> setSingleGameObjectAttr(id, "physics_type", isDynamic ? "dynamic" : "static");
}

bool isGameObjectPhysicsEnabled(objid id){
  std::optional<bool*> value = getTypeFromAttr<bool>(getObjectAttributePtr(world, id, "physics"));
  if (value.has_value()){
   return *(value.value());
  }
  modassert(false, "no physics enabled - unexpected");
  return false; 
}
void setGameObjectPhysicsEnable(objid id, bool enable){
  mainApi -> setSingleGameObjectAttr(id, "physics", enable ? "enabled" : "disabled");
}


void setGameObjectPhysicsMass(objid id, float mass){
  mainApi -> setGameObjectAttr(
    id, 
    {
      GameobjAttribute { .field = "physics_mass", .attributeValue = mass },
    }
  ); 
}

void setGameObjectPhysics(objid id, float mass, float restitution, float friction, glm::vec3 gravity){
  mainApi -> setGameObjectAttr(
    id, 
    {
      GameobjAttribute { .field = "physics_mass", .attributeValue = mass },
      GameobjAttribute { .field = "physics_restitution", .attributeValue = restitution },
      GameobjAttribute { .field = "physics_friction", .attributeValue = friction },
      GameobjAttribute { .field = "physics_gravity", .attributeValue = gravity },
    }
  );
}
void setGameObjectPhysicsOptions(objid id, glm::vec3 angle, glm::vec3 linear, glm::vec3 gravity){
  mainApi -> setGameObjectAttr(
    id, 
    {
      GameobjAttribute { .field = "physics_angle", .attributeValue = angle },
      GameobjAttribute { .field = "physics_linear", .attributeValue = linear },
      GameobjAttribute { .field = "physics_gravity", .attributeValue = gravity },
    }
  );
}

void setGameObjectEmission(objid id, std::optional<glm::vec3> emission){
  mainApi -> setGameObjectAttr(
    id,
    {
       GameobjAttribute { .field = "emission", .attributeValue = emission.has_value() ? emission.value() : glm::vec3(0.f, 0.f, 0.f) },
    }
  );
}

void setGameObjectEmitterEffectTint(objid id, glm::vec4 tint){
  mainApi -> setSingleGameObjectAttr(id, "effect-tint", tint);
}

std::string getGameObjectLayer(objid id){
  std::optional<std::string*> value = getTypeFromAttr<std::string>(getObjectAttributePtr(world, id, "layer"));
  modassert(value.has_value(), "no layer");
  return *(value.value());
}
void setGameObjectLayer(objid id, std::string layer){
  mainApi -> setSingleGameObjectAttr(id, "layer", layer);
}

std::string getGameObjectShader(objid id){
  std::optional<std::string*> value = getTypeFromAttr<std::string>(getObjectAttributePtr(world, id, "shader"));
  modassert(value.has_value(), "no shader");
  return *(value.value());
}

void setGameObjectShader(objid id, std::string shader){
  mainApi -> setSingleGameObjectAttr(id, "shader", shader);
}

bool getGameObjectHasCollision(objid id){
  std::optional<bool*> value = getTypeFromAttr<bool>(getObjectAttributePtr(world, id, "physics_collision"));
  modassert(value.has_value(), "no shader");
  return *(value.value());
}

void setGameObjectHasCollision(objid id, bool hasCollision){
  mainApi -> setSingleGameObjectAttr(id, "physics_collision", hasCollision);
}

std::string getGameObjectPhysicsShape(objid id){
  std::optional<std::string*> value = getTypeFromAttr<std::string>(getObjectAttributePtr(world, id, "physics_shape"));
  modassert(value.has_value(), "no shader");
  return *(value.value());
}

void setGameObjectPhysicsShape(objid id, std::string shape){
  mainApi -> setSingleGameObjectAttr(id, "physics_shape", shape);
}

ObjectType getObjectType(objid id){
  auto name = mainApi -> getGameObjNameForId(id).value();
  auto type = getObjectType(name);  
  return type;
}


bool isEditorDebug(){
  return state.showDebug;
}
void setEditorDebug(bool show){
  state.showDebug = show;
}
bool isShowCamera(){
  return (state.showDebugMask & 0b10) != 0;
}
void setShowCamera(bool show){
  if (show){
    state.showDebugMask = state.showDebugMask | 0b10;
  }else{
    state.showDebugMask = state.showDebugMask & ~(0b10);
  }
}
bool isShowLights(){
  return (state.showDebugMask & 0b1000) != 0;
}
void setShowLights(bool show){
  if (show){
    state.showDebugMask = state.showDebugMask | 0b1000;
  }else{
    state.showDebugMask = state.showDebugMask & ~(0b1000);
  }
}
bool isShowEmitters(){
  return (state.showDebugMask & 0b100000) != 0;
}
void setShowEmitters(bool show){
  if (show){
    state.showDebugMask = state.showDebugMask | 0b100000;
  }else{
    state.showDebugMask = state.showDebugMask & ~(0b100000);
  }
}
bool isShowSound(){
  return (state.showDebugMask & 0b100) != 0;
}
void setShowSound(bool show){
  if (show){
    state.showDebugMask = state.showDebugMask | 0b100;
  }else{
    state.showDebugMask = state.showDebugMask & ~(0b100);
  }
}


std::optional<objid> activeSceneId(){
  auto selected = mainApi -> selected();
  if (selected.size() == 0){
    return std::nullopt;
  }
  auto selectedId = selected.at(0);
  auto sceneId = mainApi -> listSceneId(selectedId);
  return sceneId;
}

bool isMuted(){
  return state.muteSound;
}
void setIsMuted(bool isMuted){
  state.muteSound = isMuted;
}

bool isDiffuseEnabled(){
  return state.enableDiffuse;
}
void setDiffuseEnabled(bool enabled){
  state.enableDiffuse = enabled;
}
bool isSpecularEnabled(){
  return state.enableSpecular;
}
void setSpecularEnabled(bool enabled){
  state.enableSpecular = enabled;
}
bool isBloomEnabled(){
  return state.enableBloom;
}
void setBloomEnabled(bool enabled){
  state.enableBloom = enabled;
}
bool isAttenuationEnabled(){
  return state.enableAttenuation;
}
void setAttenuationEnabled(bool enabled){
  state.enableAttenuation = enabled;
}
bool isShadowsEnabled(){
  return state.enableShadows;
}
void setShadowsEnabled(bool enabled){
  state.enableShadows = enabled;
}
bool isExposureEnabled(){
  return state.enableExposure;
}
void setExposureEnabled(bool enabled){
  state.enableExposure = enabled;
}
bool isGammaEnabled(){
  return state.enableGammaCorrection;
}
void setGammaEnabled(bool enabled){
  state.enableGammaCorrection = enabled;
}
bool isSkyboxEnabled(){
  return state.showSkybox;
}
void setSkyboxEnabled(bool enabled){
  state.showSkybox = enabled;
}
glm::vec3 skyboxColor(){
  return state.skyboxcolor;
}
void setSkyboxColor(glm::vec3 color){
  state.skyboxcolor = color;
}
bool isCullEnabled(){
  return state.cullEnabled;
}
void setCulling(bool cullEnabled);
void setCullEnabled(bool enabled){
  state.cullEnabled = enabled;
  setCulling(state.cullEnabled);
}
bool isFogEnabled(){
  return state.enableFog;
}
void setFogEnabled(bool enabled){
  state.enableFog = enabled;
}
float fogMinCutoff(){
  return state.fogMinCutoff;
}
void setFogMinCutoff(float value){
  state.fogMinCutoff = value;
}
float fogMaxCutoff(){
  return state.fogMaxCutoff;
}
void setFogMaxCutoff(float value){
  state.fogMaxCutoff = value;
}
glm::vec4 fogColor(){
  return state.fogColor;
}
void setFogColor(glm::vec4 value){
  state.fogColor = value;
}

glm::vec3 ambientLight(){
  return state.ambient;
}
void setAmbientLightColor(glm::vec3 amount){
  state.ambient = amount;
}

void setManipulatorMode(ManipulatorMode mode){
  state.manipulatorMode = mode;
}
ManipulatorMode getManipulatorMode(){
  return state.manipulatorMode;
}
bool isTranslateMirror(){
  return state.translateMirror;
}
void setTranslateMirror(bool isEnabled){
  state.translateMirror = isEnabled;
}
bool isUniformScale(){
  return state.preserveRelativeScale;
}
void setUniformScale(bool isEnabled){
  state.preserveRelativeScale = isEnabled;
}

Axis getManipulatorAxis(){
  return state.manipulatorAxis;
}
void setManipulatorAxis(Axis axis){
  state.manipulatorAxis = axis;
}