#include "./obj_octree.h"

std::string readFileOrPackage(std::string filepath);

std::optional<glm::ivec3> selectedIndex = glm::ivec3(1, 0, 0);
std::optional<glm::ivec3> selectionDim = glm::ivec3(1, 1, 0);
OctreeSelectionFace editorOrientation = FRONT;
int selectedTexture = 0;

std::optional<Line> line = std::nullopt;
int subdivisionLevel = 1;
std::optional<objid> selectedOctreeId = std::nullopt;

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

    resources::TEXTURE_WATER,
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
        OctreeDivision { .fill = defaultFill, .material = OCTREE_MATERIAL_DEFAULT, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .material = OCTREE_MATERIAL_DEFAULT, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .material = OCTREE_MATERIAL_DEFAULT, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .material = OCTREE_MATERIAL_DEFAULT, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .material = OCTREE_MATERIAL_DEFAULT, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .material = OCTREE_MATERIAL_DEFAULT, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .material = OCTREE_MATERIAL_DEFAULT, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
        OctreeDivision { .fill = defaultFill, .material = OCTREE_MATERIAL_DEFAULT, .shape = ShapeBlock{}, .faces = octreeSubdivision -> faces },
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

void writeOctreeCellRange(Octree& octree, int x, int y, int z, int width, int height, int depth, int subdivision, bool filled){
  for (int i = 0; i < width; i++){
    for (int j = 0; j < height; j++){
      for (int k = 0; k < depth; k++){
        writeOctreeCell(octree, x + i, y + j, z + k, subdivision, filled);
      }
    }
  }
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

void makeOctreeCellMaterial(GameObjectOctree& gameobjOctree, int x, int y, int z, int subdivision, OctreeMaterial material){
  auto octreeDivision = getOctreeSubdivisionIfExists(gameobjOctree.octree, x, y, z, subdivision);
  if (octreeDivision == NULL){
    modlog("octree editor", "octree division does not exist");
    return;
  }
  //modassert(octreeDivision, "octreeDivision does not exist");
  octreeDivision ->  material = material;
}

void makeOctreeCellMaterial(GameObjectOctree& gameobjOctree, std::function<Mesh(MeshData&)> loadMesh, OctreeMaterial material){
  for (int x = 0; x < selectionDim.value().x; x++){
    for (int y = 0; y < selectionDim.value().y; y++){
      for (int z = 0; z < selectionDim.value().z; z++){
        makeOctreeCellMaterial(gameobjOctree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel, material);
      }
    }
  }

  gameobjOctree.meshes = createOctreeMesh(gameobjOctree.octree, loadMesh);
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

  gameobjOctree.meshes = createOctreeMesh(octree, loadMesh);
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

  gameobjOctree.meshes = createOctreeMesh(octree, loadMesh);
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
  gameobjOctree.meshes = createOctreeMesh(octree, loadMesh);
}
void deleteSelectedOctreeNodes(GameObjectOctree& gameobjOctree, Octree& octree, std::function<Mesh(MeshData&)> loadMesh){
  writeOctreeCellRange(octree, selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, selectionDim.value().z, subdivisionLevel, false);
  gameobjOctree.meshes = createOctreeMesh(octree, loadMesh);
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

  gameobjOctree.meshes = createOctreeMesh(octree, loadMesh);
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

// Editor convenience fns
////////////////////////////////////////////////////////////////////////////////////



// Core objtype fns
////////////////////////////////////////////////////////////////////////////////////


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
    if (serializedFileData == ""){
      obj.octree = unsubdividedOctree;
    }else{
      obj.octree = deserializeOctree(serializedFileData);
    }
  }else{
    obj.octree = unsubdividedOctree;
  }

  obj.meshes = createOctreeMesh(obj.octree, util.loadMesh);
  return obj;
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
  octree.meshes = createOctreeMesh(octree.octree, loadMesh);
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

void addZone(GameObjectOctree& gameobjOctree, int symbol){
//  std::vector<int> tags;
  for (int x = 0; x < selectionDim.value().x; x++){
    for (int y = 0; y < selectionDim.value().y; y++){
      for (int z = 0; z < selectionDim.value().z; z++){
        auto octreeDivision = getOctreeSubdivisionIfExists(gameobjOctree.octree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel);
        bool hasTag = false;
        for (auto tag : octreeDivision -> tags){
          if (tag == symbol){
            hasTag = true;
          }
        }
        if (!hasTag){
          octreeDivision -> tags.push_back(symbol);
        }
      }
    }
  }
}
void removeZone(GameObjectOctree& gameobjOctree, int symbol){
//   std::vector<int> tags;
  for (int x = 0; x < selectionDim.value().x; x++){
    for (int y = 0; y < selectionDim.value().y; y++){
      for (int z = 0; z < selectionDim.value().z; z++){
        auto octreeDivision = getOctreeSubdivisionIfExists(gameobjOctree.octree, selectedIndex.value().x + x, selectedIndex.value().y + y, selectedIndex.value().z + z, subdivisionLevel);
        octreeDivision -> tags = {  };
      }
    }
  }
}