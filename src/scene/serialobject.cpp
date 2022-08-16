#include "./serialobject.h"

void safeVec3Set(glm::vec3* value, const char* key, GameobjAttributes& attributes, glm::vec3* defaultValue){
  if (attributes.vecAttr.vec3.find(key) != attributes.vecAttr.vec3.end()){
    *value = attributes.vecAttr.vec3.at(key);
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

ColliderShape parseShape(std::string& value){
  if (value == "shape_sphere"){
    return SPHERE;
  }else if (value == "shape_box"){
    return BOX;
  }else if (value == "shape_capsule"){
    return CAPSULE;
  }else if (value == "shape_cylinder"){
    return CYLINDER;
  }else if (value == "shape_hull"){
    return CONVEXHULL;
  }else if (value == "shape_auto"){
    return AUTOSHAPE;
  }else if (value == "shape_exact"){
    return SHAPE_EXACT;
  }
  return AUTOSHAPE;
}


void setSerialObjFromAttr(GameObject& object, GameobjAttributes& attributes){
  if (attributes.stringAttributes.find("id") != attributes.stringAttributes.end()){
    object.id = std::atoi(attributes.stringAttributes.at("id").c_str());
  }

  auto identityVec = glm::vec3(1.f, 1.f, 1.f);
  auto zeroVec = glm::vec3(0.f, 0.f, 0.f);
  auto defaultGravity = glm::vec3(0.f, -9.81f, 0.f);

  safeVec3Set(&object.transformation.position, "position", attributes, &zeroVec);
  safeVec3Set(&object.transformation.scale, "scale", attributes, &identityVec);
  safeVec3Set(&object.physicsOptions.angularFactor, "physics_angle", attributes, &identityVec);
  safeVec3Set(&object.physicsOptions.linearFactor, "physics_linear", attributes, &identityVec);
  safeVec3Set(&object.physicsOptions.gravity, "physics_gravity", attributes, &defaultGravity);
  safeVec3Set(&object.physicsOptions.velocity, "physics_velocity", attributes, &zeroVec);
  safeVec3Set(&object.physicsOptions.angularVelocity, "physics_avelocity", attributes, &zeroVec);

  auto oneValue = 1.f;
  auto negOneValue = -1.f;
  auto zeroValue = 0.f;
  safeFloatSet(&object.physicsOptions.friction, "physics_friction", attributes, &oneValue);
  safeFloatSet(&object.physicsOptions.restitution, "physics_restitution", attributes, &zeroValue);
  safeFloatSet(&object.physicsOptions.mass, "physics_mass", attributes, &oneValue);
  safeFloatSet(&object.physicsOptions.maxspeed, "physics_maxspeed", attributes, &negOneValue); // -1 ?  does sign matter? 
  safeFloatSet(&object.physicsOptions.layer, "physics_layer", attributes, &zeroValue);

  safeStringSet(&object.lookat, "lookat", attributes);
  safeStringSet(&object.script, "script", attributes);
  safeStringSet(&object.fragshader, "fragshader", attributes);
  safeStringSet(&object.layer, "layer", attributes);

  // old default in getDefaultPhysics was false
  if (attributes.stringAttributes.find("physics") != attributes.stringAttributes.end()){
    object.physicsOptions.enabled = attributes.stringAttributes.at("physics") == "enabled";
  }else{
    object.physicsOptions.enabled = false;
  }

  // old default in getDefaultPhysics was false
  if (attributes.stringAttributes.find("physics_collision") != attributes.stringAttributes.end()){
    object.physicsOptions.hasCollisions = !(attributes.stringAttributes.at("physics_collision") == "nocollide");
  }else{
    object.physicsOptions.hasCollisions = true;
  }

  if (attributes.stringAttributes.find("physics_type") != attributes.stringAttributes.end()){
    object.physicsOptions.isStatic = attributes.stringAttributes.at("physics_type") != "dynamic";
  }else{
    object.physicsOptions.isStatic = true;
  }

  if (attributes.stringAttributes.find("physics_shape") != attributes.stringAttributes.end()){
    auto value = attributes.stringAttributes.at("physics_shape");
    object.physicsOptions.shape = parseShape(value);
  }else{
    object.physicsOptions.shape = AUTOSHAPE;
  }

  if (attributes.stringAttributes.find("net") != attributes.stringAttributes.end()){
    object.netsynchronize = attributes.stringAttributes.at("net") == "sync";
  }
  safeVec3Set(&object.transformation.scale, "scale", attributes, &identityVec);

  if (attributes.vecAttr.vec4.find("rotation") != attributes.vecAttr.vec4.end()){
    object.transformation.rotation = parseQuat(attributes.vecAttr.vec4.at("rotation"));
  }else{
    object.transformation.rotation = glm::identity<glm::quat>();
  }
  object.attr = attributes; // lots of redundant information here, should only set attrs that aren't consumed elsewhere
}

GameObject gameObjectFromFields(std::string name, objid id, GameobjAttributes attributes){
  GameObject gameObject = { .id = id, .name = name };
  setSerialObjFromAttr(gameObject, attributes);
  return gameObject;
}


// property suffix looks like the parts of the tokens on the right hand side
// eg position 10
// eg tint 0.9 0.2 0.4
AttributeValue parsePropertySuffix(std::string key, std::string value){
  MODTODO("combine parse property suffix with getAttribute in serialization");
  if (key == "position" || key == "scale"){
    return parseVec(value);
  }
  return value;
}

