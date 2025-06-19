#include "./octree_types.h"

glm::ivec3 flatIndexToXYZ(int index){
  modassert(index >= 0 && index < 8, "invalid flatIndexToXYZ");

  if (index == 0){
    return glm::ivec3(0, 1, 1);
  }
  if (index == 1){
    return glm::ivec3(1, 1, 1);
  }
  if (index == 2){
    return glm::ivec3(0, 1, 0);
  }
  if (index == 3){
    return glm::ivec3(1, 1, 0);
  }
  if (index == 4){
    return glm::ivec3(0, 0, 1);
  }
  if (index == 5){
    return glm::ivec3(1, 0, 1);
  }
  if (index == 6){
    return glm::ivec3(0, 0, 0);
  }
  if (index == 7){
    return glm::ivec3(1, 0, 0);
  }
  modassert(false, "invalid flatIndexToXYZ error");
  return glm::ivec3(0, 0, 0);
}

int xyzIndexToFlatIndex(glm::ivec3 index){
  modassert(index.x >= 0 && index.x < 2, std::string("xyzIndexToFlatIndex: invalid x index: ") + print(index));
  modassert(index.y >= 0 && index.y < 2, std::string("xyzIndexToFlatIndex: invalid y index: ") + print(index));
  modassert(index.z >= 0 && index.z < 2, std::string("xyzIndexToFlatIndex: invalid z index: ") + print(index));

  // -x +y -z 
  if (index.x == 0 && index.y == 1 && index.z == 1){
    return 0;
  }

  // +x +y -z
  if (index.x == 1 && index.y == 1 && index.z == 1){
    return 1;
  }

  // -x +y +z
  if (index.x == 0 && index.y == 1 && index.z == 0){
    return 2;
  }

  // +x +y +z
  if (index.x == 1 && index.y == 1 && index.z == 0){
    return 3;
  }

  // -x -y -z 
  if (index.x == 0 && index.y == 0 && index.z == 1){
    return 4;
  }

  // +x -y -z
  if (index.x == 1 && index.y == 0 && index.z == 1){
    return 5;
  }

  // -x -y +z
  if (index.x == 0 && index.y == 0 && index.z == 0){
    return 6;
  }

  // +x -y +z
  if (index.x == 1 && index.y == 0 && index.z == 0){
    return 7;
  }

  modassert(false, "xyzIndexToFlatIndex invalid");
  return 0;
}

glm::vec3 offsetForFlatIndex(int index, float subdivisionSize, glm::vec3 rootPos){
  if (index == 0){
    return rootPos + glm::vec3(0.f, subdivisionSize, -subdivisionSize);
  }else if (index == 1){
    return rootPos + glm::vec3(subdivisionSize, subdivisionSize, -subdivisionSize);
  }else if (index == 2){
    return rootPos + glm::vec3(0.f, subdivisionSize, 0.f);
  }else if (index == 3){
    return rootPos + glm::vec3(subdivisionSize, subdivisionSize, 0.f);
  }else if (index == 4){
    return rootPos + glm::vec3(0.f, 0.f, -subdivisionSize);
  }else if (index == 5){
    return rootPos + glm::vec3(subdivisionSize, 0.f, -subdivisionSize);
  }else if (index == 6){
    return rootPos + glm::vec3(0.f, 0.f, 0.f);
  }else if (index == 7){
    return rootPos + glm::vec3(subdivisionSize, 0.f, 0.f);
  }
  modassert(false, "invalid flat index");
  return glm::vec3(0.f, 0.f, 0.f);
}

