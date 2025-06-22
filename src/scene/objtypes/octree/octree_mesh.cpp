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

struct OctreeVertex {
  glm::vec3 position;
  glm::vec2 coord;
};

void addCubePointsFront(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f);
void addCubePointsBack(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f, float depth = 1.f);
void addCubePointsLeft(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f);
void addCubePointsRight(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f);
void addCubePointsTop(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f);
void addCubePointsBottom(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float depth = 1.f, float width = 1.f);

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

// does not account if a larger subdivision exists and is filled
OctreeDivision* getOctreeSubdivisionIfExists2(Octree& octree, int x, int y, int z, int subdivision){
  if (x < 0 || y < 0 || z < 0){
    return NULL;
  }
  auto biggestSubdivisionSize = glm::pow(2, subdivision);
  if (x >= biggestSubdivisionSize || y >= biggestSubdivisionSize || z >= biggestSubdivisionSize){
    return NULL;
  }
  auto path = octreePath(x, y, z, subdivision);
  OctreeDivision* octreeSubdivision = &octree.rootNode;
  for (int i = 0; i < path.size(); i++){
    int index = xyzIndexToFlatIndex(path.at(i));
    if (octreeSubdivision -> fill == FILL_EMPTY){
      return NULL;
    }else if (octreeSubdivision -> fill == FILL_FULL){
      return octreeSubdivision;
    }
    modassert(octreeSubdivision -> divisions.size() == 8, "expected 8 subdivisions");
    octreeSubdivision = &(octreeSubdivision -> divisions.at(index));
  }
  return octreeSubdivision;
}

struct FillStatus {
  FillType fill;
  std::optional<OctreeDivision*> mixed;
};

FillStatus octreeFillStatus(Octree& octree, int subdivisionLevel, glm::ivec3 division){
  // this should be looking at the target subdivsiion level, and any level before it 

  auto octreeDivision = getOctreeSubdivisionIfExists2(octree, division.x, division.y, division.z, subdivisionLevel);
  if (!octreeDivision){
    return FillStatus { .fill = FILL_EMPTY, .mixed = std::nullopt };
  }

  auto blockShape = std::get_if<ShapeBlock>(&octreeDivision -> shape);  // depeneding on the side of this, we could hide more faces
  if (blockShape == NULL){
    return FillStatus { .fill = FILL_EMPTY, .mixed = std::nullopt };
  }

  // if this is mixed, then we need to check the corresponding side and see if it's filled 
  if (octreeDivision -> fill == FILL_MIXED){
    return FillStatus { .fill = FILL_MIXED, .mixed = octreeDivision };
  }
  return FillStatus { .fill = octreeDivision -> fill, .mixed = std::nullopt };
}

  // -x +y -z 
  // +x +y -z
  // -x +y +z
  // +x +y +z
  // -x -y -z 
  // +x -y -z
  // -x -y +z
  // +x -y +z

bool isSideFull(OctreeDivision& division, std::vector<int>& divisionIndexs){
  if (division.fill == FILL_EMPTY){
    return false;
  }else if (division.fill == FILL_FULL){
    return true;
  }

  std::vector<OctreeDivision*> divisions;
  for (auto index : divisionIndexs){
    divisions.push_back(&division.divisions.at(index));
  }
  for (auto division : divisions){
    if (!isSideFull(*division, divisionIndexs)){
      return false;
    }
  }
  return true;
}

std::vector<int> topSideIndexs = { 4, 5, 6, 7 };
bool isTopSideFull(OctreeDivision& division){
  return isSideFull(division, topSideIndexs);
}

std::vector<int> downSideIndexs = { 0, 1, 2, 3 };
bool isDownSideFull(OctreeDivision& division){
  return isSideFull(division, downSideIndexs);
}

std::vector<int> leftSideIndexs = { 1, 3, 5, 7 };
bool isLeftSideFull(OctreeDivision& division){
  return isSideFull(division, leftSideIndexs);
}

std::vector<int> rightSideIndexs = { 0, 2, 4, 6 };
bool isRightSideFull(OctreeDivision& division){
  return isSideFull(division, rightSideIndexs);
}

std::vector<int> frontSideIndexs = { 0, 1, 4, 5 };
bool isFrontSideFull(OctreeDivision& division){
  return isSideFull(division, frontSideIndexs);
}
std::vector<int> backSideIndexs = { 2, 3, 6, 7 };
bool isBackSideFull(OctreeDivision& division){
  return isSideFull(division, backSideIndexs);
}

bool shouldShowCubeSide(FillStatus fillStatus, OctreeSelectionFace side /*  { FRONT, BACK, LEFT, RIGHT, UP, DOWN }*/){
  // if it's mixed, can still check if some side of it is full 
  // so need to look at the further divided sections
  // should be able to be like fullSide(direction, octreeDivision)
  if (fillStatus.fill == FILL_FULL){
    return false;
  }else if (fillStatus.fill == FILL_EMPTY){
    return true;
  }

  OctreeDivision* octreeDivision = fillStatus.mixed.value();
  modassert(octreeDivision != NULL, "mixed should have provided octree division");

  if (side == UP){
    return !isTopSideFull(*octreeDivision);
  }else if (side == DOWN){
    return !isDownSideFull(*octreeDivision);
  }else if (side == LEFT){
    return !isLeftSideFull(*octreeDivision);
  }else if (side == RIGHT){
    return !isRightSideFull(*octreeDivision);
  }else if (side == FRONT){
    return !isFrontSideFull(*octreeDivision);
  }else if (side == BACK){
    return !isBackSideFull(*octreeDivision);
  }

  return true;
}

void addRamp(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, ShapeRamp& shapeRamp){
  float height = shapeRamp.endHeight - shapeRamp.startHeight;
  float depth = shapeRamp.endDepth - shapeRamp.startDepth;
  if (shapeRamp.direction == RAMP_RIGHT){
    glm::vec3 rampOffset(size * (1.f - shapeRamp.endDepth), size * shapeRamp.startHeight, 0.f);
    auto fullOffset = offset + rampOffset;

    addCubePointsLeft(points, size, fullOffset, faces, height);
    addCubePointsBottom(points, size, fullOffset, faces, 1.f, depth);

    FaceTexture& backFace =  faces -> at(1);
    points.push_back(OctreeVertex { .position = glm::vec3(size * depth, 0.f, -size) + fullOffset,  .coord = meshTexBottomLeft(backFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + fullOffset,  .coord = meshTexBottomRight(backFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + fullOffset, .coord = meshTexTopRight(backFace) });

    FaceTexture& frontFace =  faces -> at(0);
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + fullOffset, .coord = meshTexTopLeft(frontFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + fullOffset,  .coord = meshTexBottomLeft(frontFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size * depth, 0.f, 0.f) + fullOffset,  .coord = meshTexBottomRight(frontFace) });

    FaceTexture& topFace =  faces -> at(4);
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + fullOffset,   .coord = meshTexBottomLeft(topFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size * depth, 0.f, 0.f) + fullOffset,  .coord = meshTexBottomRight(topFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + fullOffset, .coord = meshTexTopLeft(topFace) });

    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + fullOffset,  .coord = meshTexTopLeft(topFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size * depth, 0.f, 0.f) + fullOffset,   .coord = meshTexBottomRight(topFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size * depth, 0.f, -size) + fullOffset, .coord = meshTexTopRight(topFace) });
  }else if (shapeRamp.direction == RAMP_LEFT){
    float height = shapeRamp.endHeight - shapeRamp.startHeight;
    float depth = shapeRamp.endDepth - shapeRamp.startDepth;
    auto rampOffset = glm::vec3 (size * shapeRamp.startDepth, size * shapeRamp.startHeight, 0.f);
    auto fullOffset = offset + rampOffset;
    addCubePointsRight(points, size, fullOffset + glm::vec3((-1.f +  depth) * size, 0.f, 0.f), faces, height);
    addCubePointsBottom(points, size, fullOffset, faces, 1.f, depth);

    FaceTexture& backFace =  faces -> at(1);
    points.push_back(OctreeVertex { .position = glm::vec3(size * depth, 0.f, -size) + fullOffset,  .coord = meshTexBottomLeft(backFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + fullOffset,  .coord = meshTexBottomRight(backFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size * depth, size * height, -size) + fullOffset, .coord = meshTexTopLeft(backFace) });

    FaceTexture& frontFace =  faces -> at(0);
    points.push_back(OctreeVertex { .position = glm::vec3(size * depth, size * height, 0.f) + fullOffset, .coord = meshTexTopRight(frontFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + fullOffset,  .coord = meshTexBottomLeft(frontFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size * depth, 0.f, 0.f) + fullOffset,  .coord = meshTexBottomRight(frontFace) });

    FaceTexture& topFace =  faces -> at(4);
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + fullOffset,   .coord = meshTexBottomLeft(topFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size * depth, size * height, 0.f) + fullOffset,  .coord = meshTexBottomRight(topFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + fullOffset, .coord = meshTexTopLeft(topFace) });

    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + fullOffset,  .coord = meshTexTopLeft(topFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size * depth, size * height, 0.f) + fullOffset,   .coord = meshTexBottomRight(topFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size * depth, size * height, -size) + fullOffset, .coord = meshTexTopRight(topFace) });
  }else if (shapeRamp.direction == RAMP_FORWARD){
    float height = shapeRamp.endHeight - shapeRamp.startHeight;
    float depth = shapeRamp.endDepth - shapeRamp.startDepth;
    auto rampOffset = glm::vec3(0.f, size * shapeRamp.startHeight, -size * shapeRamp.startDepth);
    auto fullOffset = offset + rampOffset;

    addCubePointsBack(points, size, fullOffset, faces, height, depth);
    addCubePointsBottom(points, size, fullOffset, faces, depth);

    FaceTexture& leftFace =  faces -> at(2);
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size * depth) + fullOffset, .coord = meshTexTopLeft(leftFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size * depth) + fullOffset,  .coord = meshTexBottomLeft(leftFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + fullOffset,    .coord = meshTexBottomRight(leftFace) });

    FaceTexture& rightFace =  faces -> at(3);
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + fullOffset,    .coord = meshTexBottomLeft(rightFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size * depth) + fullOffset,  .coord = meshTexBottomRight(rightFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size * depth) + fullOffset, .coord = meshTexTopRight(rightFace) });

    FaceTexture& frontFace =  faces -> at(0);
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + fullOffset, .coord = meshTexBottomRight(frontFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size * depth) + fullOffset, .coord = meshTexTopLeft(frontFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + fullOffset,  .coord = meshTexBottomLeft(frontFace) });

    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size * depth) + fullOffset, .coord = meshTexTopRight(frontFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size * depth) + fullOffset,  .coord = meshTexTopLeft(frontFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + fullOffset,  .coord = meshTexBottomRight(frontFace) });
  }else if (shapeRamp.direction == RAMP_BACKWARD){
    float height = shapeRamp.endHeight - shapeRamp.startHeight;
    glm::vec3 rampOffset(0.f, size * shapeRamp.startHeight, -size * (1.f - shapeRamp.endDepth));
    auto fullOffset = offset + rampOffset;

    addCubePointsFront(points, size, fullOffset, faces, height);
    addCubePointsBottom(points, size, fullOffset, faces, depth);

    FaceTexture& leftFace =  faces -> at(2);
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + fullOffset,   .coord = meshTexTopRight(leftFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size * depth) + fullOffset, .coord = meshTexBottomLeft(leftFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + fullOffset,    .coord = meshTexBottomRight(leftFace) });

    FaceTexture& rightFace =  faces -> at(3);
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + fullOffset,    .coord = meshTexBottomLeft(rightFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size * depth) + fullOffset, .coord = meshTexBottomRight(rightFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + fullOffset,   .coord = meshTexTopLeft(rightFace) });

    FaceTexture& topFace =  faces -> at(4);
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + fullOffset,   .coord = meshTexBottomLeft(topFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + fullOffset,  .coord = meshTexBottomRight(topFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size * depth) + fullOffset, .coord = meshTexTopLeft(topFace) });

    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size * depth) + fullOffset,  .coord = meshTexTopLeft(topFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + fullOffset,   .coord = meshTexBottomRight(topFace) });
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size * depth) + fullOffset, .coord = meshTexTopRight(topFace) });
  }else{
    modassert(false, "invalid ramp direction");
  }
}

void addBlockShapeMesh(Octree& octree, std::vector<OctreeVertex>& points, glm::vec3& rootPos, OctreeDivision& octreeDivision, ShapeBlock& shapeBlock,  glm::ivec3& cellAddress, float size, int subdivisionLevel){
  glm::ivec3 cellToTheFront(cellAddress.x, cellAddress.y, cellAddress.z - 1);
  if (shouldShowCubeSide(octreeFillStatus(octree, subdivisionLevel, cellToTheFront), FRONT)){
    addCubePointsFront(points, size, rootPos,  &octreeDivision.faces);
  }

  glm::ivec3 cellToTheBack(cellAddress.x, cellAddress.y, cellAddress.z + 1);
  if (shouldShowCubeSide(octreeFillStatus(octree, subdivisionLevel, cellToTheBack), BACK)){  
    addCubePointsBack(points, size, rootPos, &octreeDivision.faces);
  }
  //
  glm::ivec3 cellToTheLeft(cellAddress.x - 1, cellAddress.y, cellAddress.z);
  if (shouldShowCubeSide(octreeFillStatus(octree, subdivisionLevel, cellToTheLeft), LEFT)){  
    addCubePointsLeft(points, size, rootPos,  &octreeDivision.faces);
  }

  glm::ivec3 cellToTheRight(cellAddress.x + 1, cellAddress.y, cellAddress.z);
  if (shouldShowCubeSide(octreeFillStatus(octree, subdivisionLevel, cellToTheRight), RIGHT)){  
    addCubePointsRight(points, size, rootPos,  &octreeDivision.faces);
  }  

  glm::ivec3 cellToTheTop(cellAddress.x, cellAddress.y + 1, cellAddress.z);
  if (shouldShowCubeSide(octreeFillStatus(octree, subdivisionLevel, cellToTheTop), UP)){  
    addCubePointsTop(points, size, rootPos,  &octreeDivision.faces);
  }  

  glm::ivec3 cellToTheBottom(cellAddress.x, cellAddress.y - 1, cellAddress.z);
  if (shouldShowCubeSide(octreeFillStatus(octree, subdivisionLevel, cellToTheBottom), DOWN)){  
    addCubePointsBottom(points, size, rootPos,  &octreeDivision.faces);
  }      
}


struct OctreeToAdd {
  std::vector<int> path;
  OctreeDivision* octreeDivision;
  float size;
  int subdivisionLevel;
  glm::vec3 rootPos;
  glm::ivec3 cellAddress;
};


struct OctreeSearch {
  OctreeDivision* octreeDivision;
  std::vector<int> path;
  float size;
  int subdivisionLevel;
  glm::vec3 rootPos;
};

void addOctreeLevel(Octree& octree, glm::vec3 initialRootPos, OctreeDivision& initialOctreeDivision, float initialSize, int initialSubdivisionLevel, std::vector<int> intialPath, std::vector<OctreeToAdd>& allOctreeMeshes){
  std::queue<OctreeSearch> octreeToSearch;
  octreeToSearch.push(OctreeSearch{
    .octreeDivision = &initialOctreeDivision,
    .path = intialPath,
    .size = initialSize,
    .subdivisionLevel = initialSubdivisionLevel,
    .rootPos = initialRootPos,
  });
  while(octreeToSearch.size() > 0){
    OctreeSearch search =  octreeToSearch.front();
    octreeToSearch.pop();
    if (search.octreeDivision -> divisions.size() > 0){
      float subdivisionSize = search.size * 0.5f; 
      for (int i = 0; i < 8; i++) {
        std::vector<int> newPath = search.path;
        newPath.push_back(i);
        octreeToSearch.push(OctreeSearch{
          .octreeDivision = &search.octreeDivision -> divisions.at(i),
          .path = newPath,
          .size = subdivisionSize,
          .subdivisionLevel = search.subdivisionLevel + 1,
          .rootPos = offsetForFlatIndex(i, subdivisionSize, search.rootPos),
        });
      }
    }else if (search.octreeDivision -> fill == FILL_FULL) {
      auto cellIndex = indexForOctreePath(search.path);
      auto cellAddress = cellIndex.value;
      modassert(cellIndex.subdivisionLevel == search.subdivisionLevel, "invalid result for octree path, probably provided incorrect path for subdivisionLevel");
      allOctreeMeshes.push_back(OctreeToAdd{
        .path = search.path,
        .octreeDivision = search.octreeDivision,
        .size = search.size,
        .subdivisionLevel = search.subdivisionLevel,
        .rootPos = search.rootPos,
        .cellAddress = cellAddress,
      });
    }
  }
}


// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/#computing-the-tangents-and-bitangents
glm::vec3 computeTangent(Vertex v0, Vertex v1, Vertex v2){
  auto deltaPos1 = v1.position - v0.position;
  auto deltaPos2 = v2.position - v0.position;
  auto deltaUV1 = v1.texCoords - v0.texCoords;
  auto deltaUV2 = v2.texCoords - v0.texCoords;
  float r = 1.f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
  glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y) * r;
  return tangent;
}

Vertex createVertex2(glm::vec3 position, glm::vec2 texCoords, glm::vec3 normal){
  Vertex vertex {
    .position = position,
    .normal = normal,
    .tangent = glm::vec3(0.f, 0.f, 0.f), //  invalid value needs to be computed in context of triangle
    .texCoords = texCoords,
  };
  for (int i = 0; i < NUM_BONES_PER_VERTEX; i++){
    vertex.boneIndexes[i] = 0;
    vertex.boneWeights[i] = 0;
  }
  return vertex;
}

/*[1, 0, 1]
[1  0, 2]

[ 0 1 1 1 ]
[ 0 0 1 1 ]
[ 0 1 1 1 ]
[ 0 1 1 1 ]

1. decompose down to max subdivision levels, but make it sparse
2*/

Mesh getTestMesh();

Mesh createMeshFromPoints(std::vector<OctreeVertex>& points, std::function<Mesh(MeshData&)> loadMesh){
  std::vector<Vertex> vertices;
  for (int i = 0; i < points.size(); i+=3){
    glm::vec3 vec1 = points.at(i).position - points.at(i + 1).position;
    glm::vec3 vec2 = points.at(i).position - points.at(i + 2).position;
    auto normal = glm::cross(vec1, vec2); // think about sign better, i think this is right 
    vertices.push_back(createVertex2(points.at(i).position, points.at(i).coord, normal));  // maybe the tex coords should just be calculated as a ratio to a fix texture
    vertices.push_back(createVertex2(points.at(i + 1).position, points.at(i + 1).coord, normal));
    vertices.push_back(createVertex2(points.at(i + 2).position, points.at(i + 2).coord, normal));

    Vertex& v1 = vertices.at(vertices.size() - 3);
    Vertex& v2 = vertices.at(vertices.size() - 2);
    Vertex& v3 = vertices.at(vertices.size() - 1);
    auto tangent = computeTangent(v1, v2, v3);
    v1.tangent = tangent;
    v2.tangent = tangent;
    v3.tangent = tangent;
  }
  std::vector<unsigned int> indices;
  for (int i = 0; i < vertices.size(); i++){ // should be able to optimize this better...
    indices.push_back(i);
  }
 
  modassert(vertices.size() > 0, "no vertices create octree mesh");
  MeshData meshData {
    .vertices = vertices,
    .indices = indices,
    .diffuseTexturePath = "octree-atlas:main",
    .hasDiffuseTexture = true,
    .emissionTexturePath = "",
    .hasEmissionTexture = false,
    .opacityTexturePath = "",
    .hasOpacityTexture = false,
    .normalTexturePath = "octree-atlas:normal",
    .hasNormalTexture = true,
    .boundInfo = getBounds(vertices),
    .bones = {},
  };
  return loadMesh(meshData);
}

OctreeMeshes createOctreeMesh(Octree& octree, std::function<Mesh(MeshData&)> loadMesh){
  std::vector<OctreeVertex> points = {};
  std::vector<OctreeVertex> waterPoints = {};

  std::vector<OctreeToAdd> allOctreeMeshes;
  addOctreeLevel(octree, glm::vec3(0.f, 0.f, 0.f), octree.rootNode, 1.f, 0, {}, allOctreeMeshes);
  for (auto &octreeMesh : allOctreeMeshes){
    std::vector<OctreeVertex>* materialPoints = &points;
    if (octreeMesh.octreeDivision -> material == OCTREE_MATERIAL_DEFAULT){
      // do nothing
    }else if (octreeMesh.octreeDivision -> material == OCTREE_MATERIAL_WATER){
      materialPoints = &waterPoints;
    }else{
      modassert(false, "invalid material");
    }

    auto blockShape = std::get_if<ShapeBlock>(&octreeMesh.octreeDivision -> shape);
    if (blockShape){
      addBlockShapeMesh(octree, *materialPoints, octreeMesh.rootPos, *octreeMesh.octreeDivision,  *blockShape, octreeMesh.cellAddress, octreeMesh.size, octreeMesh.subdivisionLevel);
    }
    auto rampShape = std::get_if<ShapeRamp>(&octreeMesh.octreeDivision -> shape);
    if (rampShape){
      addRamp(*materialPoints, octreeMesh.size, octreeMesh.rootPos, &octreeMesh.octreeDivision -> faces, *rampShape);
    }
  }

  OctreeMeshes octreeMeshes {
    .mesh = createMeshFromPoints(points, loadMesh),
    .waterMesh =  waterPoints.size() > 0 ? createMeshFromPoints(waterPoints, loadMesh) : std::optional<Mesh>(std::nullopt), 
  };
  return octreeMeshes;
}