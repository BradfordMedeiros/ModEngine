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
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
          OctreeDivision { .filled = true },
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
  for (int i = 0; i < path.size(); i++){
    // todo -> if the subdivision isn't made here, should make it here
    if (octreeSubdivision -> divisions.size() == 0){
      octreeSubdivision -> divisions = {
        OctreeDivision { .filled = false },
        OctreeDivision { .filled = false },
        OctreeDivision { .filled = false },
        OctreeDivision { .filled = false },
        OctreeDivision { .filled = false },
        OctreeDivision { .filled = false },
        OctreeDivision { .filled = false },
        OctreeDivision { .filled = false },
      };
    } 
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

std::optional<glm::ivec3> selectedIndex = glm::ivec3(1, 0, 0);
std::optional<glm::ivec3> selectionDim = glm::ivec3(1, 1, 0);
OctreeSelectionFace editorOrientation = FRONT;

std::optional<Line> line = std::nullopt;
int subdivisionLevel = 2;

void drawOctreeSelectionGrid(std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
  if (selectedIndex.has_value()){
    std::cout << "draw grid, z: " << selectionDim.value().z << std::endl;

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

void handleOctreeRaycast(glm::vec3 fromPos, glm::vec3 toPosDirection){
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

void increaseSelectionSize(int width, int height){
  selectionDim.value().x+= width;
  selectionDim.value().y+= height;
}
void decreaseSelectionSize(int width, int height){
  selectionDim.value().x-= width;
  selectionDim.value().y-= height;
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


