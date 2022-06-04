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

bool isDefaultPosition(glm::vec3 pos){
  return pos.x == 0 && pos.y == 0 && pos.z == 0;
}
bool isDefaultGravity(glm::vec3 gravity){
  return gravity.x == 0 && (gravity.y < -9.80 && gravity.y > -9.82) && gravity.z == 0;
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

  if (children.size() > 0){
    sceneData = sceneData + gameobjectName + ":child:" + join(children, ',') + "\n";  
  }

  if (includeIds){
    sceneData = sceneData + gameobjectName + ":id:" + std::to_string(gameobject.id) + "\n";
  }

  if (!isDefaultPosition(gameobject.transformation.position)){
    sceneData = sceneData + gameobjectName + ":position:" + serializeVec(gameobject.transformation.position) + "\n";
  }
  if (!isIdentityVec(gameobject.transformation.scale)){
    sceneData = sceneData + gameobjectName + ":scale:" + serializeVec(gameobject.transformation.scale) + "\n";
  }
  sceneData = sceneData + gameobjectName + ":rotation:" + serializeQuat(gameobject.transformation.rotation) + "\n";

  if (gameobject.physicsOptions.enabled){
    sceneData = sceneData + gameobjectName + ":physics:enabled" + "\n"; 
  }
  if (!gameobject.physicsOptions.isStatic){
    sceneData = sceneData + gameobjectName + ":physics_type:dynamic" + "\n"; 
  }
  if (!gameobject.physicsOptions.hasCollisions){
    sceneData = sceneData + gameobjectName + ":physics_collision:nocollide" + "\n"; 
  }

  sceneData = sceneData + gameobjectName + ":physics_shape:" + physicsShapeValue(gameobject) + "\n";
 
  if (!isIdentityVec(gameobject.physicsOptions.linearFactor)){
    sceneData = sceneData + gameobjectName + ":physics_linear:" + serializeVec(gameobject.physicsOptions.linearFactor) + "\n"; 
  }
  if (!isIdentityVec(gameobject.physicsOptions.angularFactor)){
    sceneData = sceneData + gameobjectName + ":physics_angle:" + serializeVec(gameobject.physicsOptions.angularFactor) + "\n"; 
  }
  if (!isDefaultGravity(gameobject.physicsOptions.gravity)){
    sceneData = sceneData + gameobjectName + ":physics_gravity:" + serializeVec(gameobject.physicsOptions.gravity) + "\n"; 
  }
  if (gameobject.lookat != ""){
    sceneData = sceneData + gameobjectName + ":lookat:" + gameobject.lookat + "\n"; 
  }
  if (gameobject.script != ""){
    sceneData = sceneData + gameobjectName + ":script:" + gameobject.script + "\n"; 
  }
  if (gameobject.fragshader != ""){
    sceneData = sceneData + gameobjectName + ":fragshader:" + gameobject.fragshader + "\n";
  }
  if (gameobject.netsynchronize){
    sceneData = sceneData + gameobjectName + ":net:sync" + "\n";
  }

  if (gameobject.layer != ""){
    sceneData = sceneData + gameobjectName + ":layer:" + gameobject.layer + "\n";
  }

  for (auto additionalField : additionalFields){
    sceneData = sceneData + gameobjectName + ":" + additionalField.first + ":" + additionalField.second + "\n";
  }
  return sceneData;  
}