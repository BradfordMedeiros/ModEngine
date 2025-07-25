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
  glm::vec3 color;
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

std::vector<OctreeDivision*> getOctreeSubdivisions(Octree& octree, int x, int y, int z, int subdivision){
  std::cout << "tags getOctreeSubdivisions" << std::endl;
  std::vector<OctreeDivision*> tags;
  if (x < 0 || y < 0 || z < 0){
    return tags;
  }
  auto biggestSubdivisionSize = glm::pow(2, subdivision);
  if (x >= biggestSubdivisionSize || y >= biggestSubdivisionSize || z >= biggestSubdivisionSize){
    std::cout << "returning tags" << std::endl;
    return tags;
  }
  auto path = octreePath(x, y, z, subdivision);
  std::cout << "tags path size: " << path.size() << std::endl;

  OctreeDivision* octreeSubdivision = &octree.rootNode;
  tags.push_back(octreeSubdivision);

  for (int i = 0; i < path.size(); i++){
    int index = xyzIndexToFlatIndex(path.at(i));
    if (octreeSubdivision -> fill == FILL_EMPTY){
      std::cout << "returning tags empty" << std::endl;
      return tags;
    }else if (octreeSubdivision -> fill == FILL_FULL){
      std::cout << "returning tags full" << std::endl;
      return tags;
    }
    modassert(octreeSubdivision -> divisions.size() == 8, "expected 8 subdivisions");
    octreeSubdivision = &(octreeSubdivision -> divisions.at(index));
    std::cout << "searching more tags: going to subdivision " << index  << std::endl;
    tags.push_back(octreeSubdivision);
  }
  return tags;
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
  OctreeMaterial material;
};

FillStatus octreeFillStatus(Octree& octree, int subdivisionLevel, glm::ivec3 division){
  // this should be looking at the target subdivsiion level, and any level before it 

  auto octreeDivision = getOctreeSubdivisionIfExists2(octree, division.x, division.y, division.z, subdivisionLevel);
  if (!octreeDivision){
    return FillStatus { .fill = FILL_EMPTY, .mixed = std::nullopt, .material = OCTREE_MATERIAL_DEFAULT };
  }

  auto blockShape = std::get_if<ShapeBlock>(&octreeDivision -> shape);  // depeneding on the side of this, we could hide more faces
  if (blockShape == NULL){
    return FillStatus { .fill = FILL_EMPTY, .mixed = octreeDivision, .material = octreeDivision -> material };
  }

  // if this is mixed, then we need to check the corresponding side and see if it's filled 
  if (octreeDivision -> fill == FILL_MIXED){
    return FillStatus { .fill = FILL_MIXED, .mixed = octreeDivision, .material = octreeDivision -> material };
  }
  return FillStatus { .fill = octreeDivision -> fill, .mixed = octreeDivision, .material = octreeDivision -> material };
}

  // -x +y -z 
  // +x +y -z
  // -x +y +z
  // +x +y +z
  // -x -y -z 
  // +x -y -z
  // -x -y +z
  // +x -y +z

bool isSideFull(OctreeDivision& division, std::vector<int>& divisionIndexs, FillStatus& fillStatus, bool* hasWater){
  if (division.fill == FILL_EMPTY){
    return false;
  }else if (division.fill == FILL_FULL){
    //if (division.material == OCTREE_MATERIAL_WATER && fillStatus.material != OCTREE_MATERIAL_WATER){
    //  return false;
    //}
    if (division.material == OCTREE_MATERIAL_WATER){
      *hasWater = true;
    }
    return true;
  }

  std::vector<OctreeDivision*> divisions;
  for (auto index : divisionIndexs){
    divisions.push_back(&division.divisions.at(index));
  }
  for (auto division : divisions){
    if (!isSideFull(*division, divisionIndexs, fillStatus, hasWater)){
      return false;
    }
  }
  return true;
}

std::vector<int> topSideIndexs = { 0, 1, 2, 3 };
bool isTopSideFull(OctreeDivision& division, FillStatus& fillStatus, bool* hasWater){
  return isSideFull(division, topSideIndexs, fillStatus, hasWater);
}

std::vector<int> downSideIndexs = { 4, 5, 6, 7 };
bool isDownSideFull(OctreeDivision& division, FillStatus& fillStatus, bool* hasWater){
  return isSideFull(division, downSideIndexs, fillStatus, hasWater);
}

std::vector<int> leftSideIndexs = { 0, 2, 4, 6 };
bool isLeftSideFull(OctreeDivision& division, FillStatus& fillStatus, bool* hasWater){
  return isSideFull(division, leftSideIndexs, fillStatus, hasWater);
}

std::vector<int> rightSideIndexs = { 1, 3, 5, 7 };
bool isRightSideFull(OctreeDivision& division, FillStatus& fillStatus, bool* hasWater){
  return isSideFull(division, rightSideIndexs, fillStatus, hasWater);
}

std::vector<int> frontSideIndexs = { 2, 3, 6, 7 };
bool isFrontSideFull(OctreeDivision& division, FillStatus& fillStatus, bool* hasWater){
  return isSideFull(division, frontSideIndexs, fillStatus, hasWater);
}
std::vector<int> backSideIndexs = { 0, 1, 4, 5 };
bool isBackSideFull(OctreeDivision& division, FillStatus& fillStatus, bool* hasWater){
  return isSideFull(division, backSideIndexs, fillStatus, hasWater);
}

bool isFaceFull(FillStatus fillStatus, OctreeSelectionFace side /*  { FRONT, BACK, LEFT, RIGHT, UP, DOWN }*/, bool* neighborHasWater){
  // if it's mixed, can still check if some side of it is full 
  // so need to look at the further divided sections
  // should be able to be like fullSide(direction, octreeDivision)


  if (!fillStatus.mixed.has_value()){
    return false;
  }
  OctreeDivision* octreeDivision = fillStatus.mixed.value();
  modassert(octreeDivision != NULL, "mixed should have provided octree division");

  if (side == DOWN){
    return isDownSideFull(*octreeDivision, fillStatus, neighborHasWater);
  }else if (side == UP){
    return isTopSideFull(*octreeDivision, fillStatus, neighborHasWater);
  }else if (side == RIGHT){
    return isRightSideFull(*octreeDivision, fillStatus, neighborHasWater);
  }else if (side == LEFT){
    return isLeftSideFull(*octreeDivision, fillStatus, neighborHasWater);
  }else if (side == BACK){
    return isBackSideFull(*octreeDivision, fillStatus, neighborHasWater);
  }else if (side == FRONT){
    return isFrontSideFull(*octreeDivision, fillStatus, neighborHasWater);
  }
  return false;
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
  
  auto cellIsWater = octreeDivision.material == OCTREE_MATERIAL_WATER;
  {
    bool sideHasWater = false;
    glm::ivec3 cellToTheFront(cellAddress.x, cellAddress.y, cellAddress.z - 1);
    auto isFull = isFaceFull(octreeFillStatus(octree, subdivisionLevel, cellToTheFront), BACK, &sideHasWater);
    bool shouldShow = (!isFull && !(cellIsWater && sideHasWater)) || (!cellIsWater && sideHasWater) ;
    if (shouldShow){
      addCubePointsFront(points, size, rootPos,  &octreeDivision.faces);
    }    
  }

  {
    bool sideHasWater = false;
    glm::ivec3 cellToTheBack(cellAddress.x, cellAddress.y, cellAddress.z + 1);
    auto isFull = isFaceFull(octreeFillStatus(octree, subdivisionLevel, cellToTheBack), FRONT, &sideHasWater);
    bool shouldShow = (!isFull && !(cellIsWater && sideHasWater)) || (!cellIsWater && sideHasWater) ;
    if (shouldShow){  
      addCubePointsBack(points, size, rootPos, &octreeDivision.faces);
    }
  }

  //

  {
    bool sideHasWater = false;
    glm::ivec3 cellToTheLeft(cellAddress.x - 1, cellAddress.y, cellAddress.z);
    auto isFull = isFaceFull(octreeFillStatus(octree, subdivisionLevel, cellToTheLeft), RIGHT, &sideHasWater);
    bool shouldShow = (!isFull && !(cellIsWater && sideHasWater)) || (!cellIsWater && sideHasWater) ;
    if (shouldShow){  
      addCubePointsLeft(points, size, rootPos,  &octreeDivision.faces);
    }    
  }

  {
    bool sideHasWater = false;
    glm::ivec3 cellToTheRight(cellAddress.x + 1, cellAddress.y, cellAddress.z);
    auto isFull = isFaceFull(octreeFillStatus(octree, subdivisionLevel, cellToTheRight), LEFT, &sideHasWater);
    bool shouldShow = (!isFull && !(cellIsWater && sideHasWater)) || (!cellIsWater && sideHasWater) ;
    if (shouldShow){  
      addCubePointsRight(points, size, rootPos,  &octreeDivision.faces);
    }      
  }

  {
    bool sideHasWater = false;
    glm::ivec3 cellToTheTop(cellAddress.x, cellAddress.y + 1, cellAddress.z);
    auto isFull = isFaceFull(octreeFillStatus(octree, subdivisionLevel, cellToTheTop), DOWN, &sideHasWater);
    bool shouldShow = (!isFull && !(cellIsWater && sideHasWater)) || (!cellIsWater && sideHasWater) ;
    if (shouldShow){  
      addCubePointsTop(points, size, rootPos,  &octreeDivision.faces);
    }  
  }

  {
    bool sideHasWater = false;
    glm::ivec3 cellToTheBottom(cellAddress.x, cellAddress.y - 1, cellAddress.z);
    auto isFull = isFaceFull(octreeFillStatus(octree, subdivisionLevel, cellToTheBottom), UP, &sideHasWater);
    bool shouldShow = (!isFull && !(cellIsWater && sideHasWater)) || (!cellIsWater && sideHasWater) ;
    if (shouldShow){  
      addCubePointsBottom(points, size, rootPos,  &octreeDivision.faces);
    }    
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

void addOctreeLevelTags(Octree& octree, glm::vec3 initialRootPos, OctreeDivision& initialOctreeDivision, float initialSize, int initialSubdivisionLevel, std::vector<int> intialPath, std::vector<OctreeToAdd>& allOctreeMeshes){
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
    if (search.octreeDivision -> tags.size() > 0) {
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

Vertex createVertex2(glm::vec3 position, glm::vec2 texCoords, glm::vec3 normal, glm::vec3 color){
  Vertex vertex {
    .position = position,
    .normal = normal,
    .tangent = glm::vec3(0.f, 0.f, 0.f), //  invalid value needs to be computed in context of triangle
    .color = color,
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

Mesh createMeshFromPoints(std::vector<OctreeVertex>& points, std::function<Mesh(MeshData&)> loadMesh, const char* texture, const char* normal){
  std::vector<Vertex> vertices;
  for (int i = 0; i < points.size(); i+=3){
    glm::vec3 vec1 = points.at(i).position - points.at(i + 1).position;
    glm::vec3 vec2 = points.at(i).position - points.at(i + 2).position;
    auto normal = glm::cross(vec1, vec2); // think about sign better, i think this is right 
    vertices.push_back(createVertex2(points.at(i).position, points.at(i).coord, normal, points.at(i).color));  // maybe the tex coords should just be calculated as a ratio to a fix texture
    vertices.push_back(createVertex2(points.at(i + 1).position, points.at(i + 1).coord, normal, points.at(i + 1).color));
    vertices.push_back(createVertex2(points.at(i + 2).position, points.at(i + 2).coord, normal, points.at(i + 2).color));

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
    .diffuseTexturePath = texture,
    .hasDiffuseTexture = true,
    .emissionTexturePath = "",
    .hasEmissionTexture = false,
    .opacityTexturePath = "",
    .hasOpacityTexture = false,
    .normalTexturePath = normal,
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

    int startingIndex = materialPoints -> size();

    auto blockShape = std::get_if<ShapeBlock>(&octreeMesh.octreeDivision -> shape);
    if (blockShape){
      addBlockShapeMesh(octree, *materialPoints, octreeMesh.rootPos, *octreeMesh.octreeDivision,  *blockShape, octreeMesh.cellAddress, octreeMesh.size, octreeMesh.subdivisionLevel);
    }
    auto rampShape = std::get_if<ShapeRamp>(&octreeMesh.octreeDivision -> shape);
    if (rampShape){
      addRamp(*materialPoints, octreeMesh.size, octreeMesh.rootPos, &octreeMesh.octreeDivision -> faces, *rampShape);
    }

    int endingIndex = materialPoints -> size();
    for (int i = startingIndex; i < endingIndex; i++){
      materialPoints -> at(i).color = octreeMesh.octreeDivision -> color;
    }
  }


  OctreeMeshes octreeMeshes {
    .mesh = createMeshFromPoints(points, loadMesh, "octree-atlas:main", "octree-atlas:normal"),
    .waterMesh =  waterPoints.size() > 0 ? createMeshFromPoints(waterPoints, loadMesh, resources::TEXTURE_WATER, resources::TEXTURE_WATER_NORMAL) : std::optional<Mesh>(std::nullopt), 
  };
  return octreeMeshes;
}

void visualizeTags(Octree& octree, int tag, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine2){
  std::vector<OctreeToAdd> allOctreeMeshes;
  addOctreeLevelTags(octree, glm::vec3(0.f, 0.f, 0.f), octree.rootNode, 1.f, 0, {}, allOctreeMeshes);
  
  std::cout << "octree tags: " << allOctreeMeshes.size() << std::endl;
  for (auto &octreeMesh : allOctreeMeshes){
    bool foundTag = false;
    for (auto divisionTag : octreeMesh.octreeDivision -> tags){
      if (tag == divisionTag.key){
        foundTag = true;
        break;
      }
    }
    if (foundTag){
      auto cellAddress = octreeMesh.cellAddress;
      std::cout << "cell: " << print(cellAddress) << " - sub - " << octreeMesh.subdivisionLevel << ", size = " << octreeMesh.size << std::endl;
      drawGridSelectionCube(cellAddress.x, cellAddress.y, cellAddress.z, 1, 1, 1, octreeMesh.subdivisionLevel, 1.f, drawLine2, std::nullopt);      
    }
  }
}

glm::ivec3 positionToCell(glm::vec3 position, int subdivision){
  // sub0 - 1   [1]
  // sub1 - 0-1 [2]
  // sub3 - 0-3 [4]
  auto numBlocks = glm::pow(2, subdivision);
  auto widthPerBlock = 1.f / numBlocks;
  int xIndex = position.x / widthPerBlock;
  int yIndex = position.y / widthPerBlock;
  int zIndex = -1 * position.z / widthPerBlock;
  return glm::ivec3(xIndex, yIndex, zIndex);
}

std::vector<TagInfo> getTag(Octree& octree, int tag, glm::vec3 position, int subdivision){
  std::vector<OctreeToAdd> allOctreeMeshes;
  auto cellAddress = positionToCell(position, subdivision);
  auto subdivisions = getOctreeSubdivisions(octree, cellAddress.x, cellAddress.y, cellAddress.z, subdivision);
  //std::cout << "tags game get tag from octree, localpos = " << print(position) << ", celladdress = " << print(cellAddress) << ", subdivision in = " << subdivision <<  ",  subdivisions size = " << subdivisions.size()  << std::endl;
  std::vector<TagInfo> tags;
  for (auto subdivision : subdivisions){
    for (auto tagInSubdivision : subdivision -> tags){
      if (tagInSubdivision.key == tag){
        tags.push_back(TagInfo {
          .key = tagInSubdivision.key,
          .value = tagInSubdivision.value,
        });
        break;     
      }
    }
  }
  return tags;
}

std::optional<OctreeMaterial> getMaterial(Octree& octree, glm::vec3 position, int subdivision){
  std::vector<OctreeToAdd> allOctreeMeshes;
  auto cellAddress = positionToCell(position, subdivision);
  auto subdivisions = getOctreeSubdivisions(octree, cellAddress.x, cellAddress.y, cellAddress.z, subdivision);
  //std::cout << "tags game get tag from octree, localpos = " << print(position) << ", celladdress = " << print(cellAddress) << ", subdivision in = " << subdivision <<  ",  subdivisions size = " << subdivisions.size()  << std::endl;
 
  if (subdivisions.size() == 0){
    return std::nullopt;
  }
  OctreeDivision* division = subdivisions.at(subdivisions.size() - 1); 

  if (division -> fill != FILL_FULL){  // not sure i like this condition, kind of feels like an inconsistent data strucutre
    return std::nullopt;
  }
  return division -> material;
}