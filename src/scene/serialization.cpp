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
  assert(token.target.find(',') == std::string::npos);
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

std::string getType(std::string name, std::vector<Field> additionalFields){
  std::string type = "default";
  for (Field field : additionalFields){
    if (name[0] == field.prefix){
      type = field.type;
    }
  }
  return type;
}

SerializationObject getDefaultObject(std::string name, std::vector<Field> additionalFields, std::string layer){
  physicsOpts physics {
    .enabled = true,
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
    .hasParent = false,
    .parentName = "",
    .physics = physics,
    .type = getType(name, additionalFields),
    .layer = layer
  };
  return newObject;
}

std::vector<std::string> parseChildren(std::string payload){  // TODO - redundant children ? 
  std::vector<std::string> children;
  return split(payload, ',');
}

std::map<std::string, SerializationObject> deserializeSceneTokens(std::vector<Token> tokens, std::vector<Field> additionalFields){
  std::map<std::string, SerializationObject> objects;

  for (Token token : tokens){
    assert(token.target != "" && token.attribute != "" && token.payload != "");

    if (objects.find(token.target) == objects.end()) {
      objects[token.target] = getDefaultObject(token.target, additionalFields, token.layer);
      objects.at(token.target).type = getType(token.target, additionalFields);
    }

    if (token.attribute == "parent"){   // parent is special case that creates the other object as default if it does not yet exist, inherits layer declared in
      if (objects.find(token.payload) == objects.end()){
        objects[token.payload] = getDefaultObject(token.payload, additionalFields, token.layer);
        objects.at(token.payload).type = getType(token.payload, additionalFields);
      }
      objects.at(token.target).hasParent = true;
      objects.at(token.target).parentName = token.payload;
      continue;
    }
    if (token.attribute == "children"){
      auto children = parseChildren(token.payload);
      objects.at(token.target).children = children;
      for(auto child : children){
        if (objects.find(child) == objects.end()){
          objects[child] = getDefaultObject(child, additionalFields, token.layer);
        }
      }
    }

    if (token.attribute == "id"){
      objects.at(token.target).id = std::atoi(token.payload.c_str());   
      objects.at(token.target).hasId = true;
      continue;
    }
    if (token.attribute == "position"){
      objects.at(token.target).position = parseVec(token.payload);
      continue;
    }
    if (token.attribute == "scale"){
      objects.at(token.target).scale = parseVec(token.payload);
      continue;
    }
    if (token.attribute == "rotation"){
      objects.at(token.target).rotation = parseQuat(token.payload);
      continue;
    }

    if (token.attribute == "physics"){
      if (token.payload == "enabled"){
        objects.at(token.target).physics.enabled = true;
      }
      if (token.payload == "disabled"){
        objects.at(token.target).physics.enabled = false;
      }
      if (token.payload == "dynamic"){
        objects.at(token.target).physics.isStatic = false;
      }
      if (token.payload == "nocollide"){
        objects.at(token.target).physics.hasCollisions = false;
      }
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
    if (token.attribute == "physics_angle"){
      objects.at(token.target).physics.angularFactor = parseVec(token.payload);
      continue;
    }
    if (token.attribute == "physics_linear"){
      objects.at(token.target).physics.linearFactor = parseVec(token.payload);
      continue;
    }
    if (token.attribute == "physics_gravity"){
      objects.at(token.target).physics.gravity = parseVec(token.payload);
      continue;
    }
    if (token.attribute == "physics_friction"){
      objects.at(token.target).physics.friction = std::atof(token.payload.c_str());
      continue;
    }
    if (token.attribute == "physics_restitution"){
      objects.at(token.target).physics.restitution = std::atof(token.payload.c_str());
      continue;
    }
    if (token.attribute == "physics_mass"){
      objects.at(token.target).physics.mass =  std::atof(token.payload.c_str());
      continue;
    }
    if (token.attribute == "physics_maxspeed"){
      objects.at(token.target).physics.maxspeed = std::atof(token.payload.c_str());
      continue;
    }

    if (token.attribute == "lookat"){
      objects.at(token.target).lookat = token.payload;
      continue;
    }
    if (token.attribute == "script"){
      objects.at(token.target).script = token.payload;
      continue;
    }
    if (token.attribute == "net" && token.payload == "sync"){
      objects.at(token.target).netsynchronize = true;
      continue;
    }
    objects.at(token.target).additionalFields[token.attribute] = token.payload;
  }
  return objects;
}

