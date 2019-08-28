#include "./scene.h"

GameObject getGameObject(glm::vec3 position, Mesh& mesh, std::string meshName, std::string name, short id, bool isRotating){
  GameObject gameObject = {
    .id = id,
    .name = name,
    .position = position,
    .scale = glm::vec3(1.0f, 1.0f, 1.0f),
    .rotation = glm::quat(0, 0, 0, 1.0f),
    .mesh = mesh, 
    .meshName = meshName,
    .isRotating = isRotating,
  };
  return gameObject;
}

void addObjectToScene(Scene& scene, glm::vec3 position, Mesh& mesh, std::string name, short* id, bool isRotating, short parentId, std::function<void(short, std::string)> addObject){
  auto gameobjectObj = getGameObject(position, mesh, "", name, *id, isRotating);
  *id = *id + 1;

  auto gameobjectH = GameObjectH {
    .id = gameobjectObj.id,
    .parentId = parentId,
  };

  scene.idToGameObjectsH[gameobjectObj.id] = gameobjectH;
  scene.idToGameObjects[gameobjectObj.id] = gameobjectObj;
  scene.nameToId[name] = gameobjectObj.id;
  addObject(gameobjectObj.id, "mesh");
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

Scene createSceneFromTokens(std::vector<Token> tokens, Mesh& defaultMesh, std::map<std::string, Mesh> meshes, std::function<void(short, std::string)> addObject){
  Scene scene;

  short id = 0;
  for (Token tok : tokens){
    std::string objectName = tok.target;

    if (!(scene.nameToId.find(objectName) != scene.nameToId.end())){
      addObjectToScene(scene, glm::vec3(1.0f, 1.0f, 1.0f), defaultMesh, objectName, &id, false, -1, addObject);
    }
    if (tok.attribute == "mesh"){
      if(meshes.find(tok.payload) == meshes.end()){
        std::cerr << "ERROR: failed loading mesh: " << tok.payload << std::endl;
        continue;
      }
      scene.idToGameObjects[scene.nameToId[objectName]].mesh = meshes[tok.payload];
      scene.idToGameObjects[scene.nameToId[objectName]].meshName = tok.payload; 
    }else if (tok.attribute == "position"){
      scene.idToGameObjects[scene.nameToId[objectName]].position = parseVec(tok.payload);
    }else if (tok.attribute == "scale"){
      scene.idToGameObjects[scene.nameToId[objectName]].scale = parseVec(tok.payload);
    }else if (tok.attribute == "rotation"){
      glm::vec3 eulerAngles = parseVec(tok.payload);
      glm::quat rotation = glm::quat(glm::vec3(eulerAngles.x + 0, eulerAngles.y + 0, (eulerAngles.z + M_PI)));
      scene.idToGameObjects[scene.nameToId[objectName]].rotation = rotation;
    }else if (tok.attribute == "parent"){
      if (!(scene.nameToId.find(tok.payload) != scene.nameToId.end())){
        addObjectToScene(scene, glm::vec3(1.0f, 1.0f, 1.0f), defaultMesh, tok.payload, &id, false, -1, addObject);
      }
      scene.idToGameObjectsH[scene.nameToId[objectName]].parentId = scene.nameToId[tok.payload];
      scene.idToGameObjectsH[scene.nameToId[tok.payload]].children.insert(scene.idToGameObjectsH[scene.nameToId[objectName]].id);
    }
  }

  for( auto const& [id, gameobjectH] : scene.idToGameObjectsH ){
    if (gameobjectH.parentId == -1){
      scene.rootGameObjectsH.push_back(gameobjectH.id);
    }
  }
  return scene;
}

// @todo this parsing is sloppy and buggy... obviously need to harden this..
Scene deserializeScene(std::string content, Mesh& defaultMesh, std::map<std::string, Mesh> meshes, std::function<void(short, std::string)> addObject){
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

  return createSceneFromTokens(dtokens, defaultMesh, meshes, addObject);
}

std::string serializeVec(glm::vec3 vec){
  return std::to_string(vec.x) + " " + std::to_string(vec.y) + " " + std::to_string(vec.z);
}
std::string serializeRotation(glm::quat rotation){
  float xx = rotation.x;
  float yy = rotation.y;
  float zz = rotation.z;
  glm::vec3 angles = eulerAngles(rotation);
  return std::to_string(angles.x) + " " + std::to_string(angles.y) + " " + std::to_string(angles.z - M_PI); 
}
std::string serializeScene(Scene& scene){
  std::string sceneData = "# Generated scene \n";
  for (auto [id, gameobjecth]: scene.idToGameObjectsH){
    GameObject gameobject = scene.idToGameObjects[id];
    std::string gameobjectName = gameobject.name;
    std::string parentName = scene.idToGameObjects[gameobjecth.parentId].name;

    sceneData = sceneData + gameobjectName + ":position:" + serializeVec(gameobject.position) + "\n";
    sceneData = sceneData + gameobjectName + ":scale:" + serializeVec(gameobject.scale) + "\n";
    sceneData = sceneData + gameobjectName + ":rotation:" + serializeRotation(gameobject.rotation) + "\n";

    if (gameobject.meshName != ""){
      sceneData = sceneData + gameobjectName + ":mesh:" + gameobject.meshName + "\n";
    }
    if (parentName != ""){
      sceneData =  sceneData + gameobjectName + ":parent:" + parentName + "\n";
    }
  }
  return sceneData;
}
