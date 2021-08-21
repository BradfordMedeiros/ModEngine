#include "./types.h"

Transformation getTransformationFromMatrix(glm::mat4 matrix){
  glm::vec3 scale;
  glm::quat rotation;
  glm::vec3 translation;
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(matrix, scale, rotation, translation, skew, perspective);
  Transformation transform = {
    .position = translation,
    .scale = scale,
    .rotation = rotation,
  };
  return transform;  
}

void printMatrixInformation(glm::mat4 transform, std::string label){
  auto initPose = getTransformationFromMatrix(transform);
  std::cout << label <<  " posn: " << print(initPose.position) << std::endl;
  std::cout << label << " scale: " << print(initPose.scale) << std::endl;
  std::cout << label << " rot: " << print(initPose.rotation) << std::endl;
}

BoundInfo getBounds(std::vector<Vertex>& vertices){
  float xMin, xMax;
  float yMin, yMax;
  float zMin, zMax;

  xMin = vertices[0].position.x;    // @todo zero vertex model --> which is fucking stupid but correct, will error here.
  xMax = vertices[0].position.x;
  yMin = vertices[0].position.y;
  yMax = vertices[0].position.y;
  zMin = vertices[0].position.z;
  zMax = vertices[0].position.z;

  for (Vertex vert: vertices){
    if (vert.position.x > xMax){
      xMax = vert.position.x;
    }
    if (vert.position.x < xMin){
      xMin = vert.position.x;
    }
    if (vert.position.y > yMax){
      yMax = vert.position.y;
    }
    if (vert.position.y < yMin){
      yMin = vert.position.y;
    }
    if (vert.position.z > zMax){
      zMax = vert.position.z;
    }
    if (vert.position.z < zMin){
      zMin = vert.position.z;
    }
  }

  BoundInfo info = {
    .xMin = xMin, .xMax = xMax,
    .yMin = yMin, .yMax = yMax,
    .zMin = zMin, .zMax = zMax,
  };
  return info;
}