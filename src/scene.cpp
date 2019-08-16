#include "./scene.h"

#include <sstream>

GameObject getGameObject(glm::vec3 position, Mesh& mesh, std::string name, short id, bool isRotating){
  GameObject gameObject = {
    .id = id,
    .name = name,
    .position = position,
    .scale = glm::vec3(1.0f, 1.0f, 1.0f),
    .mesh = mesh, 
    .isRotating = isRotating,
  };
  return gameObject;
}

void addObjectToScene(Scene& scene, glm::vec3 position, Mesh& mesh, std::string name, short* id, bool isRotating){
  auto gameobjectObj = getGameObject(position, mesh, name, *id, isRotating);
  *id = *id + 1;

  auto gameobjectH = GameObjectH {
    .id = gameobjectObj.id,
  };

  scene.gameObjects.push_back(gameobjectH);
  scene.idToGameObjects[gameobjectObj.id] = gameobjectObj;
  scene.nameToId[name] = gameobjectObj.id;
}

Scene loadScene(Mesh& columnSeatMesh, Mesh& boxMesh, Mesh& grassMesh){
  short id = 0;
  Scene scene;

  addObjectToScene(scene, glm::vec3(0.0f, 0.0f, 0.0f), columnSeatMesh, std::to_string(id), &id, false);

  for (unsigned int i = 0;  i < 10; i++){
    addObjectToScene(scene, glm::vec3(6.0f + (i * 3.0f), 0.0f, 0.0f), boxMesh, std::to_string(id), &id, false);
  }
  addObjectToScene(scene, glm::vec3(0.0, 1.0f, 0.0f), grassMesh, std::to_string(id), &id, true);

  return scene;
}

std::string serializeScene(Scene& scene){
  
}

struct Token {
  std::string target;
  std::string attribute;
  std::string payload;
};
std::string trim(const std::string& str){
  size_t first = str.find_first_not_of(' ');
  if (std::string::npos == first){
    return str;
  }
  size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}
std::vector<std::string> split(std::string strToSplit, char delimeter){
  std::stringstream ss(strToSplit);
  std::string item;
  std::vector<std::string> splittedStrings;
  while (std::getline(ss, item, delimeter)){
    splittedStrings.push_back(item);
  }
  return splittedStrings;
}

glm::vec3 parseVec(std::string positionRaw){;
  float x, y, z;
  std::istringstream in(positionRaw);
  in >> x >> y >> z;
  return glm::vec3(x, y, z);
}

Scene createSceneFromTokens(std::vector<Token> tokens, Mesh& defaultMesh, std::map<std::string, Mesh> meshes){
  Scene scene;

  short id = 0;
  for (Token tok : tokens){
    std::string objectName = tok.target;

    if (!(scene.nameToId.find(objectName) != scene.nameToId.end())){
      addObjectToScene(scene, glm::vec3(1.0f, 1.0f, 1.0f), defaultMesh, objectName, &id, false);
    }
    if (tok.attribute == "mesh"){
      if(meshes.find(tok.payload) == meshes.end()){
        std::cerr << "ERROR: failed loading mesh: " << tok.payload << std::endl;
        continue;
      }
      scene.idToGameObjects[scene.nameToId[objectName]].mesh = meshes[tok.payload];
    }else if (tok.attribute == "position"){
      scene.idToGameObjects[scene.nameToId[objectName]].position = parseVec(tok.payload);
    }else if (tok.attribute == "scale"){
      scene.idToGameObjects[scene.nameToId[objectName]].scale = parseVec(tok.payload);
    }else if (tok.attribute == "parent"){
      // @todo add back in when scenes have parent representation
      //if (!(scene.nameToId.find(tok.payload) != scene.nameToId.end())){
      //  addObjectToScene(scene, glm::vec3(1.0f, 1.0f, 1.0f), defaultMesh, tok.payload, &id, false);
      //}
    }
  }
  return scene;
}

// @todo this parsing is sloppy and buggy... obviously need to harden this..
Scene deserializeScene(std::string content, Mesh& defaultMesh, std::map<std::string, Mesh> meshes){
  std::cout << "INFO: Deserialization: " << std::endl;

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

  return createSceneFromTokens(dtokens, defaultMesh, meshes);
}


