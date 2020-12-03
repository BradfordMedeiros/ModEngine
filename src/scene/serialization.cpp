#include "./serialization.h"

// format: <token>:<attribute>:<payload>
std::optional<Token> parseToken(std::string content, std::string layer) {
  std::vector<std::string> validToken = split(content, ':');
  Token token = { 
    .target = (validToken.size() > 0) ? trim(validToken.at(0)) : "",
    .attribute = (validToken.size() > 1) ? trim(validToken.at(1)) : "",
    .payload = (validToken.size() > 2) ? trim(validToken.at(2)) : "",
    .layer = layer,
  };
  if (token.target.length() > 0 && token.attribute.length() > 0 && token.payload.length() > 0){
    return token;
  }
  return std::nullopt;
}

// format: (<layername>:zindex:<layername>)
std::optional<LayerInfo> parseLayer(std::string content){
  if (content.size() <= 2 || content.at(0) != '(' || content.at(content.size() - 1) != ')'){
    return std::nullopt;
  }


// TODO - notice this and reduce layers isn't actually allowed more than one statement. Need to join the set of them.
std::vector<std::string> tokens = split(content.substr(1, content.size() - 2), ':');
  bool isLayerContent = (tokens.size() == 3) &&  (tokens.at(1) == "zindex" || tokens.at(1) == "projection");
  if (!isLayerContent){
    return std::nullopt;
  }
  LayerInfo layer {
    .name = trim(tokens.at(0)),
    .zIndex = tokens.at(1) != "zindex" ? 0 : std::stoi(trim(tokens.at(2))),
    .orthographic = tokens.at(1) != "projection" ? false : (tokens.at(2) == "orthographic")
  };
  return layer;
}
std::vector<LayerInfo> reduceLayers(std::vector<LayerInfo> layers){
  std::map<std::string, LayerInfo> reducedLayers;
  for (auto layer : layers){
    reducedLayers[layer.name] = layer;
  }
  std::vector<LayerInfo> newLayers;
  for (auto [_, layer] : reducedLayers){
    newLayers.push_back(layer);    
  }
  return newLayers;
}

ParsedContent parseFormat(std::string content) {
  std::vector<Token> dtokens;
  std::vector<LayerInfo> layers;

  std::string currentLayer = "default";  
  layers.push_back(LayerInfo{
    .name = currentLayer,
    .zIndex = 0,
  });

  std::vector<std::string> lines = split(content, '\n');
  for(std::string line : lines){
    std::vector<std::string> tokens = split(line, '#');
    if (tokens.size() > 0){
      auto lineCommentsStripped = tokens.at(0);
      auto layer = parseLayer(lineCommentsStripped);
      if (layer.has_value()){
        currentLayer = layer.value().name;
        layers.push_back(layer.value());
      }else{
        auto parsedToken = parseToken(lineCommentsStripped, currentLayer);
        if (parsedToken.has_value()){
          dtokens.push_back(parsedToken.value());
        }    
      }
    }
  }

  ParsedContent parsedContent {
    .tokens = dtokens,
    .layers = reduceLayers(layers),
  };
  return parsedContent;
}


std::vector<std::string> parseChildren(std::string payload){
  return split(payload, ',');
}

bool addVecFields(GameobjAttributes& attributes, std::string attribute, std::string payload){
  auto fields = { "position", "scale", "physics_angle", "physics_linear", "physics_gravity", "tint"};
  for (auto field : fields){
    if (attribute == field){
      attributes.vecAttributes[attribute] = parseVec(payload);
      return true;
    }
  }
  return false;
}
bool addFloatFields(GameobjAttributes& attributes, std::string attribute, std::string payload){
  auto fields = { "physics_friction", "physics_restitution", "physics_mass", "physics_maxspeed" };
  for (auto field : fields){
    if (attribute == field){
      attributes.numAttributes[attribute] = std::atof(payload.c_str());
      return true;
    }
  }
  return false;
}
bool addStringFields(GameobjAttributes& attributes, std::string attribute, std::string payload){
  auto fields = { "lookat", "script", "fragshader", "physics", "physics_collision", "physics_type", "physics_shape", "net", "id", "rotation"};
  for (auto field : fields){
    if (attribute == field){
      attributes.stringAttributes[attribute] = payload;
      return true;
    }
  }
  return false;
}
bool addFields(GameobjAttributes& attributes, std::string attribute, std::string payload){
  bool addedVecField = addVecFields(attributes, attribute, payload);
  if (addedVecField){
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

std::map<std::string, GameobjAttributes> deserializeSceneTokens(std::vector<Token> tokens){
  std::map<std::string, GameobjAttributes> objectAttributes;

  for (Token token : tokens){
    assert(token.target != "" && token.attribute != "" && token.payload != "");

    if (objectAttributes.find(token.target) == objectAttributes.end()) {
      assert(token.target.find(',') == std::string::npos);
      objectAttributes[token.target] = GameobjAttributes { 
        .layer = token.layer,
      };
    }

    if (token.attribute == "child"){
      auto children = parseChildren(token.payload);
      for (auto child : children){
        if (objectAttributes.find(child) == objectAttributes.end()){
          objectAttributes[child] = GameobjAttributes { 
            .layer = token.layer,
          };
        }
      }
      objectAttributes.at(token.target).children = children;
      continue;
    }

    bool addedField = addFields(objectAttributes.at(token.target), token.attribute, token.payload);
    if (addedField){
      continue;
    }  
    objectAttributes.at(token.target).additionalFields[token.attribute] = token.payload;
  }

  return objectAttributes;
}

GameobjAttributes fieldsToAttributes(std::map<std::string, std::string> fields){
  std::cout << "INFO: SERIALIZATION WARNING: -- FIELD TO ATTRIBUTES NOT YET IMPLEMENTED" << std::endl;
  std::map<std::string, double> numAttributes;
  std::map<std::string, glm::vec3> vecAttributes;
  vecAttributes["physics_gravity"] = glm::vec3(0.f, -1.f, 0.f);
  vecAttributes["position"] = glm::vec3(0.f, 0.f, 0.f);
  GameobjAttributes attributes {
    .stringAttributes = fields,
    .numAttributes = numAttributes,
    .vecAttributes = vecAttributes,
  };
  return attributes; 
}

bool isDefaultPosition(glm::vec3 pos){
  return pos.x == 0 && pos.y == 0 && pos.z == 0;
}
bool isIdentityVec(glm::vec3 scale){
  return scale.x = 1 && scale.y == 1 && scale.z == 1;
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
  sceneData = sceneData + gameobjectName + ":rotation:" + serializeRotation(gameobject.transformation.rotation) + "\n";

  if (!gameobject.physicsOptions.enabled){
    sceneData = sceneData + gameobjectName + ":physics:disabled" + "\n"; 
  }
  if (!gameobject.physicsOptions.isStatic){
    sceneData = sceneData + gameobjectName + ":physics_type:dynamic" + "\n"; 
  }
  if (!gameobject.physicsOptions.hasCollisions){
    sceneData = sceneData + gameobjectName + ":physics_collision:nocollide" + "\n"; 
  }
  if (gameobject.physicsOptions.shape == BOX){
    sceneData = sceneData + gameobjectName + ":physics_shape:shape_box" + "\n"; 
  }
  if (gameobject.physicsOptions.shape == SPHERE){
    sceneData = sceneData + gameobjectName + ":physics_shape:shape_sphere" + "\n"; 
  }
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
  if (!isIdentityVec(gameobject.tint)){
    sceneData = sceneData + gameobjectName + ":tint:" + serializeVec(gameobject.tint) + "\n";
  }

  for (auto additionalField : additionalFields){
    sceneData = sceneData + gameobjectName + ":" + additionalField.first + ":" + additionalField.second + "\n";
  }
  return sceneData;  
}