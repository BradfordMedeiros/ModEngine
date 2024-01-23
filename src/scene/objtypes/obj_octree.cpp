#include "./obj_octree.h"

std::vector<AutoSerialize> octreeAutoserializer {};



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
  std::vector<OctreeDivision> divisions; 
};

struct Octree {
  double size;
  OctreeDivision rootNode;
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


Octree unsubdividedOctree {
  .size = 1.f,
  .rootNode = OctreeDivision {
    .filled = false,
    .divisions = {},
  },
};

Octree subdividedOne {
  .size = 2.f,
  .rootNode = OctreeDivision {
    .filled = true,
    .divisions = {
      OctreeDivision { 
        .filled = true,
        .divisions = {
         
        },
      },
      OctreeDivision { .filled = false },
      OctreeDivision { 
        .filled = true,
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
        OctreeDivision { .filled = defaultFill },
        OctreeDivision { .filled = defaultFill },
        OctreeDivision { .filled = defaultFill },
        OctreeDivision { .filled = defaultFill },
        OctreeDivision { .filled = defaultFill },
        OctreeDivision { .filled = defaultFill },
        OctreeDivision { .filled = defaultFill },
        OctreeDivision { .filled = defaultFill },
      };
    } 
    // check if all filled, then set the divsions = {}, and filled = true
    octreeSubdivision = &octreeSubdivision -> divisions.at(xyzIndexToFlatIndex(path.at(i)));
  }
  octreeSubdivision -> filled = filled;
  octreeSubdivision -> divisions = {};
  
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


Vertex createVertex2(glm::vec3 position, glm::vec2 texCoords, glm::vec3 normal){
  Vertex vertex {
    .position = position,
    .normal = normal,
    .texCoords = texCoords,
  };
  for (int i = 0; i < NUM_BONES_PER_VERTEX; i++){
    vertex.boneIndexes[i] = 0;
    vertex.boneWeights[i] = 0;
  }
  return vertex;
}

void addCubePoints(std::vector<glm::vec3>& points, float size, glm::vec3 offset){
  //bottom plane
  points.push_back(glm::vec3(0.f, 0.f, -size) + offset);
  points.push_back(glm::vec3(size, 0.f, 0.f) + offset);
  points.push_back(glm::vec3(0.f, 0.f, 0.f) + offset);

  points.push_back(glm::vec3(size, 0.f, -size) + offset);
  points.push_back(glm::vec3(size, 0.f, 0.f) + offset);
  points.push_back(glm::vec3(0.f, 0.f, -size) + offset);

  // top plane
  points.push_back(glm::vec3(0.f, size, 0.f) + offset);
  points.push_back(glm::vec3(size, size, 0.f) + offset);
  points.push_back(glm::vec3(0.f, size, -size) + offset);

  points.push_back(glm::vec3(0.f, size, -size) + offset);
  points.push_back(glm::vec3(size, size, 0.f) + offset);
  points.push_back(glm::vec3(size, size, -size) + offset);

  // left plane
  points.push_back(glm::vec3(0.f, size, -size) + offset);
  points.push_back(glm::vec3(0.f, 0.f, -size) + offset);
  points.push_back(glm::vec3(0.f, 0.f, 0.f) + offset);

  points.push_back(glm::vec3(0.f, size, 0.f) + offset);
  points.push_back(glm::vec3(0.f, size, -size) + offset);
  points.push_back(glm::vec3(0.f, 0.f, 0.f) + offset);

  // right plane
  points.push_back(glm::vec3(size, 0.f, 0.f) + offset);
  points.push_back(glm::vec3(size, 0.f, -size) + offset);
  points.push_back(glm::vec3(size, size, -size) + offset);

  points.push_back(glm::vec3(size, 0.f, 0.f) + offset);
  points.push_back(glm::vec3(size, size, -size) + offset);
  points.push_back(glm::vec3(size, size, 0.f) + offset);


  // front plane
  points.push_back(glm::vec3(size, 0.f, 0.f) + offset);
  points.push_back(glm::vec3(0.f, size, 0.f) + offset);
  points.push_back(glm::vec3(0.f, 0.f, 0.f) + offset);

  points.push_back(glm::vec3(size, size, 0.f) + offset);
  points.push_back(glm::vec3(0.f, size, 0.f) + offset);
  points.push_back(glm::vec3(size, 0.f, 0.f) + offset);


  // back plane
  points.push_back(glm::vec3(0.f, 0.f, -size) + offset);
  points.push_back(glm::vec3(0.f, size, -size) + offset);
  points.push_back(glm::vec3(size, 0.f, -size) + offset);

  points.push_back(glm::vec3(size, 0.f, -size) + offset);
  points.push_back(glm::vec3(0.f, size, -size) + offset);
  points.push_back(glm::vec3(size, size, -size) + offset);
}

void addOctreeLevel(std::vector<glm::vec3>& points, glm::vec3 rootPos, OctreeDivision& octreeDivision, float size, int subdivisionLevel){
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
    addCubePoints(points, size, rootPos);
  }

}

Mesh createOctreeMesh(std::function<Mesh(MeshData&)> loadMesh){
  std::vector<Vertex> vertices;
  std::vector<glm::vec3> points = {};

  std::cout << "adding octree start" << std::endl;
  addOctreeLevel(points, glm::vec3(0.f, 0.f, 0.f), testOctree.rootNode, testOctree.size, 0);
  std::cout << "adding octree end" << std::endl;

  if (points.size() == 0){ // just hack for now 
    addCubePoints(points, 0.0001f, glm::vec3(0.f, 0.f, 0.f));
  }

  for (int i = 0; i < points.size(); i+=3){
    glm::vec3 vec1 = points.at(i) - points.at(i + 1);
    glm::vec3 vec2 = points.at(i) - points.at(i + 2);
    auto normal = glm::cross(vec1, vec2); // think about sign better, i think this is right 
    vertices.push_back(createVertex2(points.at(i), glm::vec2(0.f, 0.f), normal));  // maybe the tex coords should just be calculated as a ratio to a fix texture
    vertices.push_back(createVertex2(points.at(i + 1), glm::vec2(1.f, 0.f), normal));
    vertices.push_back(createVertex2(points.at(i + 2), glm::vec2(0.f, 1.f), normal));
  }
  std::vector<unsigned int> indices;
  for (int i = 0; i < vertices.size(); i++){ // should be able to optimize this better...
    indices.push_back(i);
  }
  MeshData meshData {
    .vertices = vertices,
    .indices = indices,
    .diffuseTexturePath = "./res/textures/grid.png",
    .hasDiffuseTexture = true,
    .emissionTexturePath = "",
    .hasEmissionTexture = false,
    .opacityTexturePath = "",
    .hasOpacityTexture = false,
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

enum OctreeSelectionFace { FRONT, BACK, LEFT, RIGHT, UP, DOWN };

void drawGridSelectionXY(int x, int y, int z, int numCellsWidth, int numCellsHeight, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
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
void drawGridSelectionYZ(int x, int y, int z, int numCellsHeight, int numCellsDepth, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
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

void drawGridSelectionCube(int x, int y, int z, int numCellsWidth, int numCellsHeight, int numCellDepth, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
  drawGridSelectionXY(x, y, z,     1, 1, subdivision, size, drawLine);
  drawGridSelectionXY(x, y, z + 1, 1, 1, subdivision, size, drawLine);
  drawGridSelectionYZ(x, y, z, 1, 1, subdivision, size, drawLine);
  drawGridSelectionYZ(x + 1, y, z, 1, 1, subdivision, size, drawLine);
}


std::optional<glm::ivec3> selectedIndex = glm::ivec3(1, 0, 0);
std::optional<glm::ivec3> selectionDim = glm::ivec3(1, 1, 0);
OctreeSelectionFace editorOrientation = FRONT;

std::optional<Line> line = std::nullopt;
int subdivisionLevel = 2;

void drawOctreeSelectedCell(int x, int y, int z, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
  drawGridSelectionCube(x, y, z, 1, 1, 1, subdivision, size, drawLine);
}

void drawOctreeSelectionGrid(std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
  if (selectedIndex.has_value()){
    //std::cout << "draw grid, z: " << selectionDim.value().z << std::endl;
    drawGridSelectionXY(selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, subdivisionLevel, testOctree.size,  drawLine);
    if (selectionDim.value().z > 0){
      drawGridSelectionXY(selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z + selectionDim.value().z, selectionDim.value().x, selectionDim.value().y, subdivisionLevel, testOctree.size,  drawLine);
    }
    //std::cout << "draw octree" << std::endl;
    if (line.has_value()){
      drawLine(line.value().fromPos, line.value().toPos, glm::vec4(1.f, 0.f, 0.f, 1.f));
    }
  }
}


struct Faces {
  float XYClose;
  float XYFar;
  float XZLeft;
  float XZRight;
  float XZTop;
  float XZBottom;
};

Faces getFaces(int x, int y, int z, float size /* todo add subdivision level */){
  Faces faces {
    .XYClose = 1.f,
    .XYFar = -1.f,
    .XZLeft = -1.f, 
    .XZRight = 1.f,
    .XZTop = 1.f,
    .XZBottom = -1.f,
  };
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
glm::vec3 calculateTForX(glm::vec3 pos, glm::vec3 dir, float x){
  float t = (x - pos.x) / dir.x;
  float y = pos.y + (dir.y * t);
  float z = pos.z + (dir.z * t);
  return glm::vec3(x, y, z);
}
glm::vec3 calculateTForY(glm::vec3 pos, glm::vec3 dir, float y){
  float t = (y - pos.y) / dir.y;
  float x = pos.x + (dir.x * t);
  float z = pos.z + (dir.z * t);
  return glm::vec3(x, y, z);
}
glm::vec3 calculateTForZ(glm::vec3 pos, glm::vec3 dir, float z){
  float t = (z - pos.z) / dir.z;
  float x = pos.x + (dir.x * t);
  float y = pos.y + (dir.y * t);
  return glm::vec3(x, y, z);
}

bool checkIfInCube(Faces& faces, bool checkX, bool checkY, bool checkZ, glm::vec3 point){
  return false;
}

bool intersectsCube(glm::vec3 fromPos, glm::vec3 toPosDirection, int x, int y, int z, float size){
  auto faces = getFaces(x, y, z, testOctree.size);

  auto intersectionLeft = calculateTForX(fromPos, toPosDirection, faces.XZLeft);
  bool intersectsLeftFace = checkIfInCube(faces, false, true, true, intersectionLeft);

  auto intersectionRight = calculateTForX(fromPos, toPosDirection, faces.XZRight);
  bool intersectsRightFace = checkIfInCube(faces, false, true, true, intersectionRight);

  auto intersectionTop = calculateTForY(fromPos, toPosDirection, faces.XZTop);
  bool intersectsTop = checkIfInCube(faces, true, false, true, intersectionTop);

  auto intersectionBottom = calculateTForY(fromPos, toPosDirection, faces.XZBottom);
  bool intersectsBottom = checkIfInCube(faces, true, false, true, intersectionBottom);

  auto intersectionClose = calculateTForZ(fromPos, toPosDirection, faces.XYClose);
  bool intersectsClose  = checkIfInCube(faces, true, false, true, intersectionClose);


  auto intersectionFar = calculateTForZ(fromPos, toPosDirection, faces.XYFar);
  bool intersectsFar  = checkIfInCube(faces, true, false, true, intersectionFar);


  bool intersectsCube = (intersectsRightFace || intersectsLeftFace || intersectsTop || intersectsBottom || intersectsClose || intersectsFar);
  return intersectsCube; 
}

std::vector<int> subdivisionIntersections(glm::vec3 fromPos, glm::vec3 toPosDirection, float size){
  std::vector<int> intersections;
  for (int x = 0; x < 2; x++){
    for (int y = 0; y < 2; y++){
      for (int z = 0; z < 2; z++){
        // notice that adjacent faces are duplicated here
        if (intersectsCube(fromPos, toPosDirection, x, y, z, size)){
          intersections.push_back(xyzIndexToFlatIndex(glm::ivec3(x, y, z)));
        }
      }

    }
  }
  return intersections;
}

void handleOctreeRaycast(glm::vec3 fromPos, glm::vec3 toPosDirection){
  auto serializedData = serializeOctree(testOctree);
  std::cout << "octree serialization: \n" << serializedData << std::endl;

  Octree octree = deserializeOctree(serializedData);
  std::cout << "octree serialization 2: \n" << serializeOctree(octree) << std::endl;




  // for each voxel face, figure out what is held constnat,
  // plug it into the parametric equations to solve for remaining values
  // then check if the boundaries of those are in the face or not (boundaries check + dot ?)
  // repeat for each face
  


  std::cout << "handle octree raycast -  from: " << print(fromPos) << ", to: " << print(glm::normalize(toPosDirection)) << std::endl;

  return;
  //if (selectedIndex.value().x > 5){
  //  selectedIndex.value().x = selectedIndex.value().x - 1;
  //}else{
  //  selectedIndex.value().x = selectedIndex.value().x + 1;
  //}

  if (selectedIndex.value().y > 5){
    selectedIndex.value().y = selectedIndex.value().y - 1;
  }else{
    selectedIndex.value().y = selectedIndex.value().y + 1;
  }

  glm::vec4 toPos =  glm::vec4(fromPos.x, fromPos.y, fromPos.z, 1.f) + glm::vec4(toPosDirection.x, toPosDirection.y, toPosDirection.z, 1.f);
  line = Line {
    .fromPos = fromPos,
    .toPos = toPos,
  };




  // model parameterically 
  // 0 = (point_x + dir_x * t) 
  // y = (point_y + dir_y * t)
  // should be able to solve for y
  // then we have origin + dir

  // iterate through the voxels, 
  // determine if 

  /*

  glm::vec4 toPosModelSpace = glm::inverse(voxelPtrModelMatrix) * toPos;
  glm::vec3 rayDirectionModelSpace =  toPosModelSpace - fromPosModelSpace;
  // This raycast happens in model space of voxel, so specify position + ray in voxel model space
  auto collidedVoxels = raycastVoxels(voxelPtr -> voxel, fromPosModelSpace, rayDirectionModelSpace);
  std::cout << "length is: " << collidedVoxels.size() << std::endl;
  if (collidedVoxels.size() > 0){
    auto collision = collidedVoxels.at(0);
    voxelPtr -> voxel.selectedVoxels.push_back(collision);
    applyTextureToCube(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels, textureId);
  }*/ 
}

int getNumOctreeNodes(OctreeDivision& octreeDivision){
  int numNodes = 1;
  for (auto &subdivision : octreeDivision.divisions){
    numNodes += getNumOctreeNodes(subdivision);
  }

  return numNodes;
}

void handleOctreeScroll(GameObjectOctree& octree, bool upDirection, std::function<Mesh(MeshData&)> loadMesh, bool holdingShift, bool holdingCtrl, OctreeDimAxis axis){
  if (holdingShift){
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

  if (holdingCtrl){
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
  writeOctreeCellRange(testOctree, selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, selectionDim.value().z, subdivisionLevel, true);
  if (axis == OCTREE_NOAXIS || axis == OCTREE_ZAXIS){
    selectedIndex.value().z = selectedIndex.value().z + (upDirection ? -1 : 1);
  }else if (axis == OCTREE_XAXIS){
    selectedIndex.value().x = selectedIndex.value().x + (upDirection ? 1 : -1);
  }else if (axis == OCTREE_YAXIS){
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

  writeOctreeCellRange(testOctree, selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, selectionDim.value().z, subdivisionLevel, true);

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

void handleSetSelectionOrientation(OctreeSelectionFace face){
  //{ FRONT, BACK, LEFT, RIGHT, UP, DOWN }
  if (face == RIGHT){
    face = LEFT;
  }else if (face == BACK){
    face = FRONT;
  }else if (face == DOWN){
    face = UP;
  }
  editorOrientation = face;
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


