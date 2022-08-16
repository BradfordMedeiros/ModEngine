#include "./serialization.h"

// format: <token>:<attribute>:<payload>
std::optional<Token> parseToken(std::string content) {
  std::vector<std::string> validToken = split(content, ':');
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

bool addVec3Fields(GameobjAttributes& attributes, std::string attribute, std::string payload){
  auto fields = { "position", "scale", "physics_angle", "physics_linear", "physics_gravity", "physics_velocity", "physics_avelocity" };
  for (auto field : fields){
    if (attribute == field){
      attributes.vecAttr.vec3[attribute] = parseVec(payload);
      return true;
    }
  }
  return false;
}
bool addVec4Fields(GameobjAttributes& attributes, std::string attribute, std::string payload){
  auto fields = { "rotation" };
  for (auto field : fields){
    if (attribute == field){
      attributes.vecAttr.vec4[attribute] = parseVec4(payload);
      return true;
    }
  }
  return false;
}
bool addFloatFields(GameobjAttributes& attributes, std::string attribute, std::string payload){
  auto fields = { "physics_friction", "physics_restitution", "physics_mass", "physics_maxspeed", "physics_layer" };
  for (auto field : fields){
    if (attribute == field){
      attributes.numAttributes[attribute] = std::atof(payload.c_str());
      return true;
    }
  }
  return false;
}
bool addStringFields(GameobjAttributes& attributes, std::string attribute, std::string payload){
  auto fields = { "layer", "lookat", "script", "fragshader", "physics", "physics_collision", "physics_type", "physics_shape", "net", "id" };
  for (auto field : fields){
    if (attribute == field){
      attributes.stringAttributes[attribute] = payload;
      return true;
    }
  }
  return false;
}
bool addFields(GameobjAttributes& attributes, std::string attribute, std::string payload){
  bool addedVecField = addVec3Fields(attributes, attribute, payload);
  if (addedVecField){
    return true;
  }
  bool addedVec4Field = addVec4Fields(attributes, attribute, payload);
  if (addedVec4Field){
    return true;
  }
  bool addedFloatField = addFloatFields(attributes, attribute, payload);
  if (addedFloatField){
    return true;
  }
  bool addedStringField = addStringFields(attributes, attribute, payload);
  if (addedStringField){
    return true;
  }
  return false;
}

void addFieldDynamic(GameobjAttributes& attributes, std::string attribute, std::string payload){
  glm::vec3 vec(0.f, 0.f, 0.f);
  bool isVec = maybeParseVec(payload, vec);
  if (isVec){
    attributes.vecAttr.vec3[attribute] = vec;
    return;
  }

  glm::vec4 vec4(0.f, 0.f, 0.f, 0.f);
  bool isVec4 = maybeParseVec4(payload, vec4);
  if (isVec4){
    attributes.vecAttr.vec4[attribute] = vec4;
    return;
  }

  float number = 0.f;
  bool isFloat = maybeParseFloat(payload, number);
  if (isFloat){
    attributes.numAttributes[attribute] = number;
    return;
  }
  attributes.stringAttributes[attribute] = payload;
}

std::string mainTargetElement(std::string target){
  return split(target, '/').at(0);
}
bool isSubelementToken(Token& token){
  auto numTokens = split(token.target, '/');
  return numTokens.size() > 1;
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
std::map<std::string, GameobjAttributes> deserializeSceneTokens(std::vector<Token> tokens){
  std::map<std::string, GameobjAttributes> objectAttributes;

  for (Token token : tokens){
    assert(token.target != "" && token.attribute != "" && token.payload != "");

    if (objectAttributes.find(token.target) == objectAttributes.end()) {
      assert(token.target.find(',') == std::string::npos);
      objectAttributes[token.target] = GameobjAttributes {};
    }

    if (token.attribute == "child"){
      auto children = parseChildren(token.payload);
      for (auto child : children){
        if (objectAttributes.find(child) == objectAttributes.end()){
          objectAttributes[child] = GameobjAttributes { };
        }
      }
      objectAttributes.at(token.target).children = children;
      continue;
    }

    bool addedField = addFields(objectAttributes.at(token.target), token.attribute, token.payload);
    if (addedField){
      continue;
    }  
    addFieldDynamic(objectAttributes.at(token.target), token.attribute, token.payload);
  }

  return objectAttributes;
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
    .structOffset = offsetof(GameObject, fragshader),
    .field = "fragshader",
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

};

std::vector<std::pair<std::string, std::string>> coreFields(GameObject& gameobject){
  std::vector<std::pair<std::string, std::string>> pairs;
  pairs.push_back({ "rotation", serializeQuat(gameobject.transformation.rotation) });
  autoserializerSerialize((char*)&gameobject, gameobjSerializer, pairs);
  return pairs;
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
  if (groupId != id){
    return sceneData;
  }
  std::string gameobjectName = name == "" ? gameobject.name : name;

  // custom autoserializer
  if (children.size() > 0){
    sceneData = sceneData + gameobjectName + ":child:" + join(children, ',') + "\n";  
  }

  if (includeIds){
    sceneData = sceneData + gameobjectName + ":id:" + std::to_string(gameobject.id) + "\n";
  }

  for (auto additionalField : coreFields(gameobject)){
    sceneData = sceneData + gameobjectName + ":" + additionalField.first + ":" + additionalField.second + "\n";
  }

  for (auto additionalField : additionalFields){
    sceneData = sceneData + gameobjectName + ":" + additionalField.first + ":" + additionalField.second + "\n";
  }
  return sceneData;  
}

void getAllAttributes(GameObject& gameobj, GameobjAttributes& _attr){
  _attr.vecAttr.vec4["rotation"] = serializeQuatToVec4(gameobj.transformation.rotation); // these representation transformations could happen offline 
  autoserializerGetAttr((char*)&gameobj, gameobjSerializer, _attr);

}