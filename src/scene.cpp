#include "./scene.h"

Scene loadScene(Mesh& columnSeatMesh, Mesh& boxMesh, Mesh& grassMesh){
  short id = 0;
  Scene scene;
  GameObject columnSeatObject = {
    .id = id,
    .position = glm::vec3(0.0f, 0.0f, 0.0f),
    .scale = glm::vec3(1.0f, 1.0f, 1.0f),
    .mesh = columnSeatMesh,
  };
  id++;
  scene.gameObjects.push_back(columnSeatObject);
  for (unsigned int i = 0;  i < 10; i++){
    GameObject boxObject = {
      .id = id,
      .position =  glm::vec3(6.0f + (i * 3.0f), 0.0f, 0.0f),
      .scale = glm::vec3(1.0f, 1.0f, 1.0f),
      .mesh = boxMesh,
    };
    id++;
    scene.gameObjects.push_back(boxObject);
  }
  GameObject grassObject = {
    .id = id,
  	.position = glm::vec3(0.0, 1.0f, 0.0f),
    .scale = glm::vec3(1.0f, 1.0f, 1.0f),
  	.mesh = grassMesh,
  };
  id++;
  scene.rotatingGameObjects.push_back(grassObject);
  return scene;
}