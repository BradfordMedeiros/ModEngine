#include "./meshgen.h"

struct MeshGenPoints { 
  std::vector<glm::vec3> points; 
};
typedef std::variant<MeshGenPoints> MeshgenInterpolator;

struct MeshgenConfig {
  std::vector<Vertex> face;
  MeshgenInterpolator interpolator;
  std::string diffuseTexturePath;
};

Vertex createVertex(glm::vec3 position, glm::vec2 texCoords){
  Vertex vertex {
    .position = position,
    .normal = glm::vec3(0.f, 0.f, 0.f),
    .texCoords = texCoords,
  };
  for (int i = 0; i < NUM_BONES_PER_VERTEX; i++){
    vertex.boneIndexes[i] = 0;
    vertex.boneWeights[i] = 0;
  }
  return vertex;
}

struct MeshInterpolated {
  glm::vec3 position;
  glm::quat rotation;
};

std::vector<MeshInterpolated> interpolatedPositions(MeshgenInterpolator& interpolator){
  std::vector<MeshInterpolated> points;
  auto pointInterpolater = std::get_if<MeshGenPoints>(&interpolator);
  if (pointInterpolater != NULL){
    for (auto point : pointInterpolater -> points){
      points.push_back(MeshInterpolated{
        .position = point,
        .rotation = glm::identity<glm::quat>(),
      });
    }
    for (int i = 0; i < points.size() - 1; i++){
      auto fromPos = points.at(i).position;
      auto targetPosition = points.at(i + 1).position;
      points.at(i).rotation = orientationFromPos(fromPos, targetPosition);
    }
    if (points.size() >= 2){
      points.at(points.size() - 1).rotation = points.at(points.size() - 2).rotation;
    }
  }else{
    std::cout << "invalid points interpolator" << std::endl;
    assert(false);
  }
  return points;
}



void add2DCrossSection(std::vector<Vertex>& _vertices, glm::mat4 transform, std::vector<Vertex>& face){
  assert(face.size() % 3 == 0);
  for (auto vertex : face){
    _vertices.push_back(createVertex(
      transform * glm::vec4(vertex.position.x, vertex.position.y, vertex.position.z, 1.f),
      vertex.texCoords
    ));
  }
}

void connectFace(std::vector<Vertex>& _vertices, Vertex& fromLeftSide, Vertex& toLeftSide, Vertex& fromRightSide, Vertex& toRightSide){
   _vertices.push_back(createVertex(
      fromLeftSide.position,
      glm::vec2(0.f, 0.f)
    ));
   _vertices.push_back(createVertex(
      toLeftSide.position,
      glm::vec2(1.f, 0.f)
    ));
   _vertices.push_back(createVertex(
      toRightSide.position,
      glm::vec2(1.f, 1.f)
    ));
   _vertices.push_back(createVertex(
      fromLeftSide.position,
      glm::vec2(1.f, 1.f)
    ));
   _vertices.push_back(createVertex(
      toRightSide.position,
      glm::vec2(0.f, 1.f)
    ));
   _vertices.push_back(createVertex(
      fromRightSide.position,
      glm::vec2(0.f, 0.f)
    ));
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

MeshData generateMesh(std::vector<glm::vec3>& points){
  MeshgenConfig config {
    .face = {
      createVertex(
        glm::vec4(0.f, 0.f, 0.f, 1.f),
        glm::vec2(0.f, 0.f)
      ),
      createVertex(
        glm::vec4(1.f, 0.f, 0.f, 1.f),
        glm::vec2(1.f, 0.f)
      ),
      createVertex(
        glm::vec4(0.5f, 1.f, 0.f, 1.f),
        glm::vec2(1.f, 1.f)
      ),
      createVertex(
        glm::vec4(0.f, 0.f, 0.f, 1.f),
        glm::vec2(1.f, 1.f)
      ),
      createVertex(
        glm::vec4(1.f, 0.f, 0.f, 1.f),
        glm::vec2(0.f, 1.f)
      ),
      createVertex(
        glm::vec4(0.5f, -1.f, 0.f, 1.f),
        glm::vec2(0.f, 0.f)
      )
    },
    .interpolator = MeshGenPoints {
      .points = points,
    },
    .diffuseTexturePath = "./res/textures/wood.jpg",
  };

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  for (auto pos : interpolatedPositions(config.interpolator)){
    // maybe should verify cross section points are actually coplanar?
    add2DCrossSection(vertices, createRotation(pos.position, pos.rotation), config.face);
  }
  connect2DCrossSections(vertices, config.face.size() / 3);
  for (int i = 0; i < vertices.size(); i++){
    indices.push_back(i);
  }
  MeshData meshdata {
    .vertices = vertices,
    .indices = indices,
    .diffuseTexturePath = config.diffuseTexturePath,
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

