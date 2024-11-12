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
  auto autoserializer = getAutoserializeByField(gameobjSerializer, fieldname.c_str());
  if (autoserializer.has_value()){
    auto serializerType = typeForSerializer(*(autoserializer.value()));
    modassert(serializerType == type, std::string("mismatch type for: ") + fieldname + " should be: " + attributeTypeStr(serializerType) + ", but got = " + attributeTypeStr(type) + " payload = " + payload);
  }
}

// property suffix looks like the parts of the tokens on the right hand side
// eg position 10
// eg tint 0.9 0.2 0.4
AttributeValue parsePropertySuffix(std::string attribute, std::string payload){
  glm::vec2 vec2(0.f, 0.f);
  bool isVec2 = maybeParseVec2(payload, vec2);
  if (isVec2){
    return vec2;
  }

  glm::vec3 vec(0.f, 0.f, 0.f);
  bool isVec = maybeParseVec(payload, vec);
  if (isVec){
    return vec;
  }

  glm::vec4 vec4(0.f, 0.f, 0.f, 0.f);
  bool isVec4 = maybeParseVec4(payload, vec4);
  if (isVec4){
    return vec4;
  }

  float number = 0.f;
  bool isFloat = maybeParseFloat(payload, number);
  if (isFloat){
    return number;
  }

  modassert(false, "return string...we sure?");
  return payload;
}

void addFieldDynamic(GameobjAttributes& attributes, std::string attribute, std::string payload){
  glm::vec2 vec2(0.f, 0.f);
  bool isVec2 = maybeParseVec2(payload, vec2);
  if (isVec2){
    attributes.attr[attribute] = vec2;
    assertCoreType(ATTRIBUTE_VEC2, attribute, payload);
    return;
  }

  glm::vec3 vec(0.f, 0.f, 0.f);
  bool isVec = maybeParseVec(payload, vec);
  if (isVec){
    attributes.attr[attribute] = vec;
    assertCoreType(ATTRIBUTE_VEC3, attribute, payload);
    return;
  }

  glm::vec4 vec4(0.f, 0.f, 0.f, 0.f);
  bool isVec4 = maybeParseVec4(payload, vec4);
  if (isVec4){
    attributes.attr[attribute] = vec4;
    assertCoreType(ATTRIBUTE_VEC4, attribute, payload);
    return;
  }

  float number = 0.f;
  bool isFloat = maybeParseFloat(payload, number);
  if (isFloat){
    attributes.attr[attribute] = number;
    assertCoreType(ATTRIBUTE_FLOAT, attribute, payload);
    return;
  }
  assertCoreType(ATTRIBUTE_STRING, attribute, payload);
  attributes.attr[attribute] = payload;
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
    modassert(token.target != "" && token.attribute != "" && token.payload != "", "empty tokens");
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
  for (auto &[field, value] : gameobject.additionalAttr.attr){
    modassert(serializedPairs.find(field) == serializedPairs.end(), std::string("serialization invalid obj state: ") + field);
    auto strValue = std::get_if<std::string>(&value);
    auto floatValue = std::get_if<float>(&value);
    auto vec2Value = std::get_if<glm::vec2>(&value);
    auto vec3Value = std::get_if<glm::vec3>(&value);
    auto vec4Value = std::get_if<glm::vec4>(&value);
    if (strValue){
      fields.push_back({ field, *strValue });
    }else if (floatValue){
      fields.push_back({ field, serializeFloat(*floatValue) });
    }else if (vec2Value){
      fields.push_back({ field, serializeVec(*vec2Value) });
    }else if (vec3Value){
      fields.push_back({ field, serializeVec(*vec3Value) });
    }else if (vec4Value){
      fields.push_back({ field, serializeVec(*vec4Value) });
    }else{
      modassert(false, "invalid type uniqueAdditionalFields");
    }
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
    .attr = {
      {"shader", gameobj.shader},
      {"layer", gameobj.layer},
      {"position", transform.position },
      {"scale",    transform.scale    },
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

std::optional<AttributeValuePtr> getAttributePtr(GameObject& gameobj, const char* field){
  return getAttributePtr((char*)&gameobj, gameobjSerializer, field);
}

bool setAttribute(GameObject& gameobj, const char* field, AttributeValue value, ObjectSetAttribUtil& util){
  modassert(std::string(field) != "script", "setting script attr not yet supported");
  return autoserializerSetAttrWithTextureLoading((char*)&gameobj, gameobjSerializer, field, value, util);
}

bool isReservedAttribute(std::string field, std::set<std::string>& autoserializerFields){
  return autoserializerFields.count(field) > 0;
}
GameobjAttributes getAdditionalAttr(GameobjAttributes& attributes, std::set<std::string>& autoserializerFields){
  GameobjAttributes extraLabels {
    .attr = {},
  };
  for (auto &[key, value] : attributes.attr){
    if (!isReservedAttribute(key, autoserializerFields)){
      extraLabels.attr[key] = value;
    }
  }
  return extraLabels;
}

std::optional<objid> objIdFromAttribute(GameobjAttributes& attr){
  if (attr.attr.find("id") == attr.attr.end()){
    return std::nullopt;
  }

  auto strValue = std::get_if<std::string>(&attr.attr.at("id"));
  if (!strValue){
    return std::nullopt;
  }
  return std::atoi(strValue -> c_str());
}

GameObject gameObjectFromFields(std::string name, objid id, GameobjAttributes attributes, std::set<std::string> objautoserializerFields, bool isBone){
  GameObject object = { .id = id, .name = name };
  createAutoSerialize((char*)&object, gameobjSerializer, attributes);

  auto objId = objIdFromAttribute(attributes);
  if (objId.has_value()){
    object.id = objId.value();
  }

  std::set<std::string> autoserializerFields = serializerFieldNames(gameobjSerializer);
  for (auto &field : objautoserializerFields){
    autoserializerFields.insert(field);
  }

  object.additionalAttr = getAdditionalAttr(attributes, autoserializerFields);
  object.isBone = isBone;
  return object;
}


