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


SerializationObject getDefaultObject(std::string name, std::string layer, bool enablePhysics){
  assert(name.find(',') == std::string::npos);

  physicsOpts physics {
    .enabled = enablePhysics,
    .isStatic = true,
    .hasCollisions = true,
    .shape = AUTOSHAPE,
    .linearFactor = glm::vec3(1.f, 1.f, 1.f),
    .angularFactor = glm::vec3(1.f, 1.f, 1.f),
    .gravity = glm::vec3(0.f, -9.81f, 0.f),
    .friction = 1.0f,
    .restitution = 0.f,
    .mass = 1.f,
    .maxspeed = -1.f,
  };

  SerializationObject newObject {
    .hasId = false,
    .id = -1,
    .name = name,
    .position = glm::vec3(0.f, 0.f, 0.f),
    .scale = glm::vec3(1.f, 1.f, 1.f),
    .rotation = glm::identity<glm::quat>(),
    .physics = physics,
    .layer = layer,
    .tint = glm::vec3(1.f, 1.f, 1.f),
  };
  return newObject;
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
  auto fields = { "lookat", "script", "fragshader" };
  for (auto field : fields){
    if (attribute == field){
      attributes.stringAttributes[attribute] = payload;
      return true;
    }
  }
  return false;
}
void safeVecSet(glm::vec3* value, const char* key, GameobjAttributes& attributes){
  if (attributes.vecAttributes.find(key) != attributes.vecAttributes.end()){
    *value = attributes.vecAttributes.at(key);
  }
}
void safeFloatSet(float* value, const char* key, GameobjAttributes& attributes){
  if (attributes.numAttributes.find(key) != attributes.numAttributes.end()){
    *value = attributes.numAttributes.at(key);
  }
}
void safeStringSet(std::string* value, const char* key, GameobjAttributes& attributes){
  if (attributes.stringAttributes.find(key) != attributes.stringAttributes.end()){
    *value = attributes.stringAttributes.at(key);
  }
}
void setSerialObjFromAttr(SerializationObject& object, GameobjAttributes& attributes){
  safeVecSet(&object.position, "position", attributes);
  safeVecSet(&object.scale, "scale", attributes);
  safeVecSet(&object.physics.angularFactor, "physics_angle", attributes);
  safeVecSet(&object.physics.linearFactor, "physics_linear", attributes);
  safeVecSet(&object.physics.gravity, "physics_gravity", attributes);
  safeVecSet(&object.tint, "tint", attributes);

  safeFloatSet(&object.physics.friction, "physics_friction", attributes);
  safeFloatSet(&object.physics.restitution, "physics_restitution", attributes);
  safeFloatSet(&object.physics.mass, "physics_mass", attributes);
  safeFloatSet(&object.physics.maxspeed, "physics_maxspeed", attributes);

  safeStringSet(&object.lookat, "lookat", attributes);
  safeStringSet(&object.script, "script", attributes);
  safeStringSet(&object.fragshader, "fragshader", attributes);


}


GameobjAttributes getDefaultAttributes() {
  std::map<std::string, std::string> stringAttributes;
  std::map<std::string, double> numAttributes;
  std::map<std::string, glm::vec3> vecAttributes; 

  GameobjAttributes attributes {
    .stringAttributes = stringAttributes,
    .numAttributes = numAttributes,
    .vecAttributes = vecAttributes,
  };
  return attributes;

}




std::map<std::string, SerializationObject> deserializeSceneTokens(std::vector<Token> tokens){
  std::map<std::string, SerializationObject> objects;
  std::map<std::string, GameobjAttributes> objectAttributes;

  for (Token token : tokens){
    assert(token.target != "" && token.attribute != "" && token.payload != "");

    if (objects.find(token.target) == objects.end()) {
      objects[token.target] = getDefaultObject(token.target, token.layer, true);
      objectAttributes[token.target] = getDefaultAttributes();
    }

    if (token.attribute == "child"){
      auto children = parseChildren(token.payload);
      for (auto child : children){
        if (objects.find(child) == objects.end()){
          objects[child] = getDefaultObject(child, token.layer, true);
          objectAttributes[child] = getDefaultAttributes();
        }
      }
      objects.at(token.target).children = children;
      continue;
    }

    if (token.attribute == "id"){
      objects.at(token.target).id = std::atoi(token.payload.c_str());   
      objects.at(token.target).hasId = true;
      continue;
    }

    bool addedVecField = addVecFields(objectAttributes.at(token.target), token.attribute, token.payload);
    if (addedVecField){
      continue;
    }
    bool addedFloatField = addFloatFields(objectAttributes.at(token.target), token.attribute, token.payload);
    if (addedFloatField){
      continue;
    }
    bool addedStringField = addStringFields(objectAttributes.at(token.target), token.attribute, token.payload);
    if (addedStringField){
      continue;
    }

    if (token.attribute == "rotation"){
      objects.at(token.target).rotation = parseQuat(token.payload);
      continue;
    }
    if (token.attribute == "physics"){
      objects.at(token.target).physics.enabled = token.payload == "enabled";
      continue;
    }
    if (token.attribute == "physics_type"){
      objects.at(token.target).physics.isStatic = token.payload != "dynamic";
      continue;
    }
    if (token.attribute == "physics_shape"){
      if (token.payload == "shape_sphere"){
        objects.at(token.target).physics.shape = SPHERE;
      }
      if (token.payload == "shape_box"){
        objects.at(token.target).physics.shape = BOX;
      }
      if (token.payload == "shape_auto"){
        objects.at(token.target).physics.shape = AUTOSHAPE;
      }
      continue;
    }
    if (token.attribute == "physics_collision" && token.payload == "nocollide"){
      objects.at(token.target).physics.hasCollisions = false;
      continue;
    }

    if (token.attribute == "net" && token.payload == "sync"){
      objects.at(token.target).netsynchronize = true;
      continue;
    }
    objects.at(token.target).additionalFields[token.attribute] = token.payload;
  }

  for (auto &[name, serialObj] : objects){
    setSerialObjFromAttr(serialObj, objectAttributes.at(name));
  }
  return objects;
}

physicsOpts defaultPhysicsOpts(GameobjAttributes attributes){
  // TODO this should use a standardized way to parse these in common with serialization
  physicsOpts defaultOption = {
    .enabled = attributes.stringAttributes.find("physics") == attributes.stringAttributes.end() ? false : (attributes.stringAttributes.at("physics") == "enabled"),
    .isStatic = attributes.stringAttributes.find("physics_type") == attributes.stringAttributes.end() ? true : !(attributes.stringAttributes.at("physics_type") == "dynamic"),
    .hasCollisions = attributes.stringAttributes.find("physics_collision") == attributes.stringAttributes.end() ? false : !(attributes.stringAttributes.at("physics_collision") == "nocollide"),
    .shape = BOX,
    .linearFactor = glm::vec3(1.f, 1.f, 1.f),
    .angularFactor = glm::vec3(1.f, 1.f, 1.f),
    .gravity = attributes.vecAttributes.find("physics_gravity") == attributes.vecAttributes.end() ? glm::vec3(0.f, -9.81f, 0.f) : attributes.vecAttributes.at("physics_gravity"),
    .friction = 1.0f,
    .restitution = 0.f,
    .mass = 1.f,
    .maxspeed = -1.f,
  };
  return defaultOption;
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