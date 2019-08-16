#include "./scene.h"

GameObject getGameObject(glm::vec3 position, Mesh& mesh, short* id){
  GameObject gameObject = {
    .id = *id,
    .position = position,
    .scale = glm::vec3(1.0f, 1.0f, 1.0f),
    .mesh = mesh, 
  };
  *id = *id + 1;
  return gameObject;
}

Scene loadScene(Mesh& columnSeatMesh, Mesh& boxMesh, Mesh& grassMesh){
  short id = 0;
  Scene scene;
  scene.gameObjects.push_back(getGameObject(glm::vec3(0.0f, 0.0f, 0.0f), columnSeatMesh, &id));
  for (unsigned int i = 0;  i < 10; i++){
    scene.gameObjects.push_back(getGameObject(glm::vec3(6.0f + (i * 3.0f), 0.0f, 0.0f), boxMesh, &id));
  }
  scene.rotatingGameObjects.push_back(getGameObject(glm::vec3(0.0, 1.0f, 0.0f), grassMesh, &id));
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


