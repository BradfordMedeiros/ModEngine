#include "./serialization.h"

// format: <token>:<attribute>:<payload>
std::optional<Token> parseToken(std::string content) {
  std::vector<std::string> rawTokens = split(content, ':');

  std::vector<std::string> validToken;
  validToken.push_back(rawTokens.at(0));
  validToken.push_back(rawTokens.at(1));
  auto lastToken = join(subvector(rawTokens, 2, rawTokens.size()), ':');
  validToken.push_back(lastToken);
  //std::cout << "tokens: " << print(validToken) << std::endl;

  if (validToken.size() != 3){
    std::cout << "invalid token: " << content << " - size = " << validToken.size() << std::endl;
    assert(false);
  }
  Token token = { 
    .target = (validToken.size() > 0) ? trim(validToken.at(0)) : "",
    .attribute = (validToken.size() > 1) ? trim(validToken.at(1)) : "",
    .payload = (validToken.size() > 2) ? trim(validToken.at(2)) : "",
  };
  if (token.target.length() > 0 && token.attribute.length() > 0 && token.payload.length() > 0){
    return token;
  }
  return std::nullopt;
}


std::vector<Token> parseFormat(std::string content) {
  std::vector<Token> dtokens;
  std::vector<std::string> lines = split(content, '\n');
  for(std::string line : lines){
    std::vector<std::string> tokens = split(line, '#');
    if (tokens.size() > 0){
      auto lineCommentsStripped = tokens.at(0);
      if (trim(lineCommentsStripped) == ""){
        continue;
      }
      auto parsedToken = parseToken(lineCommentsStripped);
      if (parsedToken.has_value()){
        dtokens.push_back(parsedToken.value());
      }    
    }
  }
  return dtokens;
}

std::string getTokenPayload(std::vector<Token>& tokens, std::string attribute){
  for (auto &token : tokens){
    if(token.attribute == attribute){
      return token.payload;
    }
  }
  assert(false);
  return "";
}

std::string serializeSceneTokens(std::vector<Token>& tokens){
  std::string sceneData = "";
  for (auto &token : tokens){
    sceneData = sceneData + token.target + ":" + token.attribute + ":" + token.payload + "\n";
  }
  return sceneData;
}


std::vector<std::string> parseChildren(std::string payload){
  return split(payload, ',');
}

bool isSubelementToken(Token& token){
  return isSubelementName(token.target);
}

// conditionally supported, but should be fine in practice, and for c++20
// either way compile time so should be fine
std::vector<AutoSerialize> gameobjSerializer {
  AutoSerializeVec3 {
    .structOffset = offsetof(GameObject, transformation.position), 
    .structOffsetFiller = std::nullopt,
    .field = "position",
    .defaultValue = glm::vec3(0.f, 0.f, 0.f),
  },
  AutoSerializeVec3 {
    .structOffset = offsetof(GameObject, transformation.scale),
    .structOffsetFiller = std::nullopt,
    .field = "scale",
    .defaultValue = glm::vec3(1.f, 1.f, 1.f),
  },
  AutoSerializeRotation {
    .structOffset = offsetof(GameObject, transformation.rotation),
    .field = "rotation",
    .defaultValue = glm::identity<glm::quat>(), // consider what the forward vector should be
  },
  AutoSerializeBool {
    .structOffset = offsetof(GameObject, physicsOptions.enabled),
    .field = "physics", 
    .onString = "enabled",
    .offString = "disabled",
    .defaultValue = false,
  },
  AutoSerializeBool {
    .structOffset = offsetof(GameObject, physicsOptions.isStatic),
    .field = "physics_type", 
    .onString = "static",
    .offString = "dynamic",
    .defaultValue = true,
  },
  AutoSerializeBool {
    .structOffset = offsetof(GameObject, physicsOptions.hasCollisions),
    .field = "physics_collision", 
    .onString = "collide",
    .offString = "nocollide",
    .defaultValue = true,
  },
  AutoSerializeEnums {
    .structOffset = offsetof(GameObject, physicsOptions.shape),
    .enums = { BOX, SPHERE, CAPSULE, CYLINDER, CONVEXHULL, SHAPE_EXACT, AUTOSHAPE },
    .enumStrings = { "shape_box", "shape_sphere", "shape_capsule", "shape_cylinder", "shape_hull", "shape_exact", "shape_auto" },
    .field = "physics_shape",
    .defaultValue = AUTOSHAPE,
  },
  AutoSerializeVec3 {
    .structOffset = offsetof(GameObject, physicsOptions.linearFactor),
    .structOffsetFiller = std::nullopt,
    .field = "physics_linear",
    .defaultValue = glm::vec3(1.f, 1.f, 1.f),
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObject, physicsOptions.linearDamping),
    .structOffsetFiller = std::nullopt,
    .field = "physics_damping_linear",
    .defaultValue = 0.f,
  },
  AutoSerializeVec3 {
    .structOffset = offsetof(GameObject, physicsOptions.angularFactor),
    .structOffsetFiller = std::nullopt,
    .field = "physics_angle",
    .defaultValue = glm::vec3(1.f, 1.f, 1.f),
  },
  AutoSerializeVec3 {
    .structOffset = offsetof(GameObject, physicsOptions.gravity),
    .structOffsetFiller = std::nullopt,
    .field = "physics_gravity",
    .defaultValue = glm::vec3(0.f, -9.81f, 0.f),
  },
  AutoSerializeVec3 {
    .structOffset = offsetof(GameObject, physicsOptions.velocity),
    .structOffsetFiller = std::nullopt,
    .field = "physics_velocity",
    .defaultValue = glm::vec3(0.f, 0.f, 0.f),
  },
  AutoSerializeVec3 {
    .structOffset = offsetof(GameObject, physicsOptions.angularVelocity),
    .structOffsetFiller = std::nullopt,
    .field = "physics_avelocity",
    .defaultValue = glm::vec3(0.f, 0.f, 0.f),
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObject, lookat),
    .field = "lookat",
    .defaultValue = "",
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObject, script),
    .field = "script",
    .defaultValue = "",
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObject, shader),
    .field = "shader",
    .defaultValue = "",
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObject, layer),
    .field = "layer",
    .defaultValue = "",
  },
  AutoSerializeBool {
    .structOffset = offsetof(GameObject, netsynchronize),
    .field = "net", 
    .onString = "sync",
    .offString = "nosync",
    .defaultValue = false,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObject, physicsOptions.mass),
    .structOffsetFiller = std::nullopt,
    .field = "physics_mass",
    .defaultValue = 1.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObject, physicsOptions.maxspeed),
    .structOffsetFiller = std::nullopt,
    .field = "physics_maxspeed",
    .defaultValue = -1.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObject, physicsOptions.friction),
    .structOffsetFiller = std::nullopt,
    .field = "physics_friction",
    .defaultValue = 1.f,
  }, 
  AutoSerializeFloat {
    .structOffset = offsetof(GameObject, physicsOptions.restitution),
    .structOffsetFiller = std::nullopt,
    .field = "physics_restitution",
    .defaultValue = 0.f,
  }, 
  AutoSerializeFloat {
    .structOffset = offsetof(GameObject, physicsOptions.layer),
    .structOffsetFiller = std::nullopt,
    .field = "physics_layer",
    .defaultValue = 0.f,
  }, 
};


void assertCoreType(AttributeValueType type, std::string& fieldname, std::string& payload){
  auto autoserializer = serializerByName(gameobjSerializer, fieldname);
  if (autoserializer.has_value()){
    auto serializerType = typeForSerializer(autoserializer.value());
    modassert(serializerType == type, std::string("mismatch type for: ") + fieldname + " should be: " + attributeTypeStr(serializerType) + " payload = " + payload);
  }
}

void addFieldDynamic(GameobjAttributes& attributes, std::string attribute, std::string payload){
  glm::vec3 vec(0.f, 0.f, 0.f);
  bool isVec = maybeParseVec(payload, vec);
  if (isVec){
    attributes.vecAttr.vec3[attribute] = vec;
    assertCoreType(ATTRIBUTE_VEC3, attribute, payload);
    return;
  }

  glm::vec4 vec4(0.f, 0.f, 0.f, 0.f);
  bool isVec4 = maybeParseVec4(payload, vec4);
  if (isVec4){
    attributes.vecAttr.vec4[attribute] = vec4;
    assertCoreType(ATTRIBUTE_VEC4, attribute, payload);
    return;
  }

  float number = 0.f;
  bool isFloat = maybeParseFloat(payload, number);
  if (isFloat){
    attributes.numAttributes[attribute] = number;
    assertCoreType(ATTRIBUTE_FLOAT, attribute, payload);
    return;
  }
  assertCoreType(ATTRIBUTE_STRING, attribute, payload);
  attributes.stringAttributes[attribute] = payload;
}
void addAttributeFieldDynamic(GameobjAttributes& attributes, std::string attribute, AttributeValue& value){
  auto vec3Field = std::get_if<glm::vec3>(&value);
  if (vec3Field != NULL){
    attributes.vecAttr.vec3[attribute] = *vec3Field;
    return;
  }
  auto vec4Field = std::get_if<glm::vec4>(&value);
  if (vec4Field != NULL){
    attributes.vecAttr.vec4[attribute] = *vec4Field;
    return;
  }
  auto numField = std::get_if<float>(&value);
  if (numField != NULL){
    attributes.numAttributes[attribute] = *numField;
    return;
  }
  auto strField = std::get_if<std::string>(&value);
  if (strField != NULL){
    attributes.stringAttributes[attribute] = *strField;
    return;
  }
  modassert(false, "add attribute dynamic invalid - " + attribute);
}

DividedTokens divideMainAndSubelementTokens(std::vector<Token> tokens){
  std::vector<Token> mainTokens;
  std::vector<Token> subelementTokens;
  for (auto &token : tokens){
    bool isSubelement = isSubelementToken(token);
    if (isSubelement){
      subelementTokens.push_back(token);
    }else{
      mainTokens.push_back(token);
    }
  }
  std::cout << std::endl;
  DividedTokens dividedTokens {
    .mainTokens = mainTokens,
    .subelementTokens = subelementTokens,
  };
  return dividedTokens;
}
std::map<std::string, AttrChildrenPair> deserializeSceneTokens(std::vector<Token> tokens){
  std::map<std::string, AttrChildrenPair> objectAttributes;

  for (Token token : tokens){
    assert(token.target != "" && token.attribute != "" && token.payload != "");

    if (objectAttributes.find(token.target) == objectAttributes.end()) {
      assert(token.target.find(',') == std::string::npos);
      objectAttributes[token.target] = AttrChildrenPair {
        .attr = GameobjAttributes { },
        .children = {},
      };
    }

    if (token.attribute == "child"){
      auto children = parseChildren(token.payload);
      for (auto child : children){
        if (objectAttributes.find(child) == objectAttributes.end()){
          objectAttributes[child] = AttrChildrenPair {
            .attr = GameobjAttributes { },
            .children = {},
          };
        }
      }
      objectAttributes.at(token.target).children = children;
      continue;
    }

   
    addFieldDynamic(objectAttributes.at(token.target).attr, token.attribute, token.payload);
  }

  return objectAttributes;
}


std::vector<std::pair<std::string, std::string>> coreFields(GameObject& gameobject){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&gameobject, gameobjSerializer, pairs);
  return pairs;
}

std::vector<std::pair<std::string, std::string>> uniqueAdditionalFields(GameObject& gameobject, std::map<std::string, std::string>& serializedPairs){
  std::vector<std::pair<std::string, std::string>> fields;
  for (auto &[field, value] : gameobject.additionalAttr.stringAttributes){
    modassert(serializedPairs.find(field) == serializedPairs.end(), std::string("serialization invalid obj state: ") + field);
    fields.push_back({ field, value });
  }
  for (auto &[field, value] : gameobject.additionalAttr.numAttributes){
    modassert(serializedPairs.find(field) == serializedPairs.end(), std::string("serialization invalid obj state: ") + field);
    fields.push_back({ field, serializeFloat(value) });
  }
  for (auto &[field, value] : gameobject.additionalAttr.vecAttr.vec3){
    modassert(serializedPairs.find(field) == serializedPairs.end(), std::string("serialization invalid obj state: ") + field);
    fields.push_back({ field, serializeVec(value) });
  }
  for (auto &[field, value] : gameobject.additionalAttr.vecAttr.vec4){
    modassert(serializedPairs.find(field) == serializedPairs.end(), std::string("serialization invalid obj state: ") + field);
    fields.push_back({ field, serializeVec(value) });
  }
  return fields;
}

std::vector<std::string> reservedMultiObjFields = { "shader", "layer", "position", "scale" };
bool isReservedSubobjectAttribute(const char* field){
  for (int i = 0; i < reservedMultiObjFields.size(); i++){
    if (reservedMultiObjFields.at(i) == field){
      return true;
    }
  }
  return false;
}
GameobjAttributes defaultAttributesForMultiObj(Transformation transform, GameObject& gameobj, GameobjAttributes& additionalFields){
  MODTODO("default inheritance of attributes...should there really be any (aside from transform)");
  GameobjAttributes attributes {
    .stringAttributes = {
      {"shader", gameobj.shader},
      {"layer", gameobj.layer},
    },
    .vecAttr = {
      .vec3 = {
        {"position", transform.position },
        {"scale",    transform.scale    },
        // 
      },
      .vec4 = {},
    },
  };
  //std::cout << "Default attributes: shader = " << gameobj.shader << ", layer = " << gameobj.layer << ", position = " << print(transform.position) << ", scale = " << print(transform.scale) << std::endl;
  mergeAttributes(attributes, additionalFields);
  return attributes;
}

std::string serializeObj(
  objid id, 
  objid groupId, 
  GameObject& gameobject, 
  std::vector<std::string> children, 
  bool includeIds, 
  std::vector<std::pair<std::string, std::string>> additionalFields, 
  std::string name
){
  std::string sceneData = "";
  std::cout << "serializing object: " << gameobject.name << std::endl;

  bool isSubobject = groupId != id;

  std::string gameobjectName = name == "" ? gameobject.name : name;

  // custom autoserializer
  if (children.size() > 0){
    sceneData = sceneData + gameobjectName + ":child:" + join(children, ',') + "\n";  
  }

  if (includeIds){
    sceneData = sceneData + gameobjectName + ":id:" + std::to_string(gameobject.id) + "\n";
  }

  std::map<std::string, std::string> serializedPairs;
  for (auto &additionalField : coreFields(gameobject)){
    serializedPairs[additionalField.first] = additionalField.second;
  }
  for (auto &additionalField : additionalFields){
    serializedPairs[additionalField.first] = additionalField.second;
  }
  for (auto &additionalField : uniqueAdditionalFields(gameobject, serializedPairs)){
    serializedPairs[additionalField.first] = additionalField.second;
  }

  for (auto &[attribute, payload] : serializedPairs){
    if (isSubobject && isReservedSubobjectAttribute(attribute.c_str())){
      continue;
    }
    sceneData = sceneData + gameobjectName + ":" + attribute + ":" + payload + "\n";
  }

  return sceneData;  
}

void getAllAttributes(GameObject& gameobj, GameobjAttributes& _attr){
  autoserializerGetAttr((char*)&gameobj, gameobjSerializer, _attr);
}

void setAllAttributes(GameObject& gameobj, GameobjAttributes& attr, ObjectSetAttribUtil& util){
  autoserializerSetAttrWithTextureLoading((char*)&gameobj, gameobjSerializer, attr, util);
  modassert(attr.stringAttributes.find("script") == attr.stringAttributes.end(), "setting script attr not yet supported");
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

bool isReservedAttribute(std::string field, std::set<std::string>& autoserializerFields){
  return autoserializerFields.count(field) > 0;
}
GameobjAttributes getAdditionalAttr(GameobjAttributes& attributes, std::set<std::string>& autoserializerFields){
  GameobjAttributes extraLabels {
    .stringAttributes = {},
    .numAttributes = {},
    .vecAttr = vectorAttributes {
      .vec3 = {},
      .vec4 = {},
    },
  };
  for (auto &[key, value] : attributes.stringAttributes){
    if (!isReservedAttribute(key, autoserializerFields)){
      extraLabels.stringAttributes[key] = value;
    }
  }
  for (auto &[key, value] : attributes.numAttributes){
    if (!isReservedAttribute(key, autoserializerFields)){
      extraLabels.numAttributes[key] = value;
    }
  }
  for (auto &[key, value] : attributes.vecAttr.vec3){
    if (!isReservedAttribute(key, autoserializerFields)){
      extraLabels.vecAttr.vec3[key] = value;
    }
  }
  for (auto &[key, value] : attributes.vecAttr.vec4){
    if (!isReservedAttribute(key, autoserializerFields)){
      extraLabels.vecAttr.vec4[key] = value;
    }
  }
  return extraLabels;
}

GameObject gameObjectFromFields(std::string name, objid id, GameobjAttributes attributes, std::set<std::string> objautoserializerFields){
  GameObject object = { .id = id, .name = name };
  createAutoSerialize((char*)&object, gameobjSerializer, attributes);
  if (attributes.stringAttributes.find("id") != attributes.stringAttributes.end()){
    object.id = std::atoi(attributes.stringAttributes.at("id").c_str());
  }

  std::set<std::string> autoserializerFields = serializerFieldNames(gameobjSerializer);
  for (auto &field : objautoserializerFields){
    autoserializerFields.insert(field);
  }

  object.additionalAttr = getAdditionalAttr(attributes, autoserializerFields);

  //std::cout << "additional attr: " << name << std::endl;
  //std::cout << print(object.additionalAttr) << std::endl << std::endl;
  ////object.attr = attributes; // lots of redundant information here, should only set attrs that aren't consumed elsewhere
//
//  //std::cout << "gameobj from fields: " << name << std::endl;
//  //std::cout << print(object.attr) << std::endl;
  //std::cout << std::endl;

  return object;
}


