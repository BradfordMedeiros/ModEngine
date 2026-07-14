#include "./object_util.h"

extern CustomApiBindings* mainApi;
extern World world;

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
