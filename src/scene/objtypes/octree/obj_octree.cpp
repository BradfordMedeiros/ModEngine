#include "./obj_octree.h"

std::string readFileOrPackage(std::string filepath);

std::optional<glm::ivec3> selectedIndex = glm::ivec3(1, 0, 0);
std::optional<glm::ivec3> selectionDim = glm::ivec3(1, 1, 0);
OctreeSelectionFace editorOrientation = FRONT;
int selectedTexture = 0;

std::optional<Line> line = std::nullopt;
int subdivisionLevel = 1;
std::optional<objid> selectedOctreeId = std::nullopt;

struct FaceIntersection {
  OctreeSelectionFace face;
  glm::vec3 position;
};
struct RaycastIntersection {
  int index;
  glm::ivec3 blockOffset;
  std::vector<FaceIntersection> faceIntersections;
};
struct RaycastResult {
  glm::vec3 fromPos;
  glm::vec3 toPosDirection;
  int subdivisionDepth;
  std::vector<RaycastIntersection> intersections;
};

struct Intersection {
  int index;
  std::vector<FaceIntersection> faceIntersections;
};

struct ClosestIntersection {
  OctreeSelectionFace face;
  glm::vec3 position;
  glm::ivec3 xyzIndex;
  int subdivisionDepth;
};

std::optional<RaycastResult> raycastResult = std::nullopt;
std::optional<ClosestIntersection> closestRaycast = std::nullopt;

std::optional<AtlasDimensions> atlasDimensions = AtlasDimensions {
  .textureNames = {
    resources::GRID_TEXTURE,
    resources::TUNNELROAD_TEXTURE,  
    resources::TEXTURE_GRASS, 
    resources::TEXTURE_PEBBLES, 
    resources::TEXTURE_METALGRID, 
    resources::TEXTURE_DRYFOREST, 
    resources::TEXTURE_FOLIAGE, 
    resources::TEXTURE_METAL_SCIFI,

    resources::TEXTURE_ICE,
    resources::TEXTURE_ICE,
    resources::TEXTURE_ICE,
    resources::TEXTURE_ICE,
    resources::TEXTURE_ICE,
    resources::TEXTURE_ICE,
    resources::TEXTURE_ICE,
    resources::TEXTURE_ICE,
  },
};

void setAtlasDimensions(AtlasDimensions newAtlasDimensions){
  atlasDimensions = newAtlasDimensions;
  auto textureDim = calculateAtlasImageDimension(atlasDimensions.value().textureNames.size());
  modlog("set atlas", std::string("textureDim = ") + std::to_string(textureDim) + ", total = " + std::to_string(newAtlasDimensions.textureNames.size()));
}


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

FaceTexture texCoords(int imageIndex, TextureOrientation texOrientation = TEXTURE_UP, int xTileDim = 1, int yTileDim = 1,  int x = 0, int y = 0){
  glm::vec2 offset(x, y);

  if (texOrientation == TEXTURE_UP){
    // do nothing
  }else if (texOrientation == TEXTURE_DOWN){
    offset.x = xTileDim - x - 1;
    offset.y = yTileDim - y - 1;
  }else if (texOrientation == TEXTURE_RIGHT){
    int oldXTileDim = xTileDim;
    int oldYTileDim = yTileDim;
    xTileDim = oldYTileDim;
    yTileDim = oldXTileDim;
    offset.x = oldYTileDim - y - 1;
    offset.y = x;
  }else if (texOrientation == TEXTURE_LEFT){
    int oldXTileDim = xTileDim;
    int oldYTileDim = yTileDim;
    xTileDim = oldYTileDim;
    yTileDim = oldXTileDim;
    offset.x = y;
    offset.y = oldXTileDim - x - 1;
  }

  glm::vec2 multiplier(1.f / xTileDim, 1.f / yTileDim);
  float xMin = multiplier.x * offset.x;
  float xMax = (multiplier.x * offset.x) + multiplier.x;
  float yMin = multiplier.y * offset.y;
  float yMax = (multiplier.y * offset.y) + multiplier.y;

  //std::cout << "update texcoord ndi x: " << multiplier.x << ", " << offset.x << ", * = " << (multiplier.x * offset.x) << std::endl;
  //std::cout << "update texcoord ndi y: " << multiplier.y << ", " << offset.y << ", * = " << (multiplier.y * offset.y) << std::endl;
  //std::cout << "update texcoords: " << print(glm::vec2(xMin, xMax)) << std::endl;
  //std::cout << "update --------------------------" << std::endl;
  FaceTexture faceTexture {
    .textureIndex = imageIndex,
    .texCoordsTopLeft = glm::vec2(xMin, yMax),
    .texCoordsTopRight = glm::vec2(xMax, yMax),
    .texCoordsBottomLeft = glm::vec2(xMin, yMin),
    .texCoordsBottomRight = glm::vec2(xMax, yMin),
  };
  if (texOrientation == TEXTURE_DOWN){
    faceTexture = FaceTexture {
      .textureIndex = imageIndex,
      .texCoordsTopLeft = glm::vec2(xMax, yMin),
      .texCoordsTopRight = glm::vec2(xMin, yMin),
      .texCoordsBottomLeft = glm::vec2(xMax, yMax),
      .texCoordsBottomRight = glm::vec2(xMin, yMax),
    };
  }
  if (texOrientation == TEXTURE_RIGHT){
    faceTexture = FaceTexture {
      .textureIndex = imageIndex,
      .texCoordsTopLeft = glm::vec2(xMin, yMin),
      .texCoordsTopRight = glm::vec2(xMin, yMax),
      .texCoordsBottomLeft = glm::vec2(xMax, yMin),
      .texCoordsBottomRight = glm::vec2(xMax, yMax),
    };
  }
  if (texOrientation == TEXTURE_LEFT){
    faceTexture = FaceTexture {
      .textureIndex = imageIndex,
      .texCoordsTopLeft = glm::vec2(xMax, yMax),
      .texCoordsTopRight = glm::vec2(xMax, yMin),
      .texCoordsBottomLeft = glm::vec2(xMin, yMax),
      .texCoordsBottomRight = glm::vec2(xMin, yMin),
    };
  }

  return faceTexture;
}

std::vector<FaceTexture> defaultTextureCoords = {
  texCoords(0),
  texCoords(0),
  texCoords(0),
  texCoords(0),
  texCoords(0),
  texCoords(0),
};


bool allFilledIn(OctreeDivision& octreeDivision, FillType fillType){
  if (octreeDivision.divisions.size() != 8){
    return false;
  }
  for (auto &division : octreeDivision.divisions){
    if (division.fill != fillType){
      return false;
    }
  }
  return true;
}

void writeOctreeCell(Octree& octree, int x, int y, int z, int subdivision, bool filled){
  OctreeDivision* octreeSubdivision = &octree.rootNode;

  std::vector<OctreeDivision*> parentSubdivisions;
  auto path = octreePath(x, y, z, subdivision);

  std::cout << "octree path: [";
  for (auto &coord : path){
    std::cout << print(coord) << ", ";
  }
  std::cout << "]" << std::endl;

  for (int i = 0; i < path.size(); i++){
    // todo -> if the subdivision isn't made here, should make it here
    if (octreeSubdivision -> divisions.size() == 0){
      auto defaultFill = octreeSubdivision -> fill;
      octreeSubdivision -> divisions = {
        OctreeDivision { .fill = defaultFill, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
      };
    } 
    // check if all filled, then set the divsions = {}, and filled = true
    parentSubdivisions.push_back(octreeSubdivision);
    octreeSubdivision = &octreeSubdivision -> divisions.at(xyzIndexToFlatIndex(path.at(i)));
  }

  octreeSubdivision -> fill = filled ? FILL_FULL : FILL_EMPTY;
  octreeSubdivision -> divisions = {};

  for (int i = parentSubdivisions.size() - 1; i >= 0; i--){
    auto parentNode = parentSubdivisions.at(i);
    parentNode -> shape = ShapeBlock{};

    if (allFilledIn(*parentNode, FILL_FULL)){
      parentNode -> divisions = {};
      parentNode -> fill = FILL_FULL;
    }else if (allFilledIn(*parentNode, FILL_EMPTY)){
      parentNode -> divisions = {};
      parentNode -> fill = FILL_EMPTY;
    }else{
      parentNode -> fill = FILL_MIXED;
    }
    modassert(parentNode -> divisions.size() == 8 ? parentNode -> fill == FILL_MIXED : true,  "write octree - no divisions, but mixed fill");
  }
  modassert(octreeSubdivision -> divisions.size() == 8 ? octreeSubdivision -> fill == FILL_MIXED : true, "write octree - no divisions, but mixed fill");
}

OctreeDivision* getOctreeSubdivisionIfExists(Octree& octree, int x, int y, int z, int subdivision){
  OctreeDivision* octreeSubdivision = &octree.rootNode;
  auto path = octreePath(x, y, z, subdivision);
  for (int i = 0; i < path.size(); i++){
    int index = xyzIndexToFlatIndex(path.at(i));
    if (octreeSubdivision -> divisions.size() == 0){
      return NULL;
    }
    octreeSubdivision = &(octreeSubdivision -> divisions.at(index));
  }
  return octreeSubdivision;
}

void writeOctreeCellRange(Octree& octree, int x, int y, int z, int width, int height, int depth, int subdivision, bool filled){
  for (int i = 0; i < width; i++){
    for (int j = 0; j < height; j++){
      for (int k = 0; k < depth; k++){
        writeOctreeCell(octree, x + i, y + j, z + k, subdivision, filled);
      }
    }
  }
}

int textureIndex(OctreeSelectionFace faceOrientation){
  int index = 0;
  if (faceOrientation == FRONT){
    index = 0;
  }else if (faceOrientation == BACK){
    index = 1;
  }else if (faceOrientation == LEFT){
    index = 2;
  }else if (faceOrientation == RIGHT){
    index = 3;
  }else if (faceOrientation == UP){
    index = 4;
  }else if (faceOrientation == DOWN){
    index = 5;
  }else{
    modassert(false, "writeOctreeTexture invalid face");
  }
  return index;  
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


struct OctreeVertex {
  glm::vec3 position;
  glm::vec2 coord;
};


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

void addCubePointsFront(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f){
  modassert(faces -> size() == 6, std::string("faces size unexpected, got: " + std::to_string(faces -> size())));
  FaceTexture& frontFace =  faces -> at(0);
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset, .coord = meshTexBottomRight(frontFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + offset, .coord = meshTexTopLeft(frontFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,  .coord = meshTexBottomLeft(frontFace) });

  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + offset, .coord = meshTexTopRight(frontFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + offset,  .coord = meshTexTopLeft(frontFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,  .coord = meshTexBottomRight(frontFace)  });
}
void addCubePointsBack(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f, float depth = 1.f){
  modassert(faces -> size() == 6, std::string("faces size unexpected, got: " + std::to_string(faces -> size())));
  FaceTexture& backFace =  faces -> at(1);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size * depth) + offset,  .coord = meshTexBottomRight(backFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size * depth) + offset, .coord = meshTexTopRight(backFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size * depth) + offset, .coord = meshTexBottomLeft(backFace) });

  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size * depth) + offset,  .coord = meshTexBottomLeft(backFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size * depth) + offset,  .coord = meshTexTopRight(backFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size * depth) + offset, .coord = meshTexTopLeft(backFace) });
}
void addCubePointsLeft(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f){
  modassert(faces -> size() == 6, std::string("faces size unexpected, got: " + std::to_string(faces -> size())));
  FaceTexture& leftFace =  faces -> at(2);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + offset, .coord = meshTexTopLeft(leftFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + offset,  .coord = meshTexBottomLeft(leftFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,    .coord = meshTexBottomRight(leftFace) });

  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + offset,   .coord = meshTexTopRight(leftFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + offset, .coord = meshTexTopLeft(leftFace)  });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,    .coord = meshTexBottomRight(leftFace) });
}
void addCubePointsRight(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f){
  modassert(faces -> size() == 6, std::string("faces size unexpected, got: " + std::to_string(faces -> size())));
  FaceTexture& rightFace =  faces -> at(3);
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,    .coord = meshTexBottomLeft(rightFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + offset,  .coord = meshTexBottomRight(rightFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size) + offset, .coord = meshTexTopRight(rightFace) });

  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,    .coord = meshTexBottomLeft(rightFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size) + offset, .coord = meshTexTopRight(rightFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + offset,   .coord = meshTexTopLeft(rightFace) });
}
void addCubePointsTop(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f){
  modassert(faces -> size() == 6, std::string("faces size unexpected, got: " + std::to_string(faces -> size())));
  FaceTexture& topFace =  faces -> at(4);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + offset,   .coord = meshTexBottomLeft(topFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + offset,  .coord = meshTexBottomRight(topFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + offset, .coord = meshTexTopLeft(topFace) });

  points.push_back(OctreeVertex { .position = glm::vec3(0.f,  size * height, -size) + offset,  .coord = meshTexTopLeft(topFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + offset,   .coord = meshTexBottomRight(topFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size) + offset, .coord = meshTexTopRight(topFace) });
}
void addCubePointsBottom(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float depth = 1.f, float width = 1.f){
  modassert(faces -> size() == 6, std::string("faces size unexpected, got: " + std::to_string(faces -> size())));
  FaceTexture& bottomFace =  faces -> at(5);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size * depth) + offset, .coord = meshTexBottomLeft(bottomFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(width * size, 0.f, 0.f) + offset,  .coord = meshTexTopRight(bottomFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,   .coord = meshTexTopLeft(bottomFace) });

  points.push_back(OctreeVertex { .position = glm::vec3(width * size, 0.f, -size * depth) + offset, .coord = meshTexBottomRight(bottomFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(width * size, 0.f, 0.f) + offset,   .coord = meshTexTopRight(bottomFace) });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size * depth) + offset,  .coord = meshTexBottomLeft(bottomFace) });
}

int calcBiggestSize(int subdivisionLevel){
  return glm::pow(2, subdivisionLevel);
}

// does not account if a larger subdivision exists and is filled
OctreeDivision* getOctreeSubdivisionIfExists2(Octree& octree, int x, int y, int z, int subdivision){
  if (x < 0 || y < 0 || z < 0){
    return NULL;
  }
  auto biggestSubdivisionSize = calcBiggestSize(subdivision);
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

void addOctreeLevel(Octree& octree, std::vector<OctreeVertex>& points, glm::vec3 rootPos, OctreeDivision& octreeDivision, float size, int subdivisionLevel, std::vector<int> path){
  std::cout << "addOctreeLevel: " << size << std::endl;
  if (octreeDivision.divisions.size() > 0){
    float subdivisionSize = size * 0.5f; 

    // -x +y -z
    auto topLeftFrontPath = path;
    topLeftFrontPath.push_back(0);
    addOctreeLevel(octree, points, offsetForFlatIndex(0, subdivisionSize, rootPos), octreeDivision.divisions.at(0), subdivisionSize, subdivisionLevel + 1, topLeftFrontPath);

    // +x +y -z
    auto topRightFrontPath = path;
    topRightFrontPath.push_back(1);
    addOctreeLevel(octree, points, offsetForFlatIndex(1, subdivisionSize, rootPos), octreeDivision.divisions.at(1), subdivisionSize, subdivisionLevel + 1, topRightFrontPath);

    // -x +y +z
    auto topLeftBackPath = path;
    topLeftBackPath.push_back(2);
    addOctreeLevel(octree, points, offsetForFlatIndex(2, subdivisionSize, rootPos), octreeDivision.divisions.at(2), subdivisionSize, subdivisionLevel + 1, topLeftBackPath);

    // +x +y +z
    auto topRightBackPath = path;
    topRightBackPath.push_back(3);
    addOctreeLevel(octree, points, offsetForFlatIndex(3, subdivisionSize, rootPos), octreeDivision.divisions.at(3), subdivisionSize, subdivisionLevel + 1, topRightBackPath);

    // -x -y -z
    auto bottomLeftFrontPath = path;
    bottomLeftFrontPath.push_back(4);
    addOctreeLevel(octree, points, offsetForFlatIndex(4, subdivisionSize, rootPos), octreeDivision.divisions.at(4), subdivisionSize, subdivisionLevel + 1, bottomLeftFrontPath);

    // +x -y -z
    auto bottomRightFrontPath = path;
    bottomRightFrontPath.push_back(5);
    addOctreeLevel(octree, points, offsetForFlatIndex(5, subdivisionSize, rootPos), octreeDivision.divisions.at(5), subdivisionSize, subdivisionLevel + 1, bottomRightFrontPath);

    // -x -y +z
    auto bottomLeftBackPath = path;
    bottomLeftBackPath.push_back(6);
    addOctreeLevel(octree, points, offsetForFlatIndex(6, subdivisionSize, rootPos), octreeDivision.divisions.at(6), subdivisionSize, subdivisionLevel + 1, bottomLeftBackPath);

    // +x -y +z
    auto bottomRightBackPath = path;
    bottomRightBackPath.push_back(7);
    addOctreeLevel(octree, points, offsetForFlatIndex(7, subdivisionSize, rootPos), octreeDivision.divisions.at(7), subdivisionSize, subdivisionLevel + 1, bottomRightBackPath);
  }else if (octreeDivision.fill == FILL_FULL){
    auto cellIndex = indexForOctreePath(path);
    auto cellAddress = cellIndex.value;
    modassert(cellIndex.subdivisionLevel == subdivisionLevel, "invalid result for octree path, probably provided incorrect path for subdivisionLevel");

    auto blockShape = std::get_if<ShapeBlock>(&octreeDivision.shape);
    if (blockShape){
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
    auto rampShape = std::get_if<ShapeRamp>(&octreeDivision.shape);
    if (rampShape){
      addRamp(points, size, rootPos, &octreeDivision.faces, *rampShape);
    }

  }
}


/*[1, 0, 1]
[1  0, 2]

[ 0 1 1 1 ]
[ 0 0 1 1 ]
[ 0 1 1 1 ]
[ 0 1 1 1 ]

1. decompose down to max subdivision levels, but make it sparse
2*/



std::string debugInfo(PhysicsShapes& physicsShapes){
  std::string value = "debug physics\n";
  value += std::string("blocks, size = ") + std::to_string(physicsShapes.blocks.size()) + "\n";
  value += std::string("special shapes num types = ") + std::to_string(physicsShapes.shapes.size()) + "\n";
  for (auto &shape : physicsShapes.shapes){
    value += std::string("shape type, size = ") + std::to_string(shape.specialBlocks.size()) + "\n";
  }
  return value;
}

Mesh createOctreeMesh(Octree& octree, std::function<Mesh(MeshData&)> loadMesh){
  std::vector<Vertex> vertices;
  std::vector<OctreeVertex> points = {};

  std::cout << "adding octree start" << std::endl;
  addOctreeLevel(octree, points, glm::vec3(0.f, 0.f, 0.f), octree.rootNode, 1.f, 0, {});

  std::cout << "adding octree end" << std::endl;

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

Mesh* getOctreeMesh(GameObjectOctree& octree){
  return &octree.mesh;
}

std::vector<AutoSerialize> octreeAutoserializer {
  AutoSerializeString {
    .structOffset = offsetof(GameObjectOctree, map),
    .field = "map",
    .defaultValue = "",
  }
};

GameObjectOctree createOctree(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectOctree obj {};
  createAutoSerializeWithTextureLoading((char*)&obj, octreeAutoserializer, attr, util);
  if (obj.map != ""){
    auto mapFilePath = util.pathForModLayer(obj.map);
    auto serializedFileData = readFileOrPackage(mapFilePath);
    std::cout << "serialized data: " << serializedFileData << std::endl;    
    if (serializedFileData == ""){
      obj.octree = unsubdividedOctree;
    }else{
      obj.octree = deserializeOctree(serializedFileData);
    }
  }else{
    obj.octree = unsubdividedOctree;
  }

  obj.mesh = createOctreeMesh(obj.octree, util.loadMesh);
  return obj;
}


Faces getFaces(int x, int y, int z, float size, int subdivisionLevel){
  float adjustedSize = size * glm::pow(0.5f, subdivisionLevel);
  Faces faces {
    .XYClose = adjustedSize * 0.5f,
    .XYFar = adjustedSize * -0.5f,
    .YZLeft = adjustedSize * -0.5f, 
    .YZRight = adjustedSize * 0.5f,
    .XZTop = adjustedSize * 0.5f,
    .XZBottom = adjustedSize * -0.5f,
  };

  float width = faces.YZRight - faces.YZLeft;
  float height = faces.XZTop - faces.XZBottom;
  float depth = faces.XYClose - faces.XYFar;

  faces.center = glm::vec3(adjustedSize * x + (0.5f * width), adjustedSize * y + (0.5f * height), -1.f * (adjustedSize * z + (0.5f * depth)));

  faces.XYClosePoint = faces.center + glm::vec3(0.f, 0.f, faces.XYClose);
  faces.XYFarPoint = faces.center + glm::vec3(0.f, 0.f, faces.XYFar);
  faces.YZLeftPoint = faces.center + glm::vec3(faces.YZLeft, 0.f, 0.f);
  faces.YZRightPoint = faces.center + glm::vec3(faces.YZRight, 0.f, 0.f);
  faces.XZTopPoint = faces.center + glm::vec3(0.f, faces.XZTop, 0.f);
  faces.XZBottomPoint = faces.center + glm::vec3(0.f, faces.XZBottom, 0.f);

  return faces;
}


/* parametric form to solve 
  pos.x + dir.x * t = x
  pos.y + dir.y * t = y
  pos.z + dir.z * t = z
  so if we provide x, y, or z, we can get the other values, 
  since those values are dependent on the value
  howerver, (x,y,z) as fn(t) doesn't necessarily lie on the face,
  but it's the only possible solution at that face
*/
glm::vec3 calculateTForX(glm::vec3 pos, glm::vec3 dir, float x, float* _t){
  float t = (x - pos.x) / dir.x;
  float y = pos.y + (dir.y * t);
  float z = pos.z + (dir.z * t);
  *_t = t;
  return glm::vec3(x, y, z);
}
glm::vec3 calculateTForY(glm::vec3 pos, glm::vec3 dir, float y, float* _t){
  float t = (y - pos.y) / dir.y;
  *_t = t;
  float x = pos.x + (dir.x * t);
  float z = pos.z + (dir.z * t);
  return glm::vec3(x, y, z);
}
glm::vec3 calculateTForZ(glm::vec3 pos, glm::vec3 dir, float z, float* _t){
  float t = (z - pos.z) / dir.z;
  *_t = t;
  float x = pos.x + (dir.x * t);
  float y = pos.y + (dir.y * t);
  return glm::vec3(x, y, z);
}

bool checkIfInCube(Faces& faces, bool checkX, bool checkY, bool checkZ, glm::vec3 point){
  float minX = faces.center.x + faces.YZLeft;
  float maxX = faces.center.x + faces.YZRight;
  float minY = faces.center.y + faces.XZBottom;
  float maxY = faces.center.y + faces.XZTop;
  float minZ = faces.center.z + faces.XYFar;
  float maxZ = faces.center.z + faces.XYClose;
  if (checkX && (point.x > maxX || point.x < minX)){
    return false;
  }
  if (checkY && (point.y > maxY || point.y < minY)){
    return false;
  }
  if (checkZ && (point.z > maxZ || point.z < minZ)){
    return false;
  }
  return true;
}

bool intersectsCube(glm::vec3 fromPos, glm::vec3 toPosDirection, int x, int y, int z, float size, int subdivisionLevel, std::vector<FaceIntersection>& _faceIntersections){
  auto faces = getFaces(x, y, z, size, subdivisionLevel);

  float leftT = 0.f;
  auto intersectionLeft = calculateTForX(fromPos, toPosDirection, faces.YZLeftPoint.x, &leftT);
  bool intersectsLeftFace = checkIfInCube(faces, false, true, true, intersectionLeft);
  if (intersectsLeftFace && leftT >= 0){
    _faceIntersections.push_back(FaceIntersection {
      .face = LEFT,
      .position = intersectionLeft,
    });
  }

  float rightT = 0.f;
  auto intersectionRight = calculateTForX(fromPos, toPosDirection, faces.YZRightPoint.x, &rightT);
  bool intersectsRightFace = checkIfInCube(faces, false, true, true, intersectionRight);
  if (intersectsRightFace && rightT >= 0){
    _faceIntersections.push_back(FaceIntersection {
      .face = RIGHT,
      .position = intersectionRight,
    });
  }

  float topT = 0.f;
  auto intersectionTop = calculateTForY(fromPos, toPosDirection, faces.XZTopPoint.y, &topT);
  bool intersectsTop = checkIfInCube(faces, true, false, true, intersectionTop);
  if (intersectsTop && topT >= 0){
    _faceIntersections.push_back(FaceIntersection {
      .face = UP,
      .position = intersectionTop,
    });
  }

  float bottomT = 0.f;
  auto intersectionBottom = calculateTForY(fromPos, toPosDirection, faces.XZBottomPoint.y, &bottomT);
  bool intersectsBottom = checkIfInCube(faces, true, false, true, intersectionBottom);
  if (intersectsBottom && bottomT >= 0){
    _faceIntersections.push_back(FaceIntersection {
      .face = DOWN,
      .position = intersectionBottom,
    });
  }

  float closeT = 0.f;
  auto intersectionClose = calculateTForZ(fromPos, toPosDirection, faces.XYClosePoint.z, &closeT);
  bool intersectsClose  = checkIfInCube(faces, true, true, false, intersectionClose);
  if (intersectsClose && closeT >= 0){
    _faceIntersections.push_back(FaceIntersection {
      .face = FRONT,
      .position = intersectionClose,
    });
  }

  float farT = 0.f;
  auto intersectionFar = calculateTForZ(fromPos, toPosDirection, faces.XYFarPoint.z, &farT);
  bool intersectsFar  = checkIfInCube(faces, true, true, false, intersectionFar);
  if (intersectsFar && farT >= 0){
    _faceIntersections.push_back(FaceIntersection {
      .face = BACK,
      .position = intersectionFar,
    });
  }

  bool intersectsCube = (intersectsRightFace || intersectsLeftFace || intersectsTop || intersectsBottom || intersectsClose || intersectsFar);
  return intersectsCube; 
}

std::vector<Intersection> subdivisionIntersections(glm::vec3 fromPos, glm::vec3 toPosDirection, float size, int subdivisionLevel, glm::ivec3 offset){
  std::vector<Intersection> intersections;
  for (int x = 0; x < 2; x++){
    for (int y = 0; y < 2; y++){
      for (int z = 0; z < 2; z++){
        // notice that adjacent faces are duplicated here
        std::vector<FaceIntersection> faceIntersections;
        if (intersectsCube(fromPos, toPosDirection, x + offset.x, y + offset.y, z + offset.z, size, subdivisionLevel, faceIntersections)){
          intersections.push_back(Intersection {
            .index = xyzIndexToFlatIndex(glm::ivec3(x, y, z)),
            .faceIntersections = faceIntersections,
          });
        }
      }

    }
  }
  return intersections;
}

void raycastSubdivision(Octree& octree, glm::vec3 fromPos, glm::vec3 toPosDirection, glm::ivec3 offset, int currentSubdivision, int subdivisionDepth, std::vector<RaycastIntersection>& finalIntersections){
  modassert(currentSubdivision <= subdivisionDepth, "current subdivision should not be greater than the target subdivision depth");
  auto intersections = subdivisionIntersections(fromPos, toPosDirection, 1.f, currentSubdivision, offset);

  for (auto &intersection : intersections){
    auto xyzIndex = flatIndexToXYZ(intersection.index);
    if (currentSubdivision == subdivisionDepth){
        finalIntersections.push_back(RaycastIntersection {
          .index = intersection.index,
          .blockOffset = offset,
          .faceIntersections = intersection.faceIntersections,
        });
        continue;
    }
    glm::ivec3 newOffset = (offset + xyzIndex) * 2;
    raycastSubdivision(octree, fromPos, toPosDirection, newOffset, currentSubdivision + 1, subdivisionDepth, finalIntersections); 
  }
}

RaycastResult doRaycast(Octree& octree, glm::vec3 fromPos, glm::vec3 toPosDirection, int subdivisionDepth){
  modassert(subdivisionDepth >= 1, "subdivisionDepth must be >= 1");
  std::vector<RaycastIntersection> finalIntersections;
  raycastSubdivision(octree, fromPos, toPosDirection, glm::ivec3(0, 0, 0), 1, subdivisionDepth, finalIntersections);

  return RaycastResult {
    .fromPos = fromPos,
    .toPosDirection = toPosDirection,
    .subdivisionDepth = subdivisionDepth,
    .intersections = finalIntersections,
  };
}

std::optional<ClosestIntersection> getClosestIntersection(RaycastResult& raycastResult){
  if (raycastResult.intersections.size() == 0){
    return std::nullopt;
  }
  std::optional<ClosestIntersection> closestIntersection = std::nullopt;
  for (int i = 0; i < raycastResult.intersections.size(); i++){
    for (auto &faceIntersection : raycastResult.intersections.at(i).faceIntersections){
      if (!closestIntersection.has_value() ||
        glm::distance(faceIntersection.position, raycastResult.fromPos) < glm::distance(closestIntersection.value().position, raycastResult.fromPos) > 0){
        closestIntersection = ClosestIntersection {
          .face = faceIntersection.face,
          .position = faceIntersection.position,
          .xyzIndex = flatIndexToXYZ(raycastResult.intersections.at(i).index) + raycastResult.intersections.at(i).blockOffset,
          .subdivisionDepth = raycastResult.subdivisionDepth,
        };
      }
    }
  }
  return closestIntersection;
}

bool cellFilledIn(Octree& octree, RaycastIntersection& intersection, int subdivision){
  auto xyzIndex = flatIndexToXYZ(intersection.index) + intersection.blockOffset;;
  auto path = octreePath(xyzIndex.x, xyzIndex.y, xyzIndex.z, subdivision);
  //std::cout << "octreepath: ";
  //for (auto index : path){
  //  std::cout << "(" << index.x << ", " << index.y << ", " << index.z << "), ";
  //}
  //std::cout << std::endl;

  OctreeDivision* currentSubdivision = &octree.rootNode;
  for (auto xyzIndex : path){
    if (currentSubdivision -> divisions.size() == 0){
      return currentSubdivision -> fill == FILL_FULL;
    }
    auto index = xyzIndexToFlatIndex(xyzIndex);
    currentSubdivision = &currentSubdivision -> divisions.at(index);
  }
  return currentSubdivision -> fill == FILL_FULL;
}

RaycastResult filterFilledInCells(Octree& octree, RaycastResult& raycastResult){
  RaycastResult filteredRaycastResult {
    .fromPos = raycastResult.fromPos,
    .toPosDirection = raycastResult.toPosDirection,
    .subdivisionDepth = raycastResult.subdivisionDepth,
  };

  std::vector<RaycastIntersection> intersections;
  for (auto& intersection : raycastResult.intersections){
    if (cellFilledIn(octree, intersection, raycastResult.subdivisionDepth)){
      intersections.push_back(intersection);
    }
  }
  filteredRaycastResult.intersections = intersections;

  return filteredRaycastResult;
}

void setSelection(glm::ivec3 selection1, glm::ivec3 selection2, OctreeSelectionFace face){
  //    auto diff = closestRaycast.value().xyzIndex - newRaycast.value().xyzIndex;
  auto minXIndex = selection1.x < selection2.x ? selection1.x : selection2.x;
  auto minYIndex = selection1.y < selection2.y ? selection1.y : selection2.y;
  auto minZIndex = selection1.z < selection2.z ? selection1.z : selection2.z;

  auto maxXIndex = selection1.x > selection2.x ? selection1.x : selection2.x;
  auto maxYIndex = selection1.y > selection2.y ? selection1.y : selection2.y;
  auto maxZIndex = selection1.z > selection2.z ? selection1.z : selection2.z;

  selectedIndex = glm::ivec3(minXIndex, minYIndex, minZIndex);
  selectionDim = glm::ivec3(maxXIndex - minXIndex + 1, maxYIndex - minYIndex + 1, maxZIndex - minZIndex + 1);
  editorOrientation = face;

  std::cout << "octree raycast selection1: " << print(selection1) << ", 2 = " << print(selection2) << std::endl;
}

void handleOctreeRaycast(Octree& octree, glm::vec3 fromPos, glm::vec3 toPosDirection, bool secondarySelection, objid id){
  auto octreeRaycast = doRaycast(octree, fromPos, toPosDirection, subdivisionLevel);
  raycastResult = octreeRaycast;

  RaycastResult filteredCells = filterFilledInCells(octree, raycastResult.value());
  if (!secondarySelection){
    closestRaycast = getClosestIntersection(filteredCells);
    if (!closestRaycast.has_value()){
      return;
    }
    selectedIndex = closestRaycast.value().xyzIndex;
    setSelection(closestRaycast.value().xyzIndex, closestRaycast.value().xyzIndex, closestRaycast.value().face);
  }else if (closestRaycast.has_value()) {
    // should change selecteddim here
    auto newRaycast = getClosestIntersection(filteredCells);
    if (!newRaycast.has_value()){
      return;
    }
    setSelection(closestRaycast.value().xyzIndex, newRaycast.value().xyzIndex, closestRaycast.value().face);
  }
}

bool drawRaycastLine = false;
bool drawAllSelectedBlocks = false;
void drawOctreeSelectionGrid(Octree& octree, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine2, glm::mat4 modelMatrix){
  std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLineModel = [drawLine2, &modelMatrix](glm::vec3 fromPos, glm::vec3 toPos, glm::vec4 color) -> void {
    glm::vec4 fromPosVec4(fromPos.x, fromPos.y, fromPos.z, 1.f);
    glm::vec4 toPosVec4(toPos.x, toPos.y, toPos.z, 1.f);
    auto transformedFrom = modelMatrix * fromPosVec4;
    auto transformedTo = modelMatrix * toPosVec4;

    drawLine2(
      glm::vec3(transformedFrom.x, transformedFrom.y, transformedFrom.z), 
      glm::vec3(transformedTo.x, transformedTo.y, transformedTo.z), 
      color
    );
  };

  if (selectedIndex.has_value()){
    drawLineModel(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 5.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 1.f));
    drawGridSelectionXY(selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, subdivisionLevel, 1.f,  drawLineModel, std::nullopt);
    if (selectionDim.value().z > 0){
      drawGridSelectionXY(selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z + selectionDim.value().z, selectionDim.value().x, selectionDim.value().y, subdivisionLevel, 1.f,  drawLineModel, std::nullopt);
    }
    if (line.has_value()){
      drawLineModel(line.value().fromPos, line.value().toPos, glm::vec4(1.f, 0.f, 0.f, 1.f));
    }
  }

  //auto faces = getFaces(selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, testOctree.size, subdivisionLevel);
  //visualizeFaces(faces, drawLine);

  if (raycastResult.has_value()){
    auto dirOffset = glm::normalize(raycastResult.value().toPosDirection);
    dirOffset.x *= 20;
    dirOffset.y *= 20;
    dirOffset.z *= 20;
    drawLineModel(raycastResult.value().fromPos, raycastResult.value().fromPos + dirOffset, glm::vec4(1.f, 1.f, 1.f, 1.f));

    if (drawAllSelectedBlocks){
      for (auto intersection : raycastResult.value().intersections){
        auto xyzIndex = flatIndexToXYZ(intersection.index);
        drawGridSelectionCube(xyzIndex.x + intersection.blockOffset.x, xyzIndex.y + intersection.blockOffset.y, xyzIndex.z + intersection.blockOffset.z, 1, 1, 1, raycastResult.value().subdivisionDepth, 1.f, drawLineModel, std::nullopt);    
        // draw hit marker on the point
        for (auto &face : intersection.faceIntersections){
          drawLineModel(face.position, face.position + glm::vec3(0.f, 0.2f, 0.f), glm::vec4(0.f, 1.f, 0.f, 1.f));
        }
      }
    }
  }

  if (false && closestRaycast.has_value()){
    drawGridSelectionCube(closestRaycast.value().xyzIndex.x, closestRaycast.value().xyzIndex.y, closestRaycast.value().xyzIndex.z, 1, 1, 1, closestRaycast.value().subdivisionDepth, 1.f, drawLineModel, std::nullopt);    
    drawLineModel(closestRaycast.value().position, closestRaycast.value().position + glm::vec3(0.f, 0.2f, 0.f), glm::vec4(0.f, 0.f, 1.f, 1.f));
  }

  ////// visualize the physics objects
  //auto physicsShapes = getPhysicsShapes(octree);
  //drawPhysicsShapes(physicsShapes, drawLine2);
}

void makeOctreeCellRamp(Octree& octree, int x, int y, int z, int subdivision, RampDirection rampDirection, float startHeight = 0.f, float endHeight = 1.f, float startDepth = 0.f, float endDepth = 1.f){
  auto octreeDivision = getOctreeSubdivisionIfExists(octree, x, y, z, subdivision);
  if (octreeDivision == NULL){
    return;
  }
  //modassert(octreeDivision, "octreeDivision does not exist");
  octreeDivision -> shape = ShapeRamp {
    .direction = rampDirection,
    .startHeight = startHeight,
    .endHeight = endHeight,
    .startDepth = startDepth,
    .endDepth = endDepth,
  };
  writeOctreeCell(octree,  x, y, z, subdivision, true); // kind of hackey, but just to ensure parents are updated
}

struct RampParams {
  float startHeight;
  float endHeight;
  float startDepth;
  float endDepth;
};

std::optional<RampParams> calculateRampParams(glm::vec2 slope, int x, int y){
  modassert(slope.x / slope.y >= 0, "slope must be positive");
  modassert(x >= 0, "x must be >= 0");
  modassert(y >= 0, "y must be >= 0");
  float startDepth = y * (slope.x / slope.y);
  float endDepth = (y + 1) * (slope.x / slope.y);

  if ((endDepth - x) > 1){
    endDepth = x + 1;
  }

  if ((startDepth - x) < 0){
    startDepth = x;
  }

  float startHeight = startDepth * (slope.y / slope.x);
  float endHeight = endDepth * (slope.y / slope.x); 


  RampParams rampParams {
    .startHeight = startHeight - y,
    .endHeight = endHeight - y,
    .startDepth = startDepth - x,
    .endDepth = endDepth - x,
  };


  std::cout << "calculate ramp: (X,Y): " << x << ", " << y << ", slope = " << print(slope) << " -  startDepth " << startDepth << ", endDepth " << endDepth << ", startHeight = " << startHeight << ", endHeight = " << endHeight << std::endl;
  if (aboutEqual(startHeight, endHeight) || aboutEqual(startDepth, endDepth) || startDepth > endDepth){
    return std::nullopt;
  }

  //std::cout << std::endl;
  return rampParams;
}

void makeOctreeCellRamp(GameObjectOctree& gameobjOctree, Octree& octree, std::function<Mesh(MeshData&)> loadMesh, RampDirection direction){
  float heightNudge = 0.f;
  auto slope = glm::vec2((direction == RAMP_FORWARD || direction == RAMP_BACKWARD) ? selectionDim.value().z : selectionDim.value().x, selectionDim.value().y - heightNudge);
  for (int x = 0; x < selectionDim.value().x; x++){
    for (int y = 0; y < selectionDim.value().y; y++){
      for (int z = 0; z < selectionDim.value().z; z++){
        if (direction == RAMP_RIGHT){
          auto rampParams = calculateRampParams(slope, selectionDim.value().x - x - 1, y);
          if (rampParams.has_value()){
            float startHeight = rampParams.value().startHeight;
            float endHeight = rampParams.value().endHeight;
            float startDepth = rampParams.value().startDepth;
            float endDepth = rampParams.value().endDepth;
            makeOctreeCellRamp(octree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel, direction, startHeight, endHeight, startDepth, endDepth);
          }else{
            writeOctreeCell(octree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel, false);
          }
        }else if (direction == RAMP_LEFT){
          auto rampParams = calculateRampParams(slope, x, y);
          if (rampParams.has_value()){
            float startHeight = rampParams.value().startHeight;
            float endHeight = rampParams.value().endHeight;
            float startDepth = rampParams.value().startDepth;
            float endDepth = rampParams.value().endDepth;
            makeOctreeCellRamp(octree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel, direction, startHeight, endHeight, startDepth, endDepth);
          }else{
            writeOctreeCell(octree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel, false);
          }
        }else if (direction == RAMP_FORWARD){
          auto rampParams = calculateRampParams(slope, z, y);
          if (rampParams.has_value()){
            float startHeight = rampParams.value().startHeight;
            float endHeight = rampParams.value().endHeight;
            float startDepth = rampParams.value().startDepth;
            float endDepth = rampParams.value().endDepth;
            makeOctreeCellRamp(octree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel, direction, startHeight, endHeight, startDepth, endDepth);
          }else{
            writeOctreeCell(octree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel, false);
          }
        }else if (direction == RAMP_BACKWARD){
          auto rampParams = calculateRampParams(slope, selectionDim.value().z - z - 1, y);
          if (rampParams.has_value()){
            float startHeight = rampParams.value().startHeight;
            float endHeight = rampParams.value().endHeight;
            float startDepth = rampParams.value().startDepth;
            float endDepth = rampParams.value().endDepth;
            makeOctreeCellRamp(octree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel, direction, startHeight, endHeight, startDepth, endDepth);
          }else{
            writeOctreeCell(octree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel, false);
          }
        }
      }
    }
  }

  gameobjOctree.mesh = createOctreeMesh(octree, loadMesh);
}

void handleOctreeScroll(GameObjectOctree& gameobjOctree, Octree& octree, bool upDirection, std::function<Mesh(MeshData&)> loadMesh, bool holdingShift, bool holdingCtrl, OctreeDimAxis axis){
  if (holdingShift && !holdingCtrl){
    if (axis == OCTREE_NOAXIS){
       if (upDirection){
         selectedIndex.value().z++;
       }else{
         selectedIndex.value().z--;
       }
     }else if (axis == OCTREE_XAXIS){
       if (upDirection){
         selectedIndex.value().x++;
       }else{
         selectedIndex.value().x--;
       }
     }else if (axis == OCTREE_YAXIS){
       if (upDirection){
         selectedIndex.value().y++;
       }else{
         selectedIndex.value().y--;
       }
     }else if (axis == OCTREE_ZAXIS){
       if (upDirection){
         selectedIndex.value().z++;
       }else{
         selectedIndex.value().z--;
       }
     }
     return;
  }
  std::cout << "octree selected index: " << print(selectedIndex.value()) << std::endl;

  if (holdingShift && holdingCtrl){
    if (axis == OCTREE_NOAXIS){
      if (upDirection){
        selectionDim.value().x++;
        selectionDim.value().y++;
      }else{
        selectionDim.value().x--;
        selectionDim.value().y--;
      }
    }else if (axis == OCTREE_XAXIS){
      if (upDirection){
        selectionDim.value().x++;
      }else{
        selectionDim.value().x--;
      }
    }else if (axis == OCTREE_YAXIS){
      if (upDirection){
        selectionDim.value().y++;
      }else{
        selectionDim.value().y--;
      }
    }else if (axis == OCTREE_ZAXIS){
      if (upDirection){
        selectionDim.value().z++;
      }else{
        selectionDim.value().z--;
      }
    }
    return;
  }

  std::cout << "octree modifiers: " << holdingShift << ", " << holdingCtrl << std::endl;
  
  writeOctreeCellRange(octree, selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, selectionDim.value().z, subdivisionLevel, !holdingCtrl);
  if (editorOrientation == FRONT || editorOrientation == BACK){
    selectedIndex.value().z = selectedIndex.value().z + (upDirection ? -1 : 1);
  }else if (editorOrientation == LEFT || editorOrientation == RIGHT){
    selectedIndex.value().x = selectedIndex.value().x + (upDirection ? 1 : -1);
  }else if (editorOrientation == UP || editorOrientation == DOWN){
    selectedIndex.value().y = selectedIndex.value().y + (upDirection ? 1 : -1);
  }
  if (selectedIndex.value().x < 0){
    selectedIndex.value().x = 0;
  }
  if (selectedIndex.value().y < 0){
    selectedIndex.value().y = 0;
  }
  if (selectedIndex.value().z < 0){
    selectedIndex.value().z = 0;
  }

  //writeOctreeCellRange(testOctree, selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, selectionDim.value().z, subdivisionLevel, true);

  gameobjOctree.mesh = createOctreeMesh(octree, loadMesh);
}

void handleMoveOctreeSelection(OctreeEditorMove direction){
  if (direction == X_POS){
    selectedIndex.value().x++;
  }else if (direction == X_NEG){
    selectedIndex.value().x--;
  }else if (direction == Y_POS){
    selectedIndex.value().y++;
  }else if (direction == Y_NEG){
    selectedIndex.value().y--;
  }else if (direction == Z_POS){
    selectedIndex.value().z++;
  }else if (direction == Z_NEG){
    selectedIndex.value().z--;
  }
}

int getCurrentSubdivisionLevel(){
  return subdivisionLevel;
}
void handleChangeSubdivisionLevel(int newSubdivisionLevel){
  if (newSubdivisionLevel < 1){
    newSubdivisionLevel = 1;
  }
  int subdivisionLevelDifference = newSubdivisionLevel - subdivisionLevel;
  if (subdivisionLevelDifference >= 0){
    int multiplier = glm::pow(2, subdivisionLevelDifference);
    selectedIndex.value().x = selectedIndex.value().x * multiplier;
    selectedIndex.value().y = selectedIndex.value().y * multiplier;
    selectedIndex.value().z = selectedIndex.value().z * multiplier;
  }else{
    int multiplier = glm::pow(2, -1 * subdivisionLevelDifference);
    selectedIndex.value().x = selectedIndex.value().x / multiplier;
    selectedIndex.value().y = selectedIndex.value().y / multiplier;
    selectedIndex.value().z = selectedIndex.value().z / multiplier;
  }

  subdivisionLevel = newSubdivisionLevel;
}

void increaseSelectionSize(int width, int height, int depth){
  selectionDim.value().x+= width;
  selectionDim.value().y+= height;
  selectionDim.value().z+= depth;
  if (selectionDim.value().x < 0){
    selectionDim.value().x = 0;
  }
  if (selectionDim.value().y < 0){
    selectionDim.value().y = 0;
  }
  if (selectionDim.value().z < 0){
    selectionDim.value().z = 0;
  }
}

void insertSelectedOctreeNodes(GameObjectOctree& gameobjOctree, Octree& octree, std::function<Mesh(MeshData&)> loadMesh){
  writeOctreeCellRange(octree, selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, selectionDim.value().z, subdivisionLevel, true);
  gameobjOctree.mesh = createOctreeMesh(octree, loadMesh);
}
void deleteSelectedOctreeNodes(GameObjectOctree& gameobjOctree, Octree& octree, std::function<Mesh(MeshData&)> loadMesh){
  writeOctreeCellRange(octree, selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, selectionDim.value().z, subdivisionLevel, false);
  gameobjOctree.mesh = createOctreeMesh(octree, loadMesh);
}

void writeOctreeTexture(GameObjectOctree& gameobjOctree, Octree& octree, std::function<Mesh(MeshData&)> loadMesh, bool unitTexture, TextureOrientation texOrientation){
  int xTileDim = selectionDim.value().x;
  int yTileDim = selectionDim.value().y;

  if (editorOrientation == FRONT){
    // do nothing
  }else if (editorOrientation == LEFT || editorOrientation == RIGHT){
    xTileDim = selectionDim.value().z;
    yTileDim = selectionDim.value().y;
  }else if (editorOrientation == UP || editorOrientation == DOWN){
    xTileDim = selectionDim.value().x;
    yTileDim = selectionDim.value().z;
  }

  for (int x = 0; x < xTileDim; x++){
    for (int y = 0; y < yTileDim; y++){
      int effectiveX = (editorOrientation == LEFT) ? (xTileDim - x - 1) : x;
      int effectiveY = (editorOrientation == DOWN) ? (yTileDim - y - 1) : y;

      if (editorOrientation == BACK){
        effectiveX = xTileDim - x - 1;
      }

      glm::vec3 divisionOffset(0, 0, 0);
      if (editorOrientation == FRONT || editorOrientation == BACK){
        divisionOffset.x = effectiveX;
        divisionOffset.y = effectiveY;
      }else if (editorOrientation == LEFT || editorOrientation == RIGHT){
        divisionOffset.z = effectiveX;
        divisionOffset.y = effectiveY;
      }else if (editorOrientation == UP || editorOrientation == DOWN){
        divisionOffset.x = effectiveX;
        divisionOffset.z = effectiveY;
      }

      glm::ivec3 subIndex(selectedIndex.value().x + divisionOffset.x, selectedIndex.value().y + divisionOffset.y, selectedIndex.value().z + divisionOffset.z);
      auto octreeDivision = getOctreeSubdivisionIfExists(octree, subIndex.x, subIndex.y, subIndex.z, subdivisionLevel);
      if (octreeDivision){
        auto index = textureIndex(editorOrientation);
        if (octreeDivision -> faces.size() == 0){
          octreeDivision -> faces = defaultTextureCoords;
        }

        if (unitTexture){
          octreeDivision -> faces.at(index) = texCoords(selectedTexture, texOrientation);
        }else{

          std::cout << "write octree texture: index = " <<  print(subIndex) << ", xTileDim = " << xTileDim << ", yTileDim = " << yTileDim << ", x = " << x << ", y = " << y << std::endl;
          octreeDivision -> faces.at(index) = texCoords(selectedTexture, texOrientation, xTileDim, yTileDim, x , y);
        }
      }
    }
  }

  gameobjOctree.mesh = createOctreeMesh(octree, loadMesh);
}

int getOctreeTextureId(){
  return selectedTexture;
}
void setOctreeTextureId(int textureId){
  if (textureId < 0){
    textureId = 0;
  }
  if (textureId >= atlasDimensions.value().textureNames.size()){
    textureId = atlasDimensions.value().textureNames.size() - 1;
  }
  selectedTexture = textureId;
}

std::vector<std::pair<std::string, std::string>> serializeOctree(GameObjectOctree& obj, ObjectSerializeUtil& util){
//  auto serializedData = serializeVoxelState(obj.voxel, util.textureName);
//
//  pairs.push_back(std::pair<std::string, std::string>("from", serializedData.voxelState));
//  if (serializedData.textureState != ""){
//    pairs.push_back(std::pair<std::string, std::string>("fromtextures", serializedData.textureState));
//  }
  saveOctree(obj, util.saveFile);
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&obj, octreeAutoserializer, pairs);
  return pairs;
}

void loadOctree(GameObjectOctree& octree, std::function<std::string(std::string)> loadFile, std::function<Mesh(MeshData&)> loadMesh){
  modlog("octree", "loading");
  auto serializedData = readFileOrPackage(octree.map);
  octree.octree = deserializeOctree(serializedData);
  octree.mesh = createOctreeMesh(octree.octree, loadMesh);
}

void saveOctree(GameObjectOctree& octree, std::function<void(std::string, std::string&)> saveFile){
  modlog("octree", "saving");
  auto serializedData = serializeOctree(octree.octree);
  saveFile(octree.map, serializedData);
}

void setSelectedOctreeId(std::optional<objid> id){
  selectedOctreeId = id;
}
std::optional<objid> getSelectedOctreeId(){
  return selectedOctreeId;
}

std::optional<AttributeValuePtr> getOctreeAttribute(GameObjectOctree& obj, const char* field){
  //modassert(false, "getOctreeAttribute not yet implemented");
  return std::nullopt;
}