#include "./types.h"

glm::vec3 distanceToSecondFromFirst(glm::mat4 y, glm::mat4 x){
  auto toRelativeToFrom = y * x;
  return getTransformationFromMatrix(toRelativeToFrom).position;
}

glm::mat4 calculateScaledMatrix(glm::mat4 view, glm::mat4 modelMatrix, float fov){
  auto transform = getTransformationFromMatrix(modelMatrix);
  auto offset = distanceToSecondFromFirst(view, modelMatrix);
  transform.scale *=  glm::tan(fov / 2.0) * offset.z; // glm::tan might not be correct
  auto mat = matrixFromComponents(transform);
  return mat;
}

void printTransformInformation(Transformation transform){
  std::cout << "x.pos = [ " << print(transform.position)  << " ]; ";
  std::cout << "x.scale = [ " << print(transform.scale) << " ]; ";
  std::cout << "x.rot = [ " << print(transform.rotation) << " ]; " << std::endl; 
}

void printMatrixInformation(glm::mat4 transform, std::string label){
  auto initPose = getTransformationFromMatrix(transform);
  std::cout << label;

  printTransformInformation(initPose);

  std::cout << label << " = [";
  for (int i = 0; i < 4; i++){
    for (int j = 0; j < 4; j++){
      std::cout << transform[j][i] << " ";
    }
    if (i != 3){
      std::cout << "; ";
    }
  }  
  std::cout << "]" << std::endl;
}

BoundInfo getBounds(std::vector<Vertex>& vertices){
  float xMin, xMax;
  float yMin, yMax;
  float zMin, zMax;

  xMin = vertices.at(0).position.x;    // @todo zero vertex model --> which is fucking stupid but correct, will error here.
  xMax = vertices.at(0).position.x;
  yMin = vertices.at(0).position.y;
  yMax = vertices.at(0).position.y;
  zMin = vertices.at(0).position.z;
  zMax = vertices.at(0).position.z;

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

Transformation interpolate(Transformation transform1, Transformation transform2, float posamount, float scaleamount, float rotamount){
  return Transformation {
    .position = glm::lerp(transform1.position, transform2.position, posamount),
    .scale = glm::lerp(transform1.scale, transform2.scale, scaleamount),
    .rotation = glm::slerp(transform1.rotation,  transform2.rotation, rotamount),
  };
}

std::string print(Vertex& vertex){
  return std::string("[ pos =  ") + print(vertex.position) + ", normal = " + print(vertex.normal) + ", texCoords = " + print(vertex.texCoords) + "]";
}

void compareEqualVertices(std::vector<glm::vec3>& verts1, std::vector<glm::vec3>& verts2){
  modassert(verts1.size() == verts2.size(), "vertices not same size, size1 = " + std::to_string(verts1.size()) + ", size2 = " + std::to_string(verts2.size()));
  for (int i = 0; i < verts1.size(); i++){
    if (!aboutEqual(verts1.at(i), verts2.at(i))){
      modassert(false, "vertices not equal");
    }
  }
}
