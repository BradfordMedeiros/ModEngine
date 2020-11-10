#include "./voxels.h"

BoundInfo generateVoxelBoundInfo(std::vector<std::vector<std::vector<int>>>& cubes, int numWidth, int numHeight, int numDepth){  
  float xMin = 0;
  float xMax = 0;
  float yMin = 0;
  float yMax = 0;
  float zMin = 0;
  float zMax = 0;

  for (int x = 0; x < cubes.size(); x++){
    for (int y = 0; y < cubes.at(0).size(); y++){
      for (int z = 0; z < cubes.at(0).at(0).size(); z++){
        int value = cubes.at(x).at(y).at(z);
        if (value != 0){
          if (x < xMin){
            xMin = x;
          }
          if ((x + 1) > xMax){
            xMax = (x + 1);
          }
          if (y < yMin){
            yMin = y;
          }
          if ((y + 1) > yMax){
            yMax = (y + 1);
          }
          if (z < zMin){
            zMin = z;
          }
          if ((z + 1) > zMax){
            zMax = (z + 1);
          }
        }
      }
    }
  }
  BoundInfo info = {
    .xMin = xMin, .xMax = xMax,
    .yMin = yMin, .yMax = yMax,
    .zMin = zMin, .zMax = zMax,
    .isNotCentered = true
  };
  return info; 
}

Voxels createVoxels(VoxelState initialState, std::function<void()> onVoxelBoundInfoChanged, unsigned int defaultTexture){
  int numWidth = initialState.numWidth;
  int numHeight = initialState.numHeight;
  int numDepth = initialState.numDepth;
  auto cubes = initialState.cubes;

  std::vector<VoxelAddress> selectedVoxels;
  Voxels vox = {
    .cubes = cubes,
    .numWidth = numWidth,
    .numHeight = numHeight,
    .numDepth = numDepth,
    .boundInfo = generateVoxelBoundInfo(cubes, numWidth, numHeight, numDepth),
    .selectedVoxels = selectedVoxels,
    .onVoxelBoundInfoChanged = onVoxelBoundInfoChanged,
    .defaultTextureId = defaultTexture,
  };

  for (int row = 0; row < numWidth; row++){
    for (int col = 0; col < numHeight; col++){
      for (int depth = 0; depth < numDepth; depth++){
        auto value = cubes.at(row).at(col).at(depth);
        if (value != 0){
          addVoxel(vox, row, col, depth, false);
        }
      }
    }
  }
  return vox;
}
VoxelState parseVoxelState(std::string voxelState){
  std::vector<std::vector<std::vector<int>>> cubes;

  auto voxelStrings = split(voxelState, '|');
  int numWidth = std::stoi(voxelStrings.at(0));
  int numHeight = std::stoi(voxelStrings.at(1));
  int numDepth = std::stoi(voxelStrings.at(2));
  std::string voxelData = voxelStrings.at(3);
  assert(voxelData.size() == (numWidth * numHeight * numDepth));

  std::vector<int> textureValues;
  for (char textureData : voxelData){
    textureValues.push_back(atoi(&textureData));
  }

  for (int row = 0; row < numWidth; row++){
    std::vector<std::vector<int>> cubestack;
    for (int col = 0; col < numHeight; col++){
      std::vector<int> cuberow;
      for (int depth = 0; depth < numDepth; depth++){
        auto flattenedIndex = (row * numHeight * numDepth) + (col * numDepth) + depth;  
        auto value = textureValues.at(flattenedIndex);
        cuberow.push_back(value == 0 ? 0 : 1);
      }
      cubestack.push_back(cuberow);
    }
    cubes.push_back(cubestack);
  }

  VoxelState state {
    .numWidth = numWidth,
    .numHeight = numHeight,
    .numDepth = numDepth,
    .cubes = cubes
  };
  return state;
}

void applyTextureToCube(Voxels& chunk, int x, int y, int z, int textureId){    
  std::cout << "need to apply to mesh texture to (" << x << ", " << y << ", " << z << ")" << std::endl;
}
void applyTextureToCube(Voxels& chunk, std::vector<VoxelAddress> voxels, int textureId){
  for (auto voxel : voxels){
    applyTextureToCube(chunk, voxel.x, voxel.y, voxel.z, textureId);
  }
}

void addVoxel(Voxels& chunk, int x, int y, int z, bool callOnChanged){    
  chunk.cubes.at(x).at(y).at(z) = 1;
  applyTextureToCube(chunk, x, y, z, 1);
  chunk.boundInfo = generateVoxelBoundInfo(chunk.cubes, chunk.numWidth, chunk.numHeight, chunk.numDepth);
  if (callOnChanged){
    chunk.onVoxelBoundInfoChanged();
  }
}
void addVoxel(Voxels& chunk, std::vector<VoxelAddress> voxels){
  for (auto voxel: voxels){
    addVoxel(chunk, voxel.x, voxel.y, voxel.z);
  }
}

void removeVoxel(Voxels& chunk, int x, int y, int z){
  chunk.cubes.at(x).at(y).at(z) = 0;
  applyTextureToCube(chunk, x, y, z, 0);
  chunk.boundInfo = generateVoxelBoundInfo(chunk.cubes, chunk.numWidth, chunk.numHeight, chunk.numDepth);
  chunk.onVoxelBoundInfoChanged();
}

void removeVoxel(Voxels& chunk, std::vector<VoxelAddress> voxels){
  for (auto voxel: voxels){
    removeVoxel(chunk, voxel.x, voxel.y, voxel.z);
  }
}

std::vector<VoxelAddress> raycastVoxels(Voxels& chunk, glm::vec3 rayPosition, glm::vec3 rayDirection){    
  float magnitudeLine = sqrt(chunk.numWidth * chunk.numWidth + chunk.numHeight * chunk.numHeight + chunk.numDepth * chunk.numDepth);
  glm::vec3 lineEnd = rayPosition + (rayDirection * magnitudeLine);

  glm::vec3 currentPosition = rayPosition;
  glm::vec3 rayIncrement = glm::normalize(rayDirection);

  std::vector<VoxelAddress> addresses;

  float endWidth = chunk.numWidth;
  float endHeight = chunk.numHeight;
  float endDepth = chunk.numDepth;

  bool terminatesXLow = rayIncrement.x < 0;
  bool terminatesYLow = rayIncrement.y < 0;
  bool terminatesZLow = rayIncrement.z < 0;

  while ( 
    ((currentPosition.x < endWidth || terminatesXLow) &&  (currentPosition.x > 0 || !terminatesXLow)) ||
    ((currentPosition.y < endHeight || terminatesYLow) && (currentPosition.y > 0 || !terminatesYLow)) || 
    ((currentPosition.z < endDepth || terminatesZLow) && (currentPosition.z > 0 || !terminatesZLow))
  ){
    auto position = currentPosition;
    currentPosition.x += rayIncrement.x;
    currentPosition.y += rayIncrement.y;
    currentPosition.z += rayIncrement.z;

    if (position.x < 0 || position.y < 0 || position.z < 0 || position.x > endWidth || position.y > endHeight || position.z > endDepth){
      continue;
    }

    if (chunk.cubes.at(position.x).at(position.y).at(position.z) == 1){
      VoxelAddress voxel = {
        .x = (int)(position.x),
        .y = (int)(position.y),
        .z = (int)(position.z)
      };
      addresses.push_back(voxel);
    } 
  }
  return addresses;
}

bool voxelEqual(VoxelAddress x, VoxelAddress y){
  return (x.x == y.x) && (x.y == y.y) && (x.z == y.z);
}
bool hasVoxel(std::vector<VoxelAddress> voxelList, VoxelAddress voxel){
  for (auto voxelAddress: voxelList){
    if (voxelEqual(voxelAddress, voxel)){
      return true;
    }
  }
  return false;
}

std::vector<VoxelAddress> expandVoxels(Voxels& chunk, std::vector<VoxelAddress> selectedVoxels, int x, int y, int z){
  std::vector<VoxelAddress> newSelectedVoxels;

  int multiplierValueX = (x >= 0) ? 1 : -1;
  int multiplierValueY = (y >= 0) ? 1 : -1;
  int multiplierValueZ = (z >= 0) ? 1 : -1;

  for (auto voxel : selectedVoxels){
    for (int xx = 0; xx <= (multiplierValueX * x); xx++){
      for (int yy = 0; yy <= (multiplierValueY * y); yy++){
        for (int zz = 0; zz <= (multiplierValueZ * z); zz++){
          int expandedX = voxel.x + (multiplierValueX * xx);
          int expandedY = voxel.y + (multiplierValueY * yy);
          int expandedZ = voxel.z + (multiplierValueZ * zz);

          if (expandedX < 0 || expandedX >= chunk.numWidth){
            continue;
          }
          if (expandedY < 0 || expandedY >= chunk.numHeight){
            continue;
          }
          if (expandedZ < 0 || expandedZ >= chunk.numDepth){
            continue;
          }
          VoxelAddress voxelToAdd {
            .x = expandedX,
            .y = expandedY,
            .z = expandedZ,
          };
          if (!hasVoxel(newSelectedVoxels, voxelToAdd)){
            newSelectedVoxels.push_back(voxelToAdd);
          }
        }
      }
    }
  }
  return newSelectedVoxels;
}

void expandVoxels(Voxels& voxel, int x, int y, int z){
  applyTextureToCube(voxel, voxel.selectedVoxels, 1);
  voxel.selectedVoxels = expandVoxels(voxel, voxel.selectedVoxels, x, y, z);
  addVoxel(voxel, voxel.selectedVoxels);
}

std::vector<VoxelBody> getVoxelBodies(Voxels& voxels){
  std::vector<VoxelBody> bodies;
  for (int x = 0; x < voxels.numWidth; x++){
    for (int y = 0; y < voxels.numHeight; y++){
      for (int z = 0; z < voxels.numDepth; z++){
        if (voxels.cubes.at(x).at(y).at(z) == 1){
          VoxelBody body = {
            .position = glm::vec3(x, y, z),
            .textureId = voxels.defaultTextureId,
          };
          bodies.push_back(body);
        }
      }
    }
  }
  return bodies;
}


