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

  auto vec4Value = std::get_if<glm::vec4>(&attr);
  if (field == "rotation" && vec4Value != NULL){
    MODTODO("probably use basic quaternion representation internally and just make the type outer layer for ease of use");
    gameobj.transformation.rotation = parseQuat(*vec4Value);
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

  auto strValue = std::get_if<std::string>(&attr);
  if (field == "lookat" && strValue != NULL){
    gameobj.lookat = *strValue;
    return;
  }
  if (field == "script" && strValue != NULL){
    assert(false); // cannot set script this way yet (would need to support load/unload)
    gameobj.lookat = *strValue;
    return;
  }
  if (field == "fragshader" && strValue != NULL){ 
    assert(false);  // would need to recompile and stuff, need to implement
    gameobj.lookat = *strValue;
    return;
  }
  if (field == "layer" && strValue != NULL){
    assert(false);  // should work already, but should verify 
    gameobj.lookat = *strValue;
    return;
  }
  if (field == "physics" && strValue != NULL){
    gameobj.physicsOptions.enabled = *strValue == "enabled";
    return;
  }
  if (field == "physics_shape" && strValue != NULL){
    gameobj.physicsOptions.shape = parseShape(*strValue);
    return;
  }
} 
void setAllAttributes(GameObject& gameobj, GameobjAttributes& attr){
  for (auto [field, fieldValue] : attr.stringAttributes){
    setAttribute(gameobj, field, fieldValue);
    gameobj.attr.stringAttributes[field] = fieldValue;
  }
  for (auto [field, fieldValue] : attr.numAttributes){
    setAttribute(gameobj, field, fieldValue);
    gameobj.attr.numAttributes[field] = fieldValue;
  }
  for (auto [field, fieldValue] : attr.vecAttr.vec3){
    setAttribute(gameobj, field, fieldValue);
    gameobj.attr.vecAttr.vec3[field] = fieldValue;
  }
  for (auto [field, fieldValue] : attr.vecAttr.vec4){
    setAttribute(gameobj, field, fieldValue);
    gameobj.attr.vecAttr.vec4[field] = fieldValue;
  }
}

std::string physicsShapeValue(GameObject& gameobject){
  if (gameobject.physicsOptions.shape == BOX){
    return "shape_box"; 
  }
  if (gameobject.physicsOptions.shape == SPHERE){
    return "shape_sphere"; 
  }
  if (gameobject.physicsOptions.shape == CAPSULE){
    return "shape_capsule"; 
  }
  if (gameobject.physicsOptions.shape == CYLINDER){
    return "shape_cylinder"; 
  } 
  if (gameobject.physicsOptions.shape == CONVEXHULL){
    return "shape_hull"; 
  } 
  if (gameobject.physicsOptions.shape == SHAPE_EXACT){
    return "shape_exact";
  }
  if (gameobject.physicsOptions.shape == AUTOSHAPE){
    return "shape_auto";
  }
  assert(false);
  return "";
}

void getAllAttributes(GameObject& gameobj, GameobjAttributes& _attr){
  _attr.vecAttr.vec3["position"] = gameobj.transformation.position;
  _attr.vecAttr.vec3["scale"] = gameobj.transformation.scale;
  _attr.vecAttr.vec4["rotation"] = serializeQuatToVec4(gameobj.transformation.rotation); // these representation transformations could happen offline 

  _attr.stringAttributes["lookat"] = gameobj.lookat;
  _attr.stringAttributes["layer"] = gameobj.layer;
  _attr.stringAttributes["script"] = gameobj.script;
  _attr.stringAttributes["fragshader"] = gameobj.fragshader;
  _attr.stringAttributes["physics_type"] = gameobj.physicsOptions.isStatic ? "static" : "dynamic";
  _attr.stringAttributes["physics_shape"] = physicsShapeValue(gameobj);
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

