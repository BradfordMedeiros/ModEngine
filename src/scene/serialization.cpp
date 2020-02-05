#include "./serialization.h"

glm::vec3 parseVec(std::string positionRaw){;
  float x, y, z;
  std::istringstream in(positionRaw);
  in >> x >> y >> z;
  return glm::vec3(x, y, z);
}
glm::quat parseQuat(std::string payload){
  glm::vec3 eulerAngles = parseVec(payload);
  glm::quat rotation = glm::quat(glm::vec3(eulerAngles.x + 0, eulerAngles.y + 0, (eulerAngles.z + M_PI)));
  return rotation;
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

std::string getType(std::string name, std::vector<Field> additionalFields){
  std::string type = "default";
  for (Field field : additionalFields){
    if (name[0] == field.prefix){
      type = field.type;
    }
  }
  return type;
}

SerializationObject getDefaultObject(std::string name, std::vector<Field> additionalFields){
  physicsOpts physics {
    .enabled = true,
    .isStatic = true,
    .hasCollisions = true,
    .shape = AUTOSHAPE
  };
  SerializationObject newObject {
    .name = name,
    .position = glm::vec3(0.f, 0.f, 0.f),
    .scale = glm::vec3(1.f, 1.f, 1.f),
    .rotation = glm::identity<glm::quat>(),
    .hasParent = false,
    .parentName = "",
    .physics = physics,
    .type = getType(name, additionalFields)
  };
  return newObject;
}

void populateAdditionalFields(std::map<std::string, SerializationObject>& objects, Token token, std::string type, std::vector<Field> additionalFields){
  for (Field field : additionalFields){
    if (field.type == type){
      for (std::string fieldName : field.additionalFields){
        if (token.attribute == fieldName){
          objects.at(token.target).additionalFields[token.attribute] = token.payload;
          return;
        }
      }
      return;
    }
  }
}


std::map<std::string, SerializationObject> deserializeScene(std::vector<Token> tokens, std::vector<Field> additionalFields){
  std::map<std::string, SerializationObject> objects;

  for (Token token : tokens){
    assert(token.target != "" && token.attribute != "" && token.payload != "");

    if (objects.find(token.target) == objects.end()) {
      objects[token.target] = getDefaultObject(token.target, additionalFields);
    }
    if (token.attribute == "parent"){   // parent is special case that creates the other object as default if it does not yet exist
      if (objects.find(token.payload) == objects.end()){
        objects[token.payload] = getDefaultObject(token.payload, additionalFields);
      }
      objects.at(token.target).hasParent = true;
      objects.at(token.target).parentName = token.payload;
    }
    if (token.attribute == "position"){
      objects.at(token.target).position = parseVec(token.payload);
    }
    if (token.attribute == "scale"){
      objects.at(token.target).scale = parseVec(token.payload);
    }
    if (token.attribute == "rotation"){
      objects.at(token.target).rotation = parseQuat(token.payload);
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
    }
    
    std::string type = getType(token.target, additionalFields);
    objects.at(token.target).type = type;
    populateAdditionalFields(objects, token, type, additionalFields);
  }
  return objects;
}

// this isn't complete output of serialization but exposes some fields
std::string serializationObjectToString(std::vector<SerializationObject> objects){
  std::string serial = "";
  for (auto obj : objects){
    serial = serial + obj.name + ":position:" + std::to_string(obj.position.x) + " " + std::to_string(obj.position.y) + " " + std::to_string(obj.position.z) + "\n";
    serial = serial + obj.name + ":scale:" + std::to_string(obj.scale.x) + " " + std::to_string(obj.scale.y) + " " + std::to_string(obj.scale.z) + "\n";
    
    if (!obj.physics.enabled){
      serial = serial + obj.name + ":physics:disabled" + "\n";
    }
    if (!obj.physics.isStatic){
      serial = serial + obj.name + ":physics:dynamic" + "\n";
    }
    if (!obj.physics.hasCollisions){
      serial = serial + obj.name + ":physics:nocollide" + "\n";
    }
    if (obj.physics.shape == SPHERE ){
      serial = serial + obj.name + ":physics:shape_sphere" + "\n";
    }
    if (obj.physics.shape == BOX){
      serial = serial + obj.name + ":physics:box" + "\n";
    }

    for (auto [field, payload] : obj.additionalFields){
      serial = serial + obj.name + ":" + field + ":" + payload + "\n";
    }
    serial = serial + "\n\n";
  }
  return serial;
}