#include "./serialobject.h"

void safeVecSet(glm::vec3* value, const char* key, GameobjAttributes& attributes, glm::vec3* defaultValue){
  if (attributes.vecAttributes.find(key) != attributes.vecAttributes.end()){
    *value = attributes.vecAttributes.at(key);
  }else if (defaultValue != NULL){
    *value = *defaultValue;
  }
}
void safeFloatSet(float* value, const char* key, GameobjAttributes& attributes, float* defaultValue){
  if (attributes.numAttributes.find(key) != attributes.numAttributes.end()){
    *value = attributes.numAttributes.at(key);
  }else if (defaultValue != NULL){
    *value = *defaultValue;
  }
}
void safeStringSet(std::string* value, const char* key, GameobjAttributes& attributes){
  if (attributes.stringAttributes.find(key) != attributes.stringAttributes.end()){
    *value = attributes.stringAttributes.at(key);
  }
}

void setSerialObjFromAttr(SerializationObject& object, GameobjAttributes& attributes){
  auto identityVec = glm::vec3(1.f, 1.f, 1.f);
  auto zeroVec = glm::vec3(0.f, 0.f, 0.f);
  auto defaultGravity = glm::vec3(0.f, -9.81f, 0.f);

  safeVecSet(&object.position, "position", attributes, &zeroVec);
  safeVecSet(&object.scale, "scale", attributes, &identityVec);
  safeVecSet(&object.physics.angularFactor, "physics_angle", attributes, &identityVec);
  safeVecSet(&object.physics.linearFactor, "physics_linear", attributes, &identityVec);
  safeVecSet(&object.physics.gravity, "physics_gravity", attributes, &defaultGravity);

  auto oneValue = 1.f;
  auto negOneValue = -1.f;
  auto zeroValue = 0.f;
  safeFloatSet(&object.physics.friction, "physics_friction", attributes, &oneValue);
  safeFloatSet(&object.physics.restitution, "physics_restitution", attributes, &zeroValue);
  safeFloatSet(&object.physics.mass, "physics_mass", attributes, &oneValue);
  safeFloatSet(&object.physics.maxspeed, "physics_maxspeed", attributes, &negOneValue); // -1 ?  does sign matter? 
  safeFloatSet(&object.physics.layer, "physics_layer", attributes, &zeroValue);

  safeStringSet(&object.lookat, "lookat", attributes);
  safeStringSet(&object.script, "script", attributes);
  safeStringSet(&object.fragshader, "fragshader", attributes);
  safeStringSet(&object.layer, "layer", attributes);

  // old default in getDefaultPhysics was false
  if (attributes.stringAttributes.find("physics") != attributes.stringAttributes.end()){
    object.physics.enabled = attributes.stringAttributes.at("physics") == "enabled";
  }else{
    object.physics.enabled = true;
  }

  // old default in getDefaultPhysics was false
  if (attributes.stringAttributes.find("physics_collision") != attributes.stringAttributes.end()){
    object.physics.hasCollisions = !(attributes.stringAttributes.at("physics_collision") == "nocollide");
  }else{
    object.physics.hasCollisions = true;
  }

  if (attributes.stringAttributes.find("physics_type") != attributes.stringAttributes.end()){
    object.physics.isStatic = attributes.stringAttributes.at("physics_type") != "dynamic";
  }else{
    object.physics.isStatic = true;
  }

  if (attributes.stringAttributes.find("physics_shape") != attributes.stringAttributes.end()){
    auto value = attributes.stringAttributes.at("physics_shape");
    if (value == "shape_sphere"){
      object.physics.shape = SPHERE;
    }else if (value == "shape_box"){
      object.physics.shape = BOX;
    }else if (value == "shape_auto"){
      object.physics.shape = AUTOSHAPE;
    }else{
      object.physics.shape = AUTOSHAPE;
    }
  }

  if (attributes.stringAttributes.find("net") != attributes.stringAttributes.end()){
    object.netsynchronize = attributes.stringAttributes.at("net") == "sync";
  }

  if (attributes.stringAttributes.find("id") != attributes.stringAttributes.end()){
    object.id = std::atoi(attributes.stringAttributes.at("id").c_str());
    object.hasId = true;
  }else{
    object.id  = -1;
    object.hasId = false;
  }

  if (attributes.stringAttributes.find("rotation") != attributes.stringAttributes.end()){
    object.rotation = parseQuat(attributes.stringAttributes.at("rotation"));
  }else{
    object.rotation = glm::identity<glm::quat>();
  }
  object.additionalFields = attributes.additionalFields;
  object.children = attributes.children;
}

SerializationObject getDefaultObject(){
  GameobjAttributes attr{ };
  SerializationObject obj{};
  setSerialObjFromAttr(obj, attr);
  return obj;
}

GameObject gameObjectFromParam(std::string name, objid id, SerializationObject& serialObj){
  std::map<std::string, std::string> stringAttributes;
  std::map<std::string, double> numAttributes;
  std::map<std::string, glm::vec3> vecAttributes;

  GameobjAttributes attributes {
    .stringAttributes = stringAttributes,
    .numAttributes = numAttributes,
    .vecAttributes = vecAttributes,
  };
  auto defaultObject = getDefaultObject();
  setSerialObjFromAttr(defaultObject, attributes);

  GameObject gameObject = {
    .id = id,
    .name = name,
    .transformation = Transformation {
      .position = serialObj.position,
      .scale = serialObj.scale,
      .rotation = serialObj.rotation,
    },
    .physicsOptions = serialObj.physics,
    .lookat =  serialObj.lookat, 
    .layer = serialObj.layer,
    .script = serialObj.script,
    .fragshader = serialObj.fragshader,
    .netsynchronize = serialObj.netsynchronize,
  };

  return gameObject;
}

SerializationObject serialObjectFromFields(GameobjAttributes attributes){
  auto defaultObject = getDefaultObject();
  setSerialObjFromAttr(defaultObject, attributes); 
  return defaultObject;
}

GameObject gameObjectFromFields(std::string name, objid id, GameobjAttributes attributes){
  auto serialObj = serialObjectFromFields(attributes);
  return gameObjectFromParam(name, id, serialObj);
}

// TODO -> eliminate all the strings in the fields and use some sort of symbol system
void applyAttribute(GameObject& gameobj, std::string field, AttributeValue delta){
  auto value = std::get_if<glm::vec3>(&delta);
  if (field == "position" && value != NULL){
     gameobj.transformation.position = gameobj.transformation.position +  *value;
     return;
  }
  if (field == "scale" && value != NULL){
     gameobj.transformation.scale = gameobj.transformation.scale +  *value;
     return;
  } 
  if (field == "physics_angle" && value != NULL){
     gameobj.physicsOptions.angularFactor = gameobj.physicsOptions.angularFactor +  *value;
     return;
  } 
  if (field == "physics_linear" && value != NULL){
     gameobj.physicsOptions.linearFactor = gameobj.physicsOptions.linearFactor +  *value;
     return;
  } 
  if (field == "physics_gravity" && value != NULL){
     gameobj.physicsOptions.gravity = gameobj.physicsOptions.gravity +  *value;
     return;
  }   

  auto fValue = std::get_if<float>(&delta);
  if (field == "physics_friction" && fValue != NULL){
    gameobj.physicsOptions.friction = *fValue;
    return;
  }
  if (field == "physics_restitution" && fValue != NULL){
    gameobj.physicsOptions.restitution = *fValue;
    return;
  }
  if (field == "physics_mass" && fValue != NULL){
    gameobj.physicsOptions.mass = *fValue;
    return;
  }
  if (field == "physics_maxspeed" && fValue != NULL){
    gameobj.physicsOptions.maxspeed = *fValue;
    return;
  }
  if (field == "physics_layer" && fValue != NULL){
    gameobj.physicsOptions.layer = *fValue;
    return;
  }

  std::cout << "attribute not yet supported: " << field << std::endl;
  assert(false);
}