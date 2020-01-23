#include "./serialization.h"

glm::vec3 parseVec(std::string positionRaw){;
  float x, y, z;
  std::istringstream in(positionRaw);
  in >> x >> y >> z;
  return glm::vec3(x, y, z);
}

std::vector<Token> getTokens(std::string content) {
  std::vector<Token> dtokens;
  std::vector<std::string> lines = split(content, '\n');
  for(std::string line: lines){
    std::vector<std::string> tokens = split(line, '#');

    if (tokens.size() > 0){
      std::vector<std::string> validToken = split(tokens[0], ':');

      Token token = {};
      if (validToken.size() > 0){
        token.target = trim(validToken[0]);
      }
      if (validToken.size() > 1){
        token.attribute = trim(validToken[1]);
      }
      if (validToken.size() > 2){
        token.payload = trim(validToken[2]);
      }
      if (token.target.length() > 0 ){
        dtokens.push_back(token);
      }
    }
  }
  return dtokens;
}

SerializationObject getDefaultObject(std::string name){
  SerializationObject newObject {
    .name = name,
    .position = glm::vec3(0.f, 0.f, 0.f),
    .scale = glm::vec3(1.f, 1.f, 1.f),
    .rotation = glm::quat(0.f, 0.f, 0.f, 0.f),
  };
  return newObject;
}

std::vector<SerializationObject> deserializeScene(std::vector<Token> tokens){
  std::map<std::string, SerializationObject> objects;

  std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
  for (Token token : tokens){
    assert(token.target != "" && token.attribute != "" && token.payload != "");

    if (objects.find(token.target) == objects.end()) {
      objects[token.target] = getDefaultObject(token.target);
    }
    if (token.attribute == "parent" && (objects.find(token.payload) == objects.end())){   // parent is special case that creates the other object as default if it does not yet exist
      objects[token.payload] = getDefaultObject(token.payload);
    }
    if (token.attribute == "position"){
      objects[token.target].position = parseVec(token.payload);
    }
    if (token.attribute == "scale"){
      objects[token.target].scale = parseVec(token.payload);
    }

    if (token.attribute == "physics"){
      if (token.payload == "enabled"){
        objects[token.target].physics.enabled = true;
      }
      if (token.payload == "disabled"){
        objects[token.target].physics.enabled = false;
      }
      if (token.payload == "dynamic"){
        objects[token.target].physics.isStatic = false;
      }
      if (token.payload == "nocollide"){
        objects[token.target].physics.hasCollisions = false;
      }
      if (token.payload == "shape_sphere"){
        objects[token.target].physics.shape = SPHERE;
      }
      if (token.payload == "shape_auto"){
        objects[token.target].physics.shape = AUTOSHAPE;
      }
    }
  }
  std::cout << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
  std::vector<SerializationObject> objs;
  for (auto [_, value] : objects){
    objs.push_back(value);
  }
  return objs;

    /*std::string objectName = tok.target;

    std::string activeType = "default";
    for (Field field : fields){
      if (tok.target[0] == field.prefix ){
        activeType = field.type;
      }
    }

    if (!(scene.nameToId.find(objectName) != scene.nameToId.end())){
      addObjectToScene(scene, glm::vec3(1.0f, 1.0f, 1.0f), objectName, getNewObjectId(), -1);
      addObject(scene.nameToId[objectName], activeType, "", "");
    }

    short objectId = scene.nameToId[objectName];
    if (tok.attribute == "position"){
      scene.idToGameObjects[objectId].position = parseVec(tok.payload);
    }else if (tok.attribute == "scale"){
      scene.idToGameObjects[objectId].scale = parseVec(tok.payload);
    }else if (tok.attribute == "rotation"){
      glm::vec3 eulerAngles = parseVec(tok.payload);
      glm::quat rotation = glm::quat(glm::vec3(eulerAngles.x + 0, eulerAngles.y + 0, (eulerAngles.z + M_PI)));
      scene.idToGameObjects[objectId].rotation = rotation;
    }else if (tok.attribute == "physics"){
      auto physicsOptions = scene.idToGameObjects[objectId].physicsOptions;
      if (tok.payload == "enabled"){
        physicsOptions.enabled = true;
      }
      if (tok.payload == "disabled"){
        physicsOptions.enabled = false;
      }
      if (tok.payload == "dynamic"){
        physicsOptions.isStatic = false;
      }
      if (tok.payload == "nocollide"){
        physicsOptions.hasCollisions = false;
      }
      if (tok.payload == "shape_sphere"){
        physicsOptions.shape = SPHERE;
      }
      if (tok.payload == "shape_auto"){
        physicsOptions.shape = AUTOSHAPE;
      }
      scene.idToGameObjects[objectId].physicsOptions = physicsOptions;
    }else if (tok.attribute == "parent"){
      if (!(scene.nameToId.find(tok.payload) != scene.nameToId.end())){
        short parentId = getNewObjectId();
        addObjectToScene(scene, glm::vec3(1.0f, 1.0f, 1.0f), tok.payload, parentId, -1);
        addObject(parentId, "default", "", "");
      }
      scene.idToGameObjectsH[objectId].parentId = scene.nameToId[tok.payload];
      scene.idToGameObjectsH[scene.nameToId[tok.payload]].children.insert(scene.idToGameObjectsH[objectId].id);
    }

    for (Field field: fields){
      if (field.type != activeType){
        continue;
      }
      for (std::string field : field.additionalFields){
        if (tok.attribute == field){
          addObject(objectId, activeType, field, tok.payload);
          break;
        }
      }
    }
  }*/

}