#include "./octree_mesh.h"

extern std::optional<AtlasDimensions> atlasDimensions;

glm::vec2 calcNdiCoordAtlasCoord(glm::vec2 ndiCoord, int imageIndex){
  auto textureDim = calculateAtlasImageDimension(atlasDimensions.value().textureNames.size());

  float atlasWide = 1.f / textureDim;         // width  per texture
  float atlasHeight = 1.f / textureDim;       // height per texture

  int xIndex = imageIndex % textureDim;
  int yIndex = imageIndex / textureDim;

  float xValue = atlasWide * xIndex + (ndiCoord.x * atlasWide);
  float yValue = atlasHeight * yIndex + (ndiCoord.y * atlasHeight);
  return glm::vec2(xValue, yValue);
}

glm::vec2 meshTexBottomRight(FaceTexture& faceTexture){
  return calcNdiCoordAtlasCoord(faceTexture.texCoordsBottomRight, faceTexture.textureIndex);
}
glm::vec2 meshTexBottomLeft(FaceTexture& faceTexture){
  return calcNdiCoordAtlasCoord(faceTexture.texCoordsBottomLeft, faceTexture.textureIndex);
}
glm::vec2 meshTexTopLeft(FaceTexture& faceTexture){
  return calcNdiCoordAtlasCoord(faceTexture.texCoordsTopLeft, faceTexture.textureIndex);
}
glm::vec2 meshTexTopRight(FaceTexture& faceTexture){
  return calcNdiCoordAtlasCoord(faceTexture.texCoordsTopRight, faceTexture.textureIndex);
}

void addCubePointsFront(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height){
  modassert(faces -> size() == 6, std::string("faces size unexpected, got: " + std::to_string(faces -> size())));
  FaceTexture& frontFace =  faces -> at(0);
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset, .coord = meshTexBottomRight(frontFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + offset, .coord = meshTexTopLeft(frontFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,  .coord = meshTexBottomLeft(frontFace) });

  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + offset, .coord = meshTexTopRight(frontFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + offset,  .coord = meshTexTopLeft(frontFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,  .coord = meshTexBottomRight(frontFace)  });
}
void addCubePointsBack(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height, float depth){
  modassert(faces -> size() == 6, std::string("faces size unexpected, got: " + std::to_string(faces -> size())));
  FaceTexture& backFace =  faces -> at(1);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size * depth) + offset,  .coord = meshTexBottomRight(backFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size * depth) + offset, .coord = meshTexTopRight(backFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size * depth) + offset, .coord = meshTexBottomLeft(backFace) });

  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size * depth) + offset,  .coord = meshTexBottomLeft(backFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size * depth) + offset,  .coord = meshTexTopRight(backFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size * depth) + offset, .coord = meshTexTopLeft(backFace) });
}
void addCubePointsLeft(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height){
  modassert(faces -> size() == 6, std::string("faces size unexpected, got: " + std::to_string(faces -> size())));
  FaceTexture& leftFace =  faces -> at(2);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + offset, .coord = meshTexTopLeft(leftFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + offset,  .coord = meshTexBottomLeft(leftFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,    .coord = meshTexBottomRight(leftFace) });

  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + offset,   .coord = meshTexTopRight(leftFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + offset, .coord = meshTexTopLeft(leftFace)  });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,    .coord = meshTexBottomRight(leftFace) });
}
void addCubePointsRight(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height){
  modassert(faces -> size() == 6, std::string("faces size unexpected, got: " + std::to_string(faces -> size())));
  FaceTexture& rightFace =  faces -> at(3);
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,    .coord = meshTexBottomLeft(rightFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + offset,  .coord = meshTexBottomRight(rightFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size) + offset, .coord = meshTexTopRight(rightFace) });

  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,    .coord = meshTexBottomLeft(rightFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size) + offset, .coord = meshTexTopRight(rightFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + offset,   .coord = meshTexTopLeft(rightFace) });
}
void addCubePointsTop(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height){
  modassert(faces -> size() == 6, std::string("faces size unexpected, got: " + std::to_string(faces -> size())));
  FaceTexture& topFace =  faces -> at(4);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + offset,   .coord = meshTexBottomLeft(topFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + offset,  .coord = meshTexBottomRight(topFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + offset, .coord = meshTexTopLeft(topFace) });

  points.push_back(OctreeVertex { .position = glm::vec3(0.f,  size * height, -size) + offset,  .coord = meshTexTopLeft(topFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + offset,   .coord = meshTexBottomRight(topFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size) + offset, .coord = meshTexTopRight(topFace) });
}
void addCubePointsBottom(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float depth, float width){
  modassert(faces -> size() == 6, std::string("faces size unexpected, got: " + std::to_string(faces -> size())));
  FaceTexture& bottomFace =  faces -> at(5);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size * depth) + offset, .coord = meshTexBottomLeft(bottomFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(width * size, 0.f, 0.f) + offset,  .coord = meshTexTopRight(bottomFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,   .coord = meshTexTopLeft(bottomFace) });

  points.push_back(OctreeVertex { .position = glm::vec3(width * size, 0.f, -size * depth) + offset, .coord = meshTexBottomRight(bottomFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(width * size, 0.f, 0.f) + offset,   .coord = meshTexTopRight(bottomFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size * depth) + offset,  .coord = meshTexBottomLeft(bottomFace) });
}