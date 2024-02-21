#include "./obj_octree.h"

std::vector<AutoSerialize> octreeAutoserializer {};


struct FaceTexture {
  //const char* texture;
  glm::vec2 texCoordsTopLeft;
  glm::vec2 texCoordsTopRight;
  glm::vec2 texCoordsBottomLeft;
  glm::vec2 texCoordsBottomRight;
};

enum FillType { FILL_FULL, FILL_EMPTY, FILL_MIXED };

struct ShapeBlock { };

enum RampDirection { RAMP_RIGHT, RAMP_LEFT, RAMP_FORWARD, RAMP_BACKWARD };
struct ShapeRamp { 
  RampDirection direction;
  float startHeight;
  float endHeight;
};
typedef std::variant<ShapeBlock, ShapeRamp> OctreeShape;

struct OctreeDivision {
  // -x +y -z 
  // +x +y -z
  // -x +y +z
  // +x +y +z
  // -x -y -z 
  // +x -y -z
  // -x -y +z
  // +x -y +z
  FillType fill;
  OctreeShape shape;
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


std::optional<AtlasDimensions> atlasDimensions = AtlasDimensions {
  .numTexturesWide = 3,
  .numTexturesHeight = 3,
  .totalTextures = 9,
};


void setAtlasDimensions(AtlasDimensions newAtlasDimensions){
  atlasDimensions = newAtlasDimensions;
  modlog("set atlas", std::string("wide = ") + std::to_string(newAtlasDimensions.numTexturesWide) + ", height = " + std::to_string(newAtlasDimensions.numTexturesHeight) + ", total = " + std::to_string(newAtlasDimensions.totalTextures));
}

/*
5
1 [0 0 1 1 [0 0 0 1 0 0 0 0] 0 0 1] 1 1 1 1 1 1]
*/

// This could easily be optimized by saving this in binary form instead
// human readable, at least for now, seems nice
std::string serializeOctreeDivision(OctreeDivision& octreeDivision, std::vector<FaceTexture>& textures){
  if (octreeDivision.divisions.size() != 0){
    std::string str = "[ ";
    modassert(octreeDivision.divisions.size() == 8, "serialization - unexpected # of octree divisions");
    modassert(octreeDivision.fill == FILL_MIXED, "octree divisions, but not mixed filled");
    for (int i = 0; i < octreeDivision.divisions.size(); i++){
      auto value = serializeOctreeDivision(octreeDivision.divisions.at(i), textures);
      str += value + " ";
    }
    str += "]";
    return str;
  }

  modassert(octreeDivision.faces.size() == 6, "serializeOctreeDivision unexpected number of octree faces");
  for (auto &face : octreeDivision.faces){
    textures.push_back(face); 
  }

  return octreeDivision.fill == FILL_FULL ? "1" : "0";
}

std::string serializeTexCoord(FaceTexture& faceTexture){
  std::string value = "";
  value += serializeVec(faceTexture.texCoordsTopLeft) + "|";
  value += serializeVec(faceTexture.texCoordsTopRight) + "|";
  value += serializeVec(faceTexture.texCoordsBottomLeft) + "|";
  value += serializeVec(faceTexture.texCoordsBottomRight);
  return value;
}

std::string serializeOctree(Octree& octree){
  // 2
  // [ 1 1 1 1 [ 1 1 1 1 1 0 1 0 ] [ 1 1 1 1 0 1 0 1 ] [ 1 1 1 1 1 0 1 0 ] [ 1 1 1 1 0 1 0 1 ] ]
  // 0.3 0.3|0.4 0.2|0.3 0.4|0.1 0.1,2,1,2,1,2;1,2,3,1,1,1;1,1,1,1,1,1
  std::vector<FaceTexture> textures;

  std::string str = std::to_string(octree.size) + "\n";
  str += serializeOctreeDivision(octree.rootNode, textures) + "\n";

  std::string textureString = "";
  for (int i = 0; i < textures.size(); i+=6){
    textureString += serializeTexCoord(textures.at(i)) + ",";  // front
    textureString += serializeTexCoord(textures.at(i + 1)) + ",";  // back
    textureString += serializeTexCoord(textures.at(i + 2)) + ",";  // left
    textureString += serializeTexCoord(textures.at(i + 3)) + ",";  // right
    textureString += serializeTexCoord(textures.at(i + 4)) + ",";  // top
    textureString += serializeTexCoord(textures.at(i + 5));  // down

    if ((i + 5) != textures.size() -1){
      textureString += ";";
    }
  }
  str += textureString;

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

  std::cout << "write octree texture inner offset = " <<  print(offset) << ", multiplier = " << print(multiplier) << std::endl;

  float atlasWide = 1.f / atlasDimensions.value().numTexturesWide;
  float atlasHeight = 1.f / atlasDimensions.value().numTexturesHeight;
  float texWide = multiplier.x * atlasWide;
  float texHeight = multiplier.y * atlasHeight;

  int xIndex = imageIndex % atlasDimensions.value().numTexturesWide;
  int yIndex = imageIndex / atlasDimensions.value().numTexturesWide;

  float xMin = xIndex * atlasWide + (texWide * offset.x);
  float xMax = xMin + texWide;
  float yMin = yIndex * atlasHeight + (texHeight * offset.y);
  float yMax = yMin + texHeight;

  if (texOrientation == TEXTURE_DOWN){
    return FaceTexture {
      .texCoordsTopLeft = glm::vec2(xMax, yMin),
      .texCoordsTopRight = glm::vec2(xMin, yMin),
      .texCoordsBottomLeft = glm::vec2(xMax, yMax),
      .texCoordsBottomRight = glm::vec2(xMin, yMax),
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
  texCoords(4),
  texCoords(4),
  texCoords(4),
  texCoords(4),
  texCoords(4),
  texCoords(4),
};

OctreeDivision deserializeOctreeDivision(std::string& value, std::vector<std::vector<FaceTexture>>& textures, int* currentTextureIndex){
  value = trim(value);
  bool inBrackets = value.size() >= 2 && value.at(0) == '[' && value.at(value.size() -1) == ']';

  if (inBrackets){
    auto withoutBrackets = value.substr(1, value.size() - 2);
    auto splitValues = splitBrackets(withoutBrackets);
    std::vector<OctreeDivision> octreeDivisions;
    for (auto &splitValue : splitValues){
      modassert(splitValue.size() > 0, "split value should not be 0 length");
      octreeDivisions.push_back(deserializeOctreeDivision(splitValue, textures, currentTextureIndex));
    }
    modassert(octreeDivisions.size() == 8, std::string("invalid division size, got: " + std::to_string(octreeDivisions.size())));
    return OctreeDivision {
      .fill = FILL_EMPTY,
      .shape = ShapeBlock{},
      .divisions = octreeDivisions,
    };
  }
  modassert(value.size() >= 1 && (value.at(0) == '0' || value.at(0) == '1'), std::string("invalid value type, got: ") + value + ", size=  " + std::to_string(value.size()));
  auto filled = value.at(0) == '1';
  *currentTextureIndex = *currentTextureIndex + 1;
  return OctreeDivision {
    .fill = filled ? FILL_FULL : FILL_EMPTY,
    .shape = ShapeBlock{},
    .faces = textures.at(*currentTextureIndex),
    .divisions = {},
  };
}

std::vector<std::vector<FaceTexture>> deserializeTextures(std::string& values){
  std::vector<std::vector<FaceTexture>> textures;
  auto valuesStr = split(values, ';'); // denotes each octree division
  for (auto &value : valuesStr){
    auto faces = split(value, ',');
    modassert(faces.size() == 6, "invalid face size for textures");
    std::vector<FaceTexture> faceTextures;
    for (auto &face : faces){
      auto textureCoords = split(face, '|');

      modassert(textureCoords.size() == 4, "invalid texture coords size");
      faceTextures.push_back(FaceTexture {
        .texCoordsTopLeft = parseVec2(textureCoords.at(0)),
        .texCoordsTopRight = parseVec2(textureCoords.at(1)),
        .texCoordsBottomLeft = parseVec2(textureCoords.at(2)),
        .texCoordsBottomRight = parseVec2(textureCoords.at(3)),
      });
    }
    textures.push_back(faceTextures);
  }
  // , denotes each face
  // | denotes each texture 

  return textures;
}

Octree deserializeOctree(std::string& value){
  auto lines = split(value, '\n');
  modassert(lines.size() == 3, "invalid line size");
  float size = std::atof(lines.at(0).c_str());

  auto textures = deserializeTextures(lines.at(2));
  int currentTextureIndex = -1;
  return Octree  {
    .size = size,
    .rootNode = deserializeOctreeDivision(lines.at(1), textures, &currentTextureIndex),
  };
}

Octree unsubdividedOctree {
  .size = 1.f,
  .rootNode = OctreeDivision {
    .fill = FILL_FULL,
    .shape = ShapeBlock{},
    .faces = defaultTextureCoords,
    .divisions = {},
  },
};
Octree subdividedOne {
  .size = 1.f,
  .rootNode = OctreeDivision {
    .fill = FILL_MIXED,
    .shape = ShapeBlock{},
    .faces = defaultTextureCoords,
    .divisions = {
      OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
      OctreeDivision { .fill = FILL_EMPTY, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
      OctreeDivision { 
        .fill = FILL_MIXED,
        .faces = defaultTextureCoords,
        .divisions = {
          OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_EMPTY, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_EMPTY, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
          OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
        },
      },
      OctreeDivision { .fill = FILL_EMPTY, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
      OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
      OctreeDivision { .fill = FILL_EMPTY, .shape = ShapeBlock{}, .faces = defaultTextureCoords},
      OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
      OctreeDivision { .fill = FILL_FULL, .shape = ShapeBlock{}, .faces = defaultTextureCoords },
    },
  },
};

Octree testOctree = subdividedOne;


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

struct ValueAndSubdivision {
  glm::ivec3 value;
  int subdivisionLevel;
};

ValueAndSubdivision indexForOctreePath(std::vector<int> path){
  std::cout << "index for octree: ";
  glm::ivec3 sumIndex(0, 0, 0);
  int subdivisionLevel = 0;
  for (int index : path){
    sumIndex = sumIndex * 2;
    sumIndex += flatIndexToXYZ(index);;
    subdivisionLevel++;

    std::cout << index << " = " << print(sumIndex) << ", ";
  }
  std::cout << std::endl;
  return ValueAndSubdivision {
    .value = sumIndex,
    .subdivisionLevel = subdivisionLevel,
  };
}

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

void addCubePointsFront(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f){
  FaceTexture& frontFace =  faces -> at(0);
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset, .coord = frontFace.texCoordsBottomRight });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + offset, .coord = frontFace.texCoordsTopLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,  .coord = frontFace.texCoordsBottomLeft  });

  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + offset, .coord = frontFace.texCoordsTopRight });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + offset,  .coord = frontFace.texCoordsTopLeft  });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,  .coord = frontFace.texCoordsBottomRight  });
}
void addCubePointsBack(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f){
  FaceTexture& backFace =  faces -> at(1);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + offset,  .coord = backFace.texCoordsBottomRight   });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + offset, .coord = backFace.texCoordsTopRight });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + offset, .coord = backFace.texCoordsBottomLeft });

  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + offset,  .coord = backFace.texCoordsBottomLeft  });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + offset,  .coord = backFace.texCoordsTopRight   });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size) + offset, .coord = backFace.texCoordsTopLeft });
}
void addCubePointsLeft(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f){
  FaceTexture& leftFace =  faces -> at(2);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + offset, .coord = leftFace.texCoordsTopLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + offset,  .coord = leftFace.texCoordsBottomLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,    .coord = leftFace.texCoordsBottomRight });

  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + offset,   .coord = leftFace.texCoordsTopRight });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + offset, .coord = leftFace.texCoordsTopLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,    .coord = leftFace.texCoordsBottomRight });
}
void addCubePointsRight(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f){
  FaceTexture& rightFace =  faces -> at(3);
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,    .coord = rightFace.texCoordsBottomLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + offset,  .coord = rightFace.texCoordsBottomRight  });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size) + offset, .coord = rightFace.texCoordsTopRight });

  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,    .coord = rightFace.texCoordsBottomLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size) + offset, .coord = rightFace.texCoordsTopRight });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + offset,   .coord = rightFace.texCoordsTopLeft });
}
void addCubePointsTop(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces, float height = 1.f){
  FaceTexture& topFace =  faces -> at(4);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + offset,   .coord = topFace.texCoordsBottomLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + offset,  .coord = topFace.texCoordsBottomRight  });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + offset, .coord = topFace.texCoordsTopLeft });

  points.push_back(OctreeVertex { .position = glm::vec3(0.f,  size * height, -size) + offset,  .coord = topFace.texCoordsTopLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + offset,   .coord = topFace.texCoordsBottomRight  });
  points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size) + offset, .coord = topFace.texCoordsTopRight });
}
void addCubePointsBottom(std::vector<OctreeVertex>& points, float size, glm::vec3 offset, std::vector<FaceTexture>* faces){
  FaceTexture& bottomFace =  faces -> at(5);
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + offset, .coord = bottomFace.texCoordsBottomLeft });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,  .coord = bottomFace.texCoordsTopRight });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + offset,   .coord = bottomFace.texCoordsTopLeft });

  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + offset, .coord = bottomFace.texCoordsBottomRight  });
  points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + offset,   .coord = bottomFace.texCoordsTopRight });
  points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + offset,  .coord = bottomFace.texCoordsBottomLeft });
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
FillStatus octreeFillStatus(int subdivisionLevel, glm::ivec3 division){
  // this should be looking at the target subdivsiion level, and any level before it 

  auto octreeDivision = getOctreeSubdivisionIfExists2(testOctree, division.x, division.y, division.z, subdivisionLevel);
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
  if (shapeRamp.direction == RAMP_RIGHT){
    float height = shapeRamp.endHeight - shapeRamp.startHeight;
    glm::vec3 rampOffset(0.f, size * shapeRamp.startHeight, 0.f);
    auto fullOffset = offset + rampOffset;

    addCubePointsLeft(points, size, fullOffset, faces, height);
    addCubePointsBottom(points, size, fullOffset, faces);

    FaceTexture& backFace =  faces -> at(1);
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + fullOffset,  .coord = backFace.texCoordsBottomLeft  });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + fullOffset,  .coord = backFace.texCoordsBottomRight   });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + fullOffset, .coord = backFace.texCoordsTopRight });

    FaceTexture& frontFace =  faces -> at(0);
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + fullOffset, .coord = frontFace.texCoordsTopLeft });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + fullOffset,  .coord = frontFace.texCoordsBottomLeft  });
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + fullOffset,  .coord = frontFace.texCoordsBottomRight  });

    FaceTexture& topFace =  faces -> at(4);
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + fullOffset,   .coord = topFace.texCoordsBottomLeft });
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + fullOffset,  .coord = topFace.texCoordsBottomRight  });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + fullOffset, .coord = topFace.texCoordsTopLeft });

    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + fullOffset,  .coord = topFace.texCoordsTopLeft });
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + fullOffset,   .coord = topFace.texCoordsBottomRight  });
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + fullOffset, .coord = topFace.texCoordsTopRight });
  }else if (shapeRamp.direction == RAMP_LEFT){
    float height = shapeRamp.endHeight - shapeRamp.startHeight;
    glm::vec3 rampOffset(0.f, size * shapeRamp.startHeight, 0.f);
    auto fullOffset = offset + rampOffset;

    addCubePointsRight(points, size, fullOffset, faces, height);
    addCubePointsBottom(points, size, fullOffset, faces);

    FaceTexture& backFace =  faces -> at(1);
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + fullOffset,  .coord = backFace.texCoordsBottomLeft  });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + fullOffset,  .coord = backFace.texCoordsBottomRight   });
    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size) + fullOffset, .coord = backFace.texCoordsTopLeft });

    FaceTexture& frontFace =  faces -> at(0);
    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + fullOffset, .coord = frontFace.texCoordsTopRight });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + fullOffset,  .coord = frontFace.texCoordsBottomLeft  });
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + fullOffset,  .coord = frontFace.texCoordsBottomRight  });

    FaceTexture& topFace =  faces -> at(4);
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + fullOffset,   .coord = topFace.texCoordsBottomLeft });
    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + fullOffset,  .coord = topFace.texCoordsBottomRight  });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + fullOffset, .coord = topFace.texCoordsTopLeft });

    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + fullOffset,  .coord = topFace.texCoordsTopLeft });
    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + fullOffset,   .coord = topFace.texCoordsBottomRight  });
    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size) + fullOffset, .coord = topFace.texCoordsTopRight });
  }else if (shapeRamp.direction == RAMP_FORWARD){
    float height = shapeRamp.endHeight - shapeRamp.startHeight;
    glm::vec3 rampOffset(0.f, size * shapeRamp.startHeight, 0.f);
    auto fullOffset = offset + rampOffset;

    addCubePointsBack(points, size, fullOffset, faces, height);
    addCubePointsBottom(points, size, fullOffset, faces);

    FaceTexture& leftFace =  faces -> at(2);
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + fullOffset, .coord = leftFace.texCoordsTopLeft });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + fullOffset,  .coord = leftFace.texCoordsBottomLeft });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + fullOffset,    .coord = leftFace.texCoordsBottomRight });

    FaceTexture& rightFace =  faces -> at(3);
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + fullOffset,    .coord = rightFace.texCoordsBottomLeft });
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + fullOffset,  .coord = rightFace.texCoordsBottomRight  });
    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, -size) + fullOffset, .coord = rightFace.texCoordsTopRight });

    FaceTexture& frontFace =  faces -> at(0);
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + fullOffset, .coord = frontFace.texCoordsBottomRight });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + fullOffset, .coord = frontFace.texCoordsTopLeft });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + fullOffset,  .coord = frontFace.texCoordsBottomLeft  });

    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height,-size) + fullOffset, .coord = frontFace.texCoordsTopRight });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, -size) + fullOffset,  .coord = frontFace.texCoordsTopLeft  });
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + fullOffset,  .coord = frontFace.texCoordsBottomRight  });
  }else if (shapeRamp.direction == RAMP_BACKWARD){
    float height = shapeRamp.endHeight - shapeRamp.startHeight;
    glm::vec3 rampOffset(0.f, size * shapeRamp.startHeight, 0.f);
    auto fullOffset = offset + rampOffset;

    addCubePointsFront(points, size, fullOffset, faces, height);
    addCubePointsBottom(points, size, fullOffset, faces);

    FaceTexture& leftFace =  faces -> at(2);
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + fullOffset,   .coord = leftFace.texCoordsTopRight });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + fullOffset, .coord = leftFace.texCoordsBottomLeft });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, 0.f) + fullOffset,    .coord = leftFace.texCoordsBottomRight });

    FaceTexture& rightFace =  faces -> at(3);
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, 0.f) + fullOffset,    .coord = rightFace.texCoordsBottomLeft });
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + fullOffset, .coord = rightFace.texCoordsBottomRight });
    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + fullOffset,   .coord = rightFace.texCoordsTopLeft });

    FaceTexture& topFace =  faces -> at(4);
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, size * height, 0.f) + fullOffset,   .coord = topFace.texCoordsBottomLeft });
    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + fullOffset,  .coord = topFace.texCoordsBottomRight  });
    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + fullOffset, .coord = topFace.texCoordsTopLeft });

    points.push_back(OctreeVertex { .position = glm::vec3(0.f, 0.f, -size) + fullOffset,  .coord = topFace.texCoordsTopLeft });
    points.push_back(OctreeVertex { .position = glm::vec3(size, size * height, 0.f) + fullOffset,   .coord = topFace.texCoordsBottomRight  });
    points.push_back(OctreeVertex { .position = glm::vec3(size, 0.f, -size) + fullOffset, .coord = topFace.texCoordsTopRight });
  }else{
    modassert(false, "invalid ramp direction");
  }
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

void addOctreeLevel(std::vector<OctreeVertex>& points, glm::vec3 rootPos, OctreeDivision& octreeDivision, float size, int subdivisionLevel, std::vector<int> path){
  std::cout << "addOctreeLevel: " << size << std::endl;
  if (octreeDivision.divisions.size() > 0){
    float subdivisionSize = size * 0.5f; 

    // -x +y -z
    auto topLeftFrontPath = path;
    topLeftFrontPath.push_back(0);
    addOctreeLevel(points, offsetForFlatIndex(0, subdivisionSize, rootPos), octreeDivision.divisions.at(0), subdivisionSize, subdivisionLevel + 1, topLeftFrontPath);

    // +x +y -z
    auto topRightFrontPath = path;
    topRightFrontPath.push_back(1);
    addOctreeLevel(points, offsetForFlatIndex(1, subdivisionSize, rootPos), octreeDivision.divisions.at(1), subdivisionSize, subdivisionLevel + 1, topRightFrontPath);

    // -x +y +z
    auto topLeftBackPath = path;
    topLeftBackPath.push_back(2);
    addOctreeLevel(points, offsetForFlatIndex(2, subdivisionSize, rootPos), octreeDivision.divisions.at(2), subdivisionSize, subdivisionLevel + 1, topLeftBackPath);

    // +x +y +z
    auto topRightBackPath = path;
    topRightBackPath.push_back(3);
    addOctreeLevel(points, offsetForFlatIndex(3, subdivisionSize, rootPos), octreeDivision.divisions.at(3), subdivisionSize, subdivisionLevel + 1, topRightBackPath);

    // -x -y -z
    auto bottomLeftFrontPath = path;
    bottomLeftFrontPath.push_back(4);
    addOctreeLevel(points, offsetForFlatIndex(4, subdivisionSize, rootPos), octreeDivision.divisions.at(4), subdivisionSize, subdivisionLevel + 1, bottomLeftFrontPath);

    // +x -y -z
    auto bottomRightFrontPath = path;
    bottomRightFrontPath.push_back(5);
    addOctreeLevel(points, offsetForFlatIndex(5, subdivisionSize, rootPos), octreeDivision.divisions.at(5), subdivisionSize, subdivisionLevel + 1, bottomRightFrontPath);

    // -x -y +z
    auto bottomLeftBackPath = path;
    bottomLeftBackPath.push_back(6);
    addOctreeLevel(points, offsetForFlatIndex(6, subdivisionSize, rootPos), octreeDivision.divisions.at(6), subdivisionSize, subdivisionLevel + 1, bottomLeftBackPath);

    // +x -y +z
    auto bottomRightBackPath = path;
    bottomRightBackPath.push_back(7);
    addOctreeLevel(points, offsetForFlatIndex(7, subdivisionSize, rootPos), octreeDivision.divisions.at(7), subdivisionSize, subdivisionLevel + 1, bottomRightBackPath);
  }else if (octreeDivision.fill == FILL_FULL){
    auto cellIndex = indexForOctreePath(path);
    auto cellAddress = cellIndex.value;
    modassert(cellIndex.subdivisionLevel == subdivisionLevel, "invalid result for octree path, probably provided incorrect path for subdivisionLevel");

    auto blockShape = std::get_if<ShapeBlock>(&octreeDivision.shape);
    if (blockShape){
      glm::ivec3 cellToTheFront(cellAddress.x, cellAddress.y, cellAddress.z - 1);
      if (shouldShowCubeSide(octreeFillStatus(subdivisionLevel, cellToTheFront), FRONT)){
        addCubePointsFront(points, size, rootPos,  &octreeDivision.faces);
      }

      glm::ivec3 cellToTheBack(cellAddress.x, cellAddress.y, cellAddress.z + 1);
      if (shouldShowCubeSide(octreeFillStatus(subdivisionLevel, cellToTheBack), BACK)){  
        addCubePointsBack(points, size, rootPos, &octreeDivision.faces);
      }
      //
      glm::ivec3 cellToTheLeft(cellAddress.x - 1, cellAddress.y, cellAddress.z);
      if (shouldShowCubeSide(octreeFillStatus(subdivisionLevel, cellToTheLeft), LEFT)){  
        addCubePointsLeft(points, size, rootPos,  &octreeDivision.faces);
      }

      glm::ivec3 cellToTheRight(cellAddress.x + 1, cellAddress.y, cellAddress.z);
      if (shouldShowCubeSide(octreeFillStatus(subdivisionLevel, cellToTheRight), RIGHT)){  
        addCubePointsRight(points, size, rootPos,  &octreeDivision.faces);
      }  

      glm::ivec3 cellToTheTop(cellAddress.x, cellAddress.y + 1, cellAddress.z);
      if (shouldShowCubeSide(octreeFillStatus(subdivisionLevel, cellToTheTop), UP)){  
        addCubePointsTop(points, size, rootPos,  &octreeDivision.faces);
      }  

      glm::ivec3 cellToTheBottom(cellAddress.x, cellAddress.y - 1, cellAddress.z);
      if (shouldShowCubeSide(octreeFillStatus(subdivisionLevel, cellToTheBottom), DOWN)){  
        addCubePointsBottom(points, size, rootPos,  &octreeDivision.faces);
      }      
    }
    auto rampShape = std::get_if<ShapeRamp>(&octreeDivision.shape);
    if (rampShape){
      addRamp(points, size, rootPos, &octreeDivision.faces, *rampShape);
    }

  }
}

void addAllDivisions(std::vector<PositionAndScale>& octreeCubes, std::vector<PositionAndScale>& rampBlocks, OctreeDivision& octreeDivision, float size, glm::vec3 rootPos){
  bool isBlockShape = std::get_if<ShapeBlock>(&octreeDivision.shape);
  bool isRampShape = std::get_if<ShapeRamp>(&octreeDivision.shape);
  modassert(isBlockShape || isRampShape, "shape type not supported");

  if (octreeDivision.fill == FILL_FULL){
    std::cout << "size = " << size << ", root = " << print(rootPos) << std::endl;
    if (isBlockShape){
      octreeCubes.push_back(PositionAndScale {
        .position = rootPos, 
        .size = glm::vec3(size, size, size),
      });
    }else if (isRampShape){
      rampBlocks.push_back(PositionAndScale {
        .position = rootPos,
        .size = glm::vec3(size, size, size),
      });
    }
  }else if (octreeDivision.fill == FILL_MIXED){
    float subdivisionSize = size * 0.5f; 
    modassert(octreeDivision.divisions.size() == 8, "expected 8 octree division addAllDivisions");
    for (int i = 0; i < octreeDivision.divisions.size(); i++){ 
      glm::vec3 offset = offsetForFlatIndex(i, subdivisionSize, rootPos);
      addAllDivisions(octreeCubes, rampBlocks, octreeDivision.divisions.at(i), subdivisionSize, offset);
    }
  }
}
PhysicsShapes getPhysicsShapes(){
  std::vector<PositionAndScale> octreeCubes;


  // ENUMATE OUT THE REST OF THE VERTS FOR A RAMP
  std::vector<PositionAndScaleVerts> shapes = {
    PositionAndScaleVerts {
      .verts = {
        glm::vec3(0.f, 0.5f, -0.5f),
        glm::vec3(0.f, 0.f, -0.5f),
        glm::vec3(0.f, 0.f, 0.f),

        glm::vec3(0.5f, 0.5f, -0.5f),
        glm::vec3(0.5f, 0.f, -0.5f),
        glm::vec3(0.5f, 0.f, 0.f),

      },
      .specialBlocks = {},
    }
  };

  addAllDivisions(octreeCubes, shapes.at(0).specialBlocks, testOctree.rootNode, 1.f, glm::vec3(0.f, 0.f, 0.f));
  PhysicsShapes physicsShapes {};
  physicsShapes.blocks = octreeCubes;
  physicsShapes.shapes = shapes;
  return physicsShapes;
}

Mesh createOctreeMesh(std::function<Mesh(MeshData&)> loadMesh){
  std::vector<Vertex> vertices;
  std::vector<OctreeVertex> points = {};

  std::cout << "adding octree start" << std::endl;
  addOctreeLevel(points, glm::vec3(0.f, 0.f, 0.f), testOctree.rootNode, testOctree.size, 0, {});

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
  modassert(currentSubdivision <= subdivisionDepth, "current subdivision should not be greater than the target subdivision depth");
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

  raycastSubdivision(fromPos, toPosDirection, glm::ivec3(0, 0, 0), 0, subdivisionDepth, finalIntersections);

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

void handleOctreeRaycast(glm::vec3 fromPos, glm::vec3 toPosDirection, bool secondarySelection){
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
    if (!newRaycast.has_value()){
      return;
    }
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


void drawPhysicsShape(PositionAndScale& physicShape, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
  auto leftX = physicShape.position.x;
  auto rightX = physicShape.position.x + physicShape.size.x;
  auto topY = physicShape.position.y + physicShape.size.y;
  auto bottomY = physicShape.position.y;
  auto farZ = physicShape.position.z - physicShape.size.z;
  auto nearZ = physicShape.position.z;

  drawLine(glm::vec3(leftX, bottomY, nearZ), glm::vec3(rightX, bottomY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(glm::vec3(leftX, topY, nearZ), glm::vec3(rightX, topY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));

  drawLine(glm::vec3(leftX, bottomY, farZ), glm::vec3(rightX, bottomY, farZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(glm::vec3(leftX, topY, farZ), glm::vec3(rightX, topY, farZ), glm::vec4(1.f, 0.f, 0.f, 1.f));

  drawLine(glm::vec3(leftX, topY, farZ), glm::vec3(leftX, topY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(glm::vec3(rightX, topY, farZ), glm::vec3(rightX, topY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));

  drawLine(glm::vec3(leftX, bottomY, farZ), glm::vec3(leftX, bottomY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(glm::vec3(rightX, bottomY, farZ), glm::vec3(rightX, bottomY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));

  drawLine(glm::vec3(rightX, bottomY, farZ), glm::vec3(rightX, topY, farZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(glm::vec3(leftX, bottomY, farZ), glm::vec3(leftX, topY, farZ), glm::vec4(1.f, 0.f, 0.f, 1.f));

  drawLine(glm::vec3(rightX, bottomY, nearZ), glm::vec3(rightX, topY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(glm::vec3(leftX, bottomY, nearZ), glm::vec3(leftX, topY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
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

  // visualize the physics objects
  auto physicsShapes = getPhysicsShapes();
  for (auto &physicShape : physicsShapes.blocks){
    drawPhysicsShape(physicShape, drawLine);
  }
}

int getNumOctreeNodes(OctreeDivision& octreeDivision){
  int numNodes = 1;
  for (auto &subdivision : octreeDivision.divisions){
    numNodes += getNumOctreeNodes(subdivision);
  }

  return numNodes;
}

void makeOctreeCellRamp(Octree& octree, int x, int y, int z, int subdivision, RampDirection rampDirection, float startHeight = 0.f, float endHeight = 1.f){
  auto octreeDivision = getOctreeSubdivisionIfExists(octree, x, y, z, subdivision);
  modassert(octreeDivision, "octreeDivision does not exist");
  octreeDivision -> shape = ShapeRamp {
    .direction = rampDirection,
    .startHeight = startHeight,
    .endHeight = endHeight,
  };
  writeOctreeCell(octree,  x, y, z, subdivision, true); // kind of hackey, but just to ensure parents are updated
}

void makeOctreeCellRamp(GameObjectOctree& octree, std::function<Mesh(MeshData&)> loadMesh){
  for (int x = 0; x < selectionDim.value().x; x++){
    for (int y = 0; y < selectionDim.value().y; y++){
      for (int z = 0; z < selectionDim.value().z; z++){
        RampDirection direction = RAMP_FORWARD;
        if (direction == RAMP_RIGHT){
          float unitHeight = 1.f / selectionDim.value().x;
          float startHeight = unitHeight * (selectionDim.value().x - x - 1);
          float endHeight = unitHeight * (selectionDim.value().x - x);
          makeOctreeCellRamp(testOctree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel, direction, startHeight, endHeight);
        }else if (direction == RAMP_LEFT){
          float unitHeight = 1.f / selectionDim.value().x;
          float startHeight = unitHeight * x;
          float endHeight = unitHeight * (x + 1);
          std::cout << "make ramp left: " << startHeight << ", " << endHeight << std::endl;
          makeOctreeCellRamp(testOctree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel, direction, startHeight, endHeight);
        }else if (direction == RAMP_FORWARD){
          float unitHeight = 1.f / selectionDim.value().z;
          float startHeight = unitHeight * z;
          float endHeight = unitHeight * (z + 1);
          makeOctreeCellRamp(testOctree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel, direction, startHeight, endHeight);
        }else if (direction == RAMP_BACKWARD){
          float unitHeight = 1.f / selectionDim.value().z;
          float startHeight = unitHeight * (selectionDim.value().z - z - 1);
          float endHeight = unitHeight * (selectionDim.value().z - z);
          makeOctreeCellRamp(testOctree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel, direction, startHeight, endHeight);

        }
      }
    }
  }

  octree.mesh = createOctreeMesh(loadMesh);
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
      auto octreeDivision = getOctreeSubdivisionIfExists(testOctree, subIndex.x, subIndex.y, subIndex.z, subdivisionLevel);
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

  std::cout << "write octree texture-------------------" << std::endl;

  std::cout << std::endl << std::endl;
  octree.mesh = createOctreeMesh(loadMesh);

}

int getOctreeTextureId(){
  return selectedTexture;
}
void setOctreeTextureId(int textureId){
  if (textureId < 0){
    textureId = 0;
  }
  if (textureId >= atlasDimensions.value().totalTextures){
    textureId = atlasDimensions.value().totalTextures - 1;
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


std::string serializedOctreeStr = serializeOctree(testOctree);
void loadOctree(GameObjectOctree& octree, std::function<Mesh(MeshData&)> loadMesh){
  modlog("octree", "loading");
  testOctree = deserializeOctree(serializedOctreeStr);
  octree.mesh = createOctreeMesh(loadMesh);
}

void saveOctree(){
  modlog("octree", "saving");
  auto serializedData = serializeOctree(testOctree);
  std::cout << "octree data: \n" << serializedData << std::endl;
  serializedOctreeStr = serializedData;
}
