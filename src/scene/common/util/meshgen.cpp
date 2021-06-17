#include "./meshgen.h"

struct MeshInterpolated {
  glm::vec3 position;
  glm::quat rotation;
};

std::vector<MeshInterpolated> interpolatedPositions(){
  std::vector<MeshInterpolated> points;
  points.push_back(MeshInterpolated{
    .position = glm::vec3(0.f, 0.f, 0.f),
    .rotation = glm::identity<glm::quat>(),
  });
  points.push_back(MeshInterpolated{
    .position = glm::vec3(0.f, 0.f, 5.f),
    .rotation = glm::identity<glm::quat>(),
  });
  points.push_back(MeshInterpolated{
    .position = glm::vec3(1.f, 0.f, 7.f),
    .rotation = glm::identity<glm::quat>(),
  });
  return points;
}

Vertex createVertex(glm::vec4 position){
  Vertex vertex {
    .position = position,
    .normal = {},
    .texCoords = {},
  };
  for (int i = 0; i < NUM_BONES_PER_VERTEX; i++){
    vertex.boneIndexes[i] = 0;
    vertex.boneWeights[i] = 0;
  }
  return vertex;
}

void add2DCrossSection(std::vector<Vertex>& vertices, glm::mat4 transform){
  vertices.push_back(createVertex(transform * glm::vec4(0.f, 0.f, 0.f, 1.f)));
  vertices.push_back(createVertex(transform * glm::vec4(1.f, 0.f, 0.f, 1.f)));
  vertices.push_back(createVertex(transform * glm::vec4(0.5f, 1.f, 0.f, 1.f)));
  vertices.push_back(createVertex(transform * glm::vec4(0.f, 0.f, 0.f, 1.f)));
  vertices.push_back(createVertex(transform * glm::vec4(1.f, 0.f, 0.f, 1.f)));
  vertices.push_back(createVertex(transform * glm::vec4(0.5f, -1.f, 0.f, 1.f)));
}

void connectFace(std::vector<Vertex>& _vertices, Vertex& fromLeftSide, Vertex& toLeftSide, Vertex& fromRightSide, Vertex& toRightSide){
   _vertices.push_back(fromLeftSide);
   _vertices.push_back(toLeftSide);
   _vertices.push_back(toRightSide);
   _vertices.push_back(fromLeftSide);
   _vertices.push_back(toRightSide);
   _vertices.push_back(fromRightSide);
}

// TODO -> should remove inner faces.  Need to think about how (probably some existing algorithm?)
void connect2DCrossSections(std::vector<Vertex>& vertices, int faceTriangleCount){
  auto trianglesPerFace = 3 * faceTriangleCount;
  auto numFaces = vertices.size() / trianglesPerFace;

  std::vector<Vertex> verticesToAdd;
  for (int face = 0; face < numFaces - 1; face++){
    auto fromFaceOffset = face * trianglesPerFace;
    auto toFaceOffset = (face + 1) * trianglesPerFace;
    for (int triangle = 0; triangle < faceTriangleCount; triangle++){
      auto triangleOffset = triangle * 3;
      auto fromVertex1 = vertices.at(fromFaceOffset + triangleOffset);
      auto toVertex1 = vertices.at(toFaceOffset + triangleOffset);
      auto fromVertex2 = vertices.at(fromFaceOffset + triangleOffset + 1);
      auto toVertex2 = vertices.at(toFaceOffset + triangleOffset + 1);
      auto fromVertex3 = vertices.at(fromFaceOffset + triangleOffset + 2);
      auto toVertex3 = vertices.at(toFaceOffset + triangleOffset + 2);
      connectFace(verticesToAdd, fromVertex1, toVertex1, fromVertex2, toVertex2);
      connectFace(verticesToAdd, fromVertex1, toVertex1, fromVertex3, toVertex3);
      connectFace(verticesToAdd, fromVertex2, toVertex2, fromVertex3, toVertex3);
    }
  }
  for (auto vertex : verticesToAdd){
    vertices.push_back(vertex);
  }
}

glm::mat4 createRotation(glm::vec3 position, glm::quat rotation){
  return glm::translate(glm::mat4(1.f), position) * glm::toMat4(rotation);
}

MeshData generateMesh(){
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  for (auto pos : interpolatedPositions()){
    add2DCrossSection(vertices, createRotation(pos.position, pos.rotation));
  }
  connect2DCrossSections(vertices, 2);
  
  for (int i = 0; i < vertices.size(); i++){
    indices.push_back(i);
  }

  MeshData meshdata {
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
  return meshdata;
}

