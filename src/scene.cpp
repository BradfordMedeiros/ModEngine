#include "./scene.h"

GameObject getGameObject(glm::vec3 position, Mesh& mesh, short id){
  GameObject gameObject = {
    .id = id,
    .position = position,
    .scale = glm::vec3(1.0f, 1.0f, 1.0f),
    .mesh = mesh, 
  };
  return gameObject;
}

void addObjectToScene(Scene& scene, glm::vec3 position, Mesh& mesh, short* id){
  auto gameobjectObj = getGameObject(position, mesh, *id);
  *id = *id + 1;

  auto gameobjectH = GameObjectH {
    .id = gameobjectObj.id,
  };
  
  scene.gameObjects.push_back(gameobjectH);
  scene.idToGameObjects[gameobjectObj.id] = gameobjectObj;
}

Scene loadScene(Mesh& columnSeatMesh, Mesh& boxMesh, Mesh& grassMesh){
  short id = 0;
  Scene scene;

  addObjectToScene(scene, glm::vec3(0.0f, 0.0f, 0.0f), columnSeatMesh, &id);
  for (unsigned int i = 0;  i < 10; i++){
    addObjectToScene(scene, glm::vec3(6.0f + (i * 3.0f), 0.0f, 0.0f), boxMesh, &id);
  }
  addObjectToScene(scene, glm::vec3(0.0, 1.0f, 0.0f), grassMesh, &id);
  return scene;
}

std::string serializeScene(Scene& scene){
  
}

Scene deserializeScene(std::string content){
   std::cout << "INFO: Deserialization: " << std::endl;
   std::cout << "INFO: Deserialization: content is " << content << std::endl;
   Scene scene;
   return scene;
}


