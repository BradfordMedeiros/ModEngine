#include "./obj_octree.h"

std::vector<AutoSerialize> octreeAutoserializer {};


struct FaceTexture {
  //const char* texture;
  glm::vec2 texCoordsTopLeft;
  glm::vec2 texCoordsTopRight;
  glm::vec2 texCoordsBottomLeft;
  glm::vec2 texCoordsBottomRight;
};

struct OctreeDivision {
  // -x +y -z 
  // +x +y -z
  // -x +y +z
  // +x +y +z
  // -x -y -z 
  // +x -y -z
  // -x -y +z
  // +x -y +z
  bool filled;
  std::vector<FaceTexture> faces;
  std::vector<OctreeDivision> divisions;
};

struct Octree {
  double size;
  OctreeDivision rootNode;
};

enum OctreeSelectionFace { FRONT, BACK, LEFT, RIGHT, UP, DOWN };
std::optional<glm::ivec3> selectedIndex = glm::ivec3(1, 0, 0);
std::optional<glm::ivec3> selectionDim = glm::ivec3(1, 1, 0);
OctreeSelectionFace editorOrientation = FRONT;
int selectedTexture = 0;

std::optional<Line> line = std::nullopt;
int subdivisionLevel = 2;

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

struct Faces {
  float XYClose;
  glm::vec3 XYClosePoint;
  float XYFar;
  glm::vec3 XYFarPoint;
  float YZLeft;
  glm::vec3 YZLeftPoint;
  float YZRight;
  glm::vec3 YZRightPoint;
  float XZTop;
  glm::vec3 XZTopPoint;
  float XZBottom;
  glm::vec3 XZBottomPoint;
  glm::vec3 center;
};

struct AtlasDimensions {
  int numTexturesWide;
  int numTexturesHeight;
  int totalTextures;
};
AtlasDimensions atlasDimensions {
  .numTexturesWide = 3,
  .numTexturesHeight = 3,
  .totalTextures = 5,
};


/*
5
1 [0 0 1 1 [0 0 0 1 0 0 0 0] 0 0 1] 1 1 1 1 1 1]
*/

// This could easily be optimized by saving this in binary form instead
// human readable, at least for now, seems nice
std::string serializeOctreeDivision(OctreeDivision& octreeDivision){
  if (octreeDivision.divisions.size() != 0){
    std::string str = "[ ";
    modassert(octreeDivision.divisions.size() == 8, "serialization - unexpected # of octree divisions");
    for (int i = 0; i < octreeDivision.divisions.size(); i++){
      auto value = serializeOctreeDivision(octreeDivision.divisions.at(i));
      str += value + " ";
    }
    str += "]";
    return str;
  }
  return octreeDivision.filled ? "1" : "0";
}
std::string serializeOctree(Octree& octree){
  std::string str = std::to_string(octree.size) + "\n";
  str += serializeOctreeDivision(octree.rootNode);
  return str;
}

std::vector<std::string> splitBrackets(std::string& value){
  int numBrackets = 0;
  std::vector<std::string> values;
  std::string currString = "";
  for (int i = 0; i < value.size(); i++){
    if (value.at(i) == '['){
      numBrackets++;
    }else if (value.at(i) == ']'){
      numBrackets--;
    }
 
    currString += value.at(i);
    if (numBrackets == 0){
      if (currString != " "){
        values.push_back(currString);
      }
      currString = "";
    }
  }
  if (currString.size() > 0){
    if (currString != " "){
      values.push_back(currString);
    }
  }
  modassert(numBrackets == 0, std::string("invalid balancing to brackets: ") + value + ", " + std::to_string(numBrackets));
  return values;
}

OctreeDivision deserializeOctreeDivision(std::string value){
  value = trim(value);
  bool inBrackets = value.size() >= 2 && value.at(0) == '[' && value.at(value.size() -1) == ']';

  if (inBrackets){
    auto withoutBrackets = value.substr(1, value.size() - 2);
    auto splitValues = splitBrackets(withoutBrackets);
    std::vector<OctreeDivision> octreeDivisions;
    for (auto &splitValue : splitValues){
      modassert(splitValue.size() > 0, "split value should not be 0 length");
      octreeDivisions.push_back(deserializeOctreeDivision(splitValue));
    }
    modassert(octreeDivisions.size() == 8, std::string("invalid division size, got: " + std::to_string(octreeDivisions.size())));
    return OctreeDivision {
      .filled = false,
      .divisions = octreeDivisions,
    };
  }
  modassert(value.size() >= 1 && (value.at(0) == '0' || value.at(0) == '1'), std::string("invalid value type, got: ") + value + ", size=  " + std::to_string(value.size()));
  auto filled = value.at(0) == '1';
  return OctreeDivision {
    .filled = filled,
    .faces = {},
    .divisions = {},
  };
}
Octree deserializeOctree(std::string& value){
  auto lines = split(value, '\n');
  modassert(lines.size() == 2, "invalid line size");
  float size = std::atof(lines.at(0).c_str());
  return Octree  {
    .size = size,
    .rootNode = deserializeOctreeDivision(lines.at(1)),
  };
}

FaceTexture texCoords(int imageIndex, TextureOrientation texOrientation = TEXTURE_UP, glm::vec2 multiplier = glm::vec2(1.f, 1.f), glm::ivec2 offset = glm::ivec2(0, 0)){
  float atlasWide = 1.f / atlasDimensions.numTexturesWide;
  float atlasHeight = 1.f / atlasDimensions.numTexturesHeight;
  float texWide = multiplier.x * atlasWide;
  float texHeight = multiplier.y * atlasHeight;

  int xIndex = imageIndex % atlasDimensions.numTexturesWide;
  int yIndex = imageIndex / atlasDimensions.numTexturesWide;

  float xMin = xIndex * atlasWide + (texWide * offset.x);
  float xMax = xMin + texWide;
  float yMin = yIndex * atlasHeight + (texHeight * offset.y);
  float yMax = yMin + texHeight;

  if (texOrientation == TEXTURE_DOWN){
    return FaceTexture {
      .texCoordsTopLeft = glm::vec2(xMin, yMin),
      .texCoordsTopRight = glm::vec2(xMax, yMin),
      .texCoordsBottomLeft = glm::vec2(xMin, yMax),
      .texCoordsBottomRight = glm::vec2(xMax, yMax),
    };
  }
  if (texOrientation == TEXTURE_RIGHT){
    return FaceTexture {
      .texCoordsTopLeft = glm::vec2(xMin, yMin),
      .texCoordsTopRight = glm::vec2(xMin, yMax),
      .texCoordsBottomLeft = glm::vec2(xMax, yMin),
      .texCoordsBottomRight = glm::vec2(xMax, yMax),
    };
  }
  if (texOrientation == TEXTURE_LEFT){
    return FaceTexture {
      .texCoordsTopLeft = glm::vec2(xMax, yMax),
      .texCoordsTopRight = glm::vec2(xMax, yMin),
      .texCoordsBottomLeft = glm::vec2(xMin, yMax),
      .texCoordsBottomRight = glm::vec2(xMin, yMin),
    };
  }
  return FaceTexture {
    .texCoordsTopLeft = glm::vec2(xMin, yMax),
    .texCoordsTopRight = glm::vec2(xMax, yMax),
    .texCoordsBottomLeft = glm::vec2(xMin, yMin),
    .texCoordsBottomRight = glm::vec2(xMax, yMin),
  };
}

std::vector<FaceTexture> defaultTextureCoords = {
  texCoords(2),
  texCoords(2),
  texCoords(2),
  texCoords(2),
  texCoords(2),
  texCoords(2),
};

Octree unsubdividedOctree {
  .size = 1.f,
  .rootNode = OctreeDivision {
    .filled = true,
    .faces = defaultTextureCoords,
    .divisions = {},
  },
};
Octree subdividedOne {
  .size = 10.f,
  .rootNode = OctreeDivision {
    .filled = true,
    .faces = {},
    .divisions = {
      OctreeDivision { 
        .filled = true,
      },
      OctreeDivision { .filled = false },
      OctreeDivision { 
        .filled = false,
        .faces = {},
        .divisions = {
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = false },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = false },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
        },
      },
      OctreeDivision { .filled = false },
      OctreeDivision { .filled = true },
      OctreeDivision { .filled = false },
      OctreeDivision { .filled = true },
      OctreeDivision { .filled = true },
    },
  },
};

Octree testOctree = unsubdividedOctree;


glm::ivec3 indexForSubdivision(int x, int y, int z, int sourceSubdivision, int targetSubdivision){
  if (sourceSubdivision < targetSubdivision){ // same formula as other case, just being mindful of integer division
    int numCells = glm::pow(2, targetSubdivision - sourceSubdivision);
    return glm::ivec3(x * numCells, y * numCells, z * numCells);
  }
  int numCells = glm::pow(2, sourceSubdivision - targetSubdivision);
  return glm::ivec3(x / numCells, y / numCells, z / numCells);
}
glm::ivec3 localIndexForSubdivision(int x, int y, int z, int sourceSubdivision, int targetSubdivision){
  auto indexs = indexForSubdivision(x, y, z, sourceSubdivision, targetSubdivision);
  return glm::ivec3(indexs.x % 2, indexs.y % 2, indexs.z % 2);
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

std::vector<glm::ivec3> octreePath(int x, int y, int z, int subdivision){
  std::vector<glm::ivec3> path;
  for (int currentSubdivision = 1; currentSubdivision <= subdivision; currentSubdivision++){
    auto indexs = localIndexForSubdivision(x, y, z, subdivision, currentSubdivision);
    std::cout << "octree current subdivision index: " << print(indexs) << std::endl;
    path.push_back(indexs);
  }
  return path;
}

void writeOctreeCell(Octree& octree, int x, int y, int z, int subdivision, bool filled){
  OctreeDivision* octreeSubdivision = &octree.rootNode;
  auto path = octreePath(x, y, z, subdivision);

  std::cout << "octree path: [";
  for (auto &coord : path){
    std::cout << print(coord) << ", ";
  }
  std::cout << "]" << std::endl;

  for (int i = 0; i < path.size(); i++){
    // todo -> if the subdivision isn't made here, should make it here
    if (octreeSubdivision -> divisions.size() == 0){
      bool defaultFill = octreeSubdivision -> filled;
      octreeSubdivision -> divisions = {
        OctreeDivision { .filled = defaultFill, .faces = octreeSubdivision -> faces },
        OctreeDivision { .filled = defaultFill, .faces = octreeSubdivision -> faces },
        OctreeDivision { .filled = defaultFill, .faces = octreeSubdivision -> faces },
        OctreeDivision { .filled = defaultFill, .faces = octreeSubdivision -> faces },
        OctreeDivision { .filled = defaultFill, .faces = octreeSubdivision -> faces },
        OctreeDivision { .filled = defaultFill, .faces = octreeSubdivision -> faces },
        OctreeDivision { .filled = defaultFill, .faces = octreeSubdivision -> faces },
        OctreeDivision { .filled = defaultFill, .faces = octreeSubdivision -> faces },
      };
    } 
    // check if all filled, then set the divsions = {}, and filled = true
    octreeSubdivision = &octreeSubdivision -> divisions.at(xyzIndexToFlatIndex(path.at(i)));
  }
  octreeSubdivision -> filled = filled;
  octreeSubdivision -> divisions = {};
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

Vertex createVertex2(glm::vec3 position, glm::vec2 texCoords, glm::vec3 normal){
  Vertex vertex {
    .position = position,
    .normal = normal,
    //.glm::vec3 tangent;

    .texCoords = texCoords,
  };
  for (int i = 0; i < NUM_BONES_PER_VERTEX; i++){
    vertex.boneIndexes[i] = 0;
    vertex.boneWeights[i] = 0;
  }
  return vertex;
}

// enum OctreeSelectionFace { FRONT, BACK, LEFT, RIGHT, UP, DOWN };


struct OctreeVertex {
  glm::vec3 position;
  glm::vec2 coord;
};
void addCubePoints(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces){
  if (faces -> size() == 0){
    faces = &defaultTextureCoords;
  }

  // front plane
  FaceTexture& frontFace =  faces -> at(0);
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset, .coord = frontFace.texCoordsBottomRight });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size, 0.f) + offset, .coord = frontFace.texCoordsTopLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,  .coord = frontFace.texCoordsBottomLeft  });

  points.push_back(OctreeVertex { .position = glm::vec3(size, size, 0.f) + offset, .coord = frontFace.texCoordsTopRight });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size, 0.f) + offset,  .coord = frontFace.texCoordsTopLeft  });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,  .coord = frontFace.texCoordsBottomRight  });

  // back plane
  FaceTexture& backFace =  faces -> at(1);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + offset,  .coord = backFace.texCoordsBottomLeft   });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size, -size) + offset, .coord = backFace.texCoordsTopLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + offset, .coord = glm::vec2(backFace.texCoordsBottomRight.x, backFace.texCoordsBottomRight.y) });

  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + offset,  .coord = glm::vec2(backFace.texCoordsBottomRight.x, backFace.texCoordsBottomRight.y)  });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size, -size) + offset,  .coord = backFace.texCoordsTopLeft   });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size, -size) + offset, .coord = glm::vec2(backFace.texCoordsTopRight.x, backFace.texCoordsTopRight.y) });

  // left plane
  FaceTexture& leftFace =  faces -> at(2);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size, -size) + offset, .coord = leftFace.texCoordsTopLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + offset,  .coord = leftFace.texCoordsBottomLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,    .coord = leftFace.texCoordsBottomRight });

  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size, 0.f) + offset,   .coord = leftFace.texCoordsTopRight });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size, -size) + offset, .coord = leftFace.texCoordsTopLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,    .coord = leftFace.texCoordsBottomRight });


  // right plane
  FaceTexture& rightFace =  faces -> at(3);
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,    .coord = rightFace.texCoordsBottomLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + offset,  .coord = rightFace.texCoordsBottomRight  });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size, -size) + offset, .coord = rightFace.texCoordsTopRight });

  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,    .coord = rightFace.texCoordsBottomLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size, -size) + offset, .coord = rightFace.texCoordsTopRight });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size, 0.f) + offset,   .coord = rightFace.texCoordsTopLeft });

  // top plane
  FaceTexture& topFace =  faces -> at(4);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size, 0.f) + offset,   .coord = topFace.texCoordsBottomLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size, 0.f) + offset,  .coord = topFace.texCoordsBottomRight  });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size, -size) + offset, .coord = topFace.texCoordsTopLeft });

  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size, -size) + offset,  .coord = topFace.texCoordsTopLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size, 0.f) + offset,   .coord = topFace.texCoordsBottomRight  });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size, -size) + offset, .coord = topFace.texCoordsTopRight });
  

  //bottom plane
  FaceTexture& bottomFace =  faces -> at(5);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + offset, .coord = bottomFace.texCoordsBottomLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,  .coord = bottomFace.texCoordsTopRight });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,   .coord = bottomFace.texCoordsTopLeft });

  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + offset, .coord = bottomFace.texCoordsBottomRight  });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,   .coord = bottomFace.texCoordsTopRight });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + offset,  .coord = bottomFace.texCoordsBottomLeft });
}

void addOctreeLevel(std::vector<OctreeVertex>& points, glm::vec3 rootPos, OctreeDivision& octreeDivision, float size, int subdivisionLevel){
  std::cout << "addOctreeLevel: " << size << std::endl;
  if (octreeDivision.divisions.size() > 0){
    float subdivisionSize = size * 0.5f;

    // -x +y -z 
    addOctreeLevel(points, rootPos + glm::vec3(0.f, subdivisionSize, -subdivisionSize), octreeDivision.divisions.at(0), subdivisionSize, subdivisionLevel + 1);

    // +x +y -z
    addOctreeLevel(points, rootPos + glm::vec3(subdivisionSize, subdivisionSize, -subdivisionSize), octreeDivision.divisions.at(1), subdivisionSize, subdivisionLevel + 1);

    // -x +y +z
    addOctreeLevel(points, rootPos + glm::vec3(0.f, subdivisionSize, 0.f), octreeDivision.divisions.at(2), subdivisionSize, subdivisionLevel + 1);

    // +x +y +z
    addOctreeLevel(points, rootPos + glm::vec3(subdivisionSize, subdivisionSize, 0.f), octreeDivision.divisions.at(3), subdivisionSize, subdivisionLevel + 1);

    // -x -y -z 
    addOctreeLevel(points, rootPos + glm::vec3(0.f, 0.f, -subdivisionSize), octreeDivision.divisions.at(4), subdivisionSize, subdivisionLevel + 1);

    // +x -y -z
    addOctreeLevel(points, rootPos + glm::vec3(subdivisionSize, 0.f, -subdivisionSize), octreeDivision.divisions.at(5), subdivisionSize, subdivisionLevel + 1);

    // -x -y +z
    addOctreeLevel(points, rootPos + glm::vec3(0.f, 0.f, 0.f), octreeDivision.divisions.at(6), subdivisionSize, subdivisionLevel + 1);

    // +x -y +z
    addOctreeLevel(points, rootPos + glm::vec3(subdivisionSize, 0.f, 0.f), octreeDivision.divisions.at(7), subdivisionSize, subdivisionLevel + 1);
  }else if (octreeDivision.filled){
    addCubePoints(points, size, rootPos, &octreeDivision.faces);
  }
}

Mesh createOctreeMesh(std::function<Mesh(MeshData&)> loadMesh){
  std::vector<Vertex> vertices;
  std::vector<OctreeVertex> points = {};

  std::cout << "adding octree start" << std::endl;
  addOctreeLevel(points, glm::vec3(0.f, 0.f, 0.f), testOctree.rootNode, testOctree.size, 0);
  std::cout << "adding octree end" << std::endl;

  for (int i = 0; i < points.size(); i+=3){
    glm::vec3 vec1 = points.at(i).position - points.at(i + 1).position;
    glm::vec3 vec2 = points.at(i).position - points.at(i + 2).position;
    auto normal = glm::cross(vec1, vec2); // think about sign better, i think this is right 
    vertices.push_back(createVertex2(points.at(i).position, points.at(i).coord, normal));  // maybe the tex coords should just be calculated as a ratio to a fix texture
    vertices.push_back(createVertex2(points.at(i + 1).position, points.at(i + 1).coord, normal));
    vertices.push_back(createVertex2(points.at(i + 2).position, points.at(i + 2).coord, normal));
  }
  std::vector<unsigned int> indices;
  for (int i = 0; i < vertices.size(); i++){ // should be able to optimize this better...
    indices.push_back(i);
  }
 
  MeshData meshData {
    .vertices = vertices,
    .indices = indices,
    .diffuseTexturePath = "octree-atlas:main",
    .hasDiffuseTexture = true,
    .emissionTexturePath = "",
    .hasEmissionTexture = false,
    .opacityTexturePath = "",
    .hasOpacityTexture = false,
    .normalTexturePath = "../gameresources/build/textures/metal_vent_worn.normal.jpg",
    .hasNormalTexture = true,
    .boundInfo = getBounds(vertices),
    .bones = {},
  };
  return loadMesh(meshData);
}

Mesh* getOctreeMesh(GameObjectOctree& octree){
  return &octree.mesh;
}

GameObjectOctree createOctree(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectOctree obj {};
  obj.mesh = createOctreeMesh(util.loadMesh);
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
void visualizeFaces(Faces& faces, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
  drawLine(faces.center, faces.YZLeftPoint, glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(faces.center, faces.YZRightPoint, glm::vec4(0.f, 1.f, 0.f, 1.f));

  drawLine(faces.center, faces.XZTopPoint, glm::vec4(0.f, 1.f, 0.f, 1.f));
  drawLine(faces.center, faces.XZBottomPoint, glm::vec4(0.f, 0.f, 1.f, 1.f));

  drawLine(faces.center, faces.XYClosePoint, glm::vec4(1.f, 1.f, 0.f, 1.f));
  drawLine(faces.center, faces.XYFarPoint, glm::vec4(0.f, 1.f, 1.f, 1.f));
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

void raycastSubdivision(glm::vec3 fromPos, glm::vec3 toPosDirection, glm::ivec3 offset, int currentSubdivision, int subdivisionDepth, std::vector<RaycastIntersection>& finalIntersections){
  auto intersections = subdivisionIntersections(fromPos, toPosDirection, testOctree.size, currentSubdivision, offset);
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
    raycastSubdivision(fromPos, toPosDirection, newOffset, currentSubdivision + 1, subdivisionDepth, finalIntersections); 
  }
}

RaycastResult doRaycast(glm::vec3 fromPos, glm::vec3 toPosDirection, int subdivisionDepth){
  std::vector<RaycastIntersection> finalIntersections;
  std::cout << "GET raycast START" << std::endl;

  raycastSubdivision(fromPos, toPosDirection, glm::ivec3(0, 0, 0), 1, subdivisionDepth, finalIntersections);

  std::cout << "GET raycast END" << std::endl;
  std::cout << std::endl << std::endl;

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
  std::cout << "octreepath: ";
  for (auto index : path){
    std::cout << "(" << index.x << ", " << index.y << ", " << index.z << "), ";
  }
  std::cout << std::endl;

  OctreeDivision* currentSubdivision = &octree.rootNode;
  for (auto xyzIndex : path){
    if (currentSubdivision -> divisions.size() == 0){
      return currentSubdivision -> filled;
    }

    auto index = xyzIndexToFlatIndex(xyzIndex);
    currentSubdivision = &currentSubdivision -> divisions.at(index);
    std::cout << "octreepath: " << serializeOctreeDivision(*currentSubdivision) << std::endl;
  }
  return currentSubdivision -> filled;
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

void handleOctreeRaycast(glm::vec3 fromPos, glm::vec3 toPosDirection, bool secondarySelection){
  auto serializedData = serializeOctree(testOctree);
  std::cout << "octree serialization: \n" << serializedData << std::endl;

  Octree octree = deserializeOctree(serializedData);
  std::cout << "octree serialization 2: \n" << serializeOctree(octree) << std::endl;

  auto octreeRaycast = doRaycast(fromPos, toPosDirection, subdivisionLevel);
  raycastResult = octreeRaycast;

  RaycastResult filteredCells = filterFilledInCells(testOctree, raycastResult.value());
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
    setSelection(closestRaycast.value().xyzIndex, newRaycast.value().xyzIndex, closestRaycast.value().face);
  }
}

void drawGridSelectionXY(int x, int y, int z, int numCellsWidth, int numCellsHeight, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, std::optional<OctreeSelectionFace> face){
  float cellSize = size * glm::pow(0.5f, subdivision);

  float offsetX = x * cellSize;
  float offsetY = y * cellSize;
  float offsetZ = -1 * z * cellSize;
  glm::vec3 offset(offsetX, offsetY, offsetZ);

  glm::vec4 color(0.f, 0.f, 1.f, 1.f);
  drawLine(offset + glm::vec3(0.f, 0.f, 0.f), offset + glm::vec3(numCellsWidth * cellSize, 0.f, 0.f), color);
  drawLine(offset + glm::vec3(0.f, 0.f, 0.f), offset + glm::vec3(0.f, numCellsHeight * cellSize, 0.f), color);
  drawLine(offset + glm::vec3(0.f, numCellsHeight * cellSize, 0.f), offset + glm::vec3(numCellsWidth * cellSize, numCellsHeight * cellSize, 0.f), color);
  drawLine(offset + glm::vec3(numCellsWidth * cellSize, 0.f, 0.f), offset + glm::vec3(numCellsWidth * cellSize, numCellsHeight * cellSize, 0.f), color);
}
void drawGridSelectionYZ(int x, int y, int z, int numCellsHeight, int numCellsDepth, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, std::optional<OctreeSelectionFace> face){
  float cellSize = size * glm::pow(0.5f, subdivision);

  float offsetX = x * cellSize;
  float offsetY = y * cellSize;
  float offsetZ = -1 * z * cellSize;
  glm::vec3 offset(offsetX, offsetY, offsetZ);

  glm::vec4 color(0.f, 0.f, 1.f, 1.f);
  drawLine(offset + glm::vec3(0.f, 0.f, 0.f), offset + glm::vec3(0.f, 0.f, -1 * numCellsDepth * cellSize), color);
  drawLine(offset + glm::vec3(0.f, 0.f, 0.f), offset + glm::vec3(0.f, numCellsHeight * cellSize, 0.f), color);
  drawLine(offset + glm::vec3(0.f, numCellsHeight * cellSize, 0.f), offset + glm::vec3(0.f, numCellsHeight * cellSize, -1 * numCellsDepth * cellSize), color);
  drawLine(offset + glm::vec3(0.f, 0.f, -1 * numCellsDepth * cellSize), offset + glm::vec3(0.f, numCellsHeight * cellSize, -1 * numCellsDepth * cellSize), color);
}
void drawGridSelectionCube(int x, int y, int z, int numCellsWidth, int numCellsHeight, int numCellDepth, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, std::optional<OctreeSelectionFace> face){
  drawGridSelectionXY(x, y, z,     1, 1, subdivision, size, drawLine, face);
  drawGridSelectionXY(x, y, z + 1, 1, 1, subdivision, size, drawLine, face);
  drawGridSelectionYZ(x, y, z, 1, 1, subdivision, size, drawLine, face);
  drawGridSelectionYZ(x + 1, y, z, 1, 1, subdivision, size, drawLine, face);
}
void drawOctreeSelectedCell(int x, int y, int z, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
  drawGridSelectionCube(x, y, z, 1, 1, 1, subdivision, size, drawLine, std::nullopt);
}

bool drawAllSelectedBlocks = false;
void drawOctreeSelectionGrid(std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
  if (selectedIndex.has_value()){
    //std::cout << "draw grid, z: " << selectionDim.value().z << std::endl;
    drawGridSelectionXY(selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, subdivisionLevel, testOctree.size,  drawLine, std::nullopt);
    if (selectionDim.value().z > 0){
      drawGridSelectionXY(selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z + selectionDim.value().z, selectionDim.value().x, selectionDim.value().y, subdivisionLevel, testOctree.size,  drawLine, std::nullopt);
    }
    //std::cout << "draw octree" << std::endl;
    if (line.has_value()){
      drawLine(line.value().fromPos, line.value().toPos, glm::vec4(1.f, 0.f, 0.f, 1.f));
    }
  }

  auto faces = getFaces(selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, testOctree.size, subdivisionLevel);
  //visualizeFaces(faces, drawLine);

  if (raycastResult.has_value()){
    auto dirOffset = glm::normalize(raycastResult.value().toPosDirection);
    dirOffset.x *= 20;
    dirOffset.y *= 20;
    dirOffset.z *= 20;
    drawLine(raycastResult.value().fromPos, raycastResult.value().fromPos + dirOffset, glm::vec4(1.f, 1.f, 1.f, 1.f));

    if (drawAllSelectedBlocks){
      for (auto intersection : raycastResult.value().intersections){
        auto xyzIndex = flatIndexToXYZ(intersection.index);
        drawGridSelectionCube(xyzIndex.x + intersection.blockOffset.x, xyzIndex.y + intersection.blockOffset.y, xyzIndex.z + intersection.blockOffset.z, 1, 1, 1, raycastResult.value().subdivisionDepth, testOctree.size, drawLine, std::nullopt);    
        // draw hit marker on the point
        for (auto &face : intersection.faceIntersections){
          drawLine(face.position, face.position + glm::vec3(0.f, 0.2f, 0.f), glm::vec4(0.f, 1.f, 0.f, 1.f));
        }
      }
    }
  }

  if (false && closestRaycast.has_value()){
    drawGridSelectionCube(closestRaycast.value().xyzIndex.x, closestRaycast.value().xyzIndex.y, closestRaycast.value().xyzIndex.z, 1, 1, 1, closestRaycast.value().subdivisionDepth, testOctree.size, drawLine, std::nullopt);    
    drawLine(closestRaycast.value().position, closestRaycast.value().position + glm::vec3(0.f, 0.2f, 0.f), glm::vec4(0.f, 0.f, 1.f, 1.f));
  }
}

int getNumOctreeNodes(OctreeDivision& octreeDivision){
  int numNodes = 1;
  for (auto &subdivision : octreeDivision.divisions){
    numNodes += getNumOctreeNodes(subdivision);
  }

  return numNodes;
}

void handleOctreeScroll(GameObjectOctree& octree, bool upDirection, std::function<Mesh(MeshData&)> loadMesh, bool holdingShift, bool holdingCtrl, OctreeDimAxis axis){
  if (holdingShift && !holdingCtrl){
    std::cout << "num octree nodes: " << getNumOctreeNodes(testOctree.rootNode) << std::endl;
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
  
  writeOctreeCellRange(testOctree, selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, selectionDim.value().z, subdivisionLevel, !holdingCtrl);
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

  octree.mesh = createOctreeMesh(loadMesh);
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
  if (newSubdivisionLevel < 0){
    newSubdivisionLevel = 0;
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

void insertSelectedOctreeNodes(GameObjectOctree& octree, std::function<Mesh(MeshData&)> loadMesh){
  writeOctreeCellRange(testOctree, selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, selectionDim.value().z, subdivisionLevel, true);
  octree.mesh = createOctreeMesh(loadMesh);
}
void deleteSelectedOctreeNodes(GameObjectOctree& octree, std::function<Mesh(MeshData&)> loadMesh){
  writeOctreeCellRange(testOctree, selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, selectionDim.value().z, subdivisionLevel, false);
  octree.mesh = createOctreeMesh(loadMesh);
}

void writeOctreeTexture(GameObjectOctree& octree, std::function<Mesh(MeshData&)> loadMesh, bool unitTexture, TextureOrientation texOrientation){
  int xTileDim = selectionDim.value().x;
  int yTileDim = selectionDim.value().y;
  if (editorOrientation == FRONT || editorOrientation == BACK){
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

      auto octreeDivision = getOctreeSubdivisionIfExists(testOctree, selectedIndex.value().x + divisionOffset.x, selectedIndex.value().y + divisionOffset.y, selectedIndex.value().z + divisionOffset.z, subdivisionLevel);
      if (octreeDivision){
        auto index = textureIndex(editorOrientation);
        if (octreeDivision -> faces.size() == 0){
          octreeDivision -> faces = defaultTextureCoords;
        }

        if (unitTexture){
          octreeDivision -> faces.at(index) = texCoords(selectedTexture, texOrientation);
        }else{
          octreeDivision -> faces.at(index) = texCoords(selectedTexture, texOrientation, glm::vec2(1.f / xTileDim, 1.f / yTileDim), glm::vec2(x, y));
        }
      }
    }
  }
  octree.mesh = createOctreeMesh(loadMesh);
}

int getOctreeTextureId(){
  return selectedTexture;
}
void setOctreeTextureId(int textureId){
  if (textureId < 0){
    textureId = 0;
  }
  if (textureId >= atlasDimensions.totalTextures){
    textureId = atlasDimensions.totalTextures - 1;
  }
  selectedTexture = textureId;
}


std::vector<std::pair<std::string, std::string>> serializeOctree(GameObjectOctree& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
//  auto serializedData = serializeVoxelState(obj.voxel, util.textureName);
//
//  pairs.push_back(std::pair<std::string, std::string>("from", serializedData.voxelState));
//  if (serializedData.textureState != ""){
//    pairs.push_back(std::pair<std::string, std::string>("fromtextures", serializedData.textureState));
//  }
  return pairs;
}  //


