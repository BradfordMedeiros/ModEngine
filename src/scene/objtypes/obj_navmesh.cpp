#include "./obj_navmesh.h"

std::vector<AutoSerialize> navmeshAutoserializer {
};



std::optional<objid> targetNavmeshId(glm::vec3 target, raycastFn raycast, std::function<bool(objid)> isNavmesh){
  auto abitAbove = glm::vec3(target.x, target.y + 1, target.z);
  auto direction = orientationFromPos(abitAbove, target);
  auto hitObjects = raycast(abitAbove, direction, 5);
  for (auto targetObject : hitObjects){
    if (isNavmesh(targetObject.id)){
      return targetObject.id;
    }
  }
  return std::nullopt;
}

glm::vec3 aiNavPosition(objid id, glm::vec3 target, std::function<glm::vec3(objid)> position, raycastFn raycast, std::function<bool(objid)> isNavmesh){
  auto objectPosition = position(id);
  auto directionTowardPoint = orientationFromPos(objectPosition, target);
  return moveRelative(objectPosition, directionTowardPoint, glm::vec3(0.f, 0.f, -0.5f), false);



Mesh createNavmeshFromPointList(std::vector<glm::vec3> points, ObjectTypeUtil& util){
  std::vector<Vertex> vertices;
  for (int i = 0; i < points.size(); i++){
    int triangleIndex = i % 3;
    if (triangleIndex == 0){
      vertices.push_back(createVertex(points.at(i), glm::vec2(0.f, 0.f)));  // maybe the tex coords should just be calculated as a ratio to a fix texture
      continue;
    }else if (triangleIndex == 1){
      vertices.push_back(createVertex(points.at(i), glm::vec2(1.f, 0.f)));
      continue;
    }else if (triangleIndex == 2){
      vertices.push_back(createVertex(points.at(i), glm::vec2(0.f, 1.f)));
      continue;
    }
    modassert(false, "invalid triangleIndex");
  }
  std::vector<unsigned int> indices;
  for (int i = 0; i < vertices.size(); i++){ // should be able to optimize this better...
    indices.push_back(i);
  }
  MeshData meshData {
    .vertices = vertices,
    .indices = indices,
    .diffuseTexturePath = "./res/textures/wood.jpg",
    .hasDiffuseTexture = true,
    .emissionTexturePath = "",
    .hasEmissionTexture = false,
    .opacityTexturePath = "",
    .hasOpacityTexture = false,
    .boundInfo = getBounds(vertices),
    .bones = {},
  };
  return util.loadMesh(meshData);
}

std::vector<glm::vec3> points = {
  glm::vec3(0.f, 0.f, 0.f),
  glm::vec3(2.f, 0.f, 0.f),
  glm::vec3(0.f, 0.f, -2.f),


  glm::vec3(0.f, 0.f, -2.f),
  glm::vec3(2.f, 0.f, 0.f),
  glm::vec3(2.f, 0.f, -2.f),


  glm::vec3(2.f, 0.f, -2.f),
  glm::vec3(4.f, 0.f, -2.f),
  glm::vec3(8.f, 0.f, -4.f),


};

GameObjectNavmesh createNavmesh(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectNavmesh obj {
    .mesh = createNavmeshFromPointList(points, util),
  };
  return obj;
}