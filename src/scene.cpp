#include "./scene.h"

Scene loadScene(Mesh& columnSeatMesh, Mesh& boxMesh, Mesh& grassMesh){
  Scene scene;
  GameObject columnSeatObject = {
    .position = glm::vec3(0.0f, 0.0f, 0.0f),
    .mesh = columnSeatMesh,
  };
  scene.gameObjects.push_back(columnSeatObject);
  for (unsigned int i = 0;  i < 10; i++){
    GameObject boxObject = {
      .position =  glm::vec3(6.0f + (i * 3.0f), 0.0f, 0.0f),
      .mesh = boxMesh,
    };
    scene.gameObjects.push_back(boxObject);
  }
  GameObject grassObject = {
  	.position = glm::vec3(0.0, 1.0f, 0.0f),
  	.mesh = grassMesh,
  };
  scene.rotatingGameObjects.push_back(grassObject);
  return scene;
}