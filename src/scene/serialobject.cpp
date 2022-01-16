#include "./serialobject.h"

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
  safeVecSet(&object.physics.velocity, "physics_velocity", attributes, &zeroVec);
  safeVecSet(&object.physics.angularVelocity, "physics_avelocity", attributes, &zeroVec);

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
    object.physics.enabled = false;
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
    }else if (value == "shape_capsule"){
      object.physics.shape = CAPSULE;
    }else if (value == "shape_cylinder"){
      object.physics.shape = CYLINDER;
    }else if (value == "shape_hull"){
      object.physics.shape = CONVEXHULL;
    }else if (value == "shape_auto"){
      object.physics.shape = AUTOSHAPE;
    }else if (value == "shape_exact"){
      object.physics.shape = SHAPE_EXACT;
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
  object.additionalFields = attributes;
  object.children = attributes.children;
}

GameObject gameObjectFromFields(std::string name, objid id, GameobjAttributes attributes){
  SerializationObject serialObj{};
  setSerialObjFromAttr(serialObj, attributes);

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

void setAttribute(GameObject& gameobj, std::string field, AttributeValue attr){
  auto value = std::get_if<glm::vec3>(&attr);
  if (field == "position" && value != NULL){
     gameobj.transformation.position = *value;
     return;
  }
  if (field == "scale" && value != NULL){
     gameobj.transformation.scale = *value;
     return;
  } 
  if (field == "physics_angle" && value != NULL){
     gameobj.physicsOptions.angularFactor = *value;
     return;
  } 
  if (field == "physics_linear" && value != NULL){
     gameobj.physicsOptions.linearFactor = *value;
     return;
  } 
  if (field == "physics_gravity" && value != NULL){
     gameobj.physicsOptions.gravity = *value;
     return;
  }   
  if (field == "physics_velocity" && value != NULL){
    gameobj.physicsOptions.velocity = *value;
    return;
  }
  if (field == "physics_avelocity" && value != NULL){
    gameobj.physicsOptions.angularVelocity = *value;
    return;
  }

  auto fValue = std::get_if<float>(&attr);
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
} 
void setAllAttributes(GameObject& gameobj, GameobjAttributes& attr){
  for (auto [field, fieldValue] : attr.stringAttributes){
    setAttribute(gameobj, field, fieldValue);
  }
  for (auto [field, fieldValue] : attr.numAttributes){
    setAttribute(gameobj, field, fieldValue);
  }
  for (auto [field, fieldValue] : attr.vecAttributes){
    setAttribute(gameobj, field, fieldValue);
  }
}
void getAllAttributes(GameObject& gameobj, GameobjAttributes& _attr){
  _attr.vecAttributes["position"] = gameobj.transformation.position;
  _attr.vecAttributes["scale"] = gameobj.transformation.scale;

  _attr.stringAttributes["lookat"] = gameobj.lookat;
  _attr.stringAttributes["layer"] = gameobj.layer;
  _attr.stringAttributes["script"] = gameobj.script;
  _attr.stringAttributes["fragshader"] = gameobj.fragshader;
  _attr.stringAttributes["physics_type"] = gameobj.physicsOptions.isStatic ? "static" : "dynamic";
}

AttributeValue attributeValue(GameObject& gameobj, std::string field){
  if (field == "position"){
    return gameobj.transformation.position;
  }
  if (field == "scale"){
    return gameobj.transformation.scale;
  } 
  if (field == "physics_angle"){
    return gameobj.physicsOptions.angularFactor;
  } 
  if (field == "physics_linear"){
    return gameobj.physicsOptions.linearFactor;
  } 
  if (field == "physics_gravity"){
    return gameobj.physicsOptions.gravity;
  }   
  if (field == "physics_velocity"){
    return gameobj.physicsOptions.velocity;
  }
  if (field == "physics_avelocity"){
    return gameobj.physicsOptions.angularVelocity;
  }
  if (field == "physics_friction"){
    return gameobj.physicsOptions.friction;
  }
  if (field == "physics_restitution"){
    return gameobj.physicsOptions.restitution;
  }
  if (field == "physics_mass"){
    return gameobj.physicsOptions.mass;
  }
  if (field == "physics_maxspeed"){
    return gameobj.physicsOptions.maxspeed;
  }
  if (field == "physics_layer"){
    return gameobj.physicsOptions.layer;
  }
  std::cout << "attribute not yet supported: " << field << std::endl;
  assert(false);
  return 0;
}

AttributeValue addAttributes(AttributeValue one, AttributeValue two){
  auto valueOne = std::get_if<glm::vec3>(&one);
  auto valueTwo = std::get_if<glm::vec3>(&two);
  if (valueOne != NULL){
    assert(valueTwo != NULL);
    return *valueOne + *valueTwo;
  }
  auto fValueOne = std::get_if<float>(&one);
  auto fValueTwo = std::get_if<float>(&two);
  if (fValueOne != NULL){
    assert(fValueTwo != NULL);
    return *fValueOne + *valueTwo;
  }
  // TODO -> wtf to do for strings?
  std::cout << "string values not supported" << std::endl;
  assert(false);
}

// TODO -> eliminate all the strings in the fields and use some sort of symbol system
void applyAttributeDelta(GameObject& gameobj, std::string field, AttributeValue delta){
  setAttribute(gameobj, field, addAttributes(attributeValue(gameobj, field), delta));
}

// property suffix looks like the parts of the tokens on the right hand side
// eg position 10
// eg tint 0.9 0.2 0.4
AttributeValue parsePropertySuffix(std::string key, std::string value){
  std::cout << "TODO -> combine parse property suffix with getAttribute in serialization" << std::endl;
  if (key == "position" || key == "scale"){
    return parseVec(value);
  }
  return value;
}
