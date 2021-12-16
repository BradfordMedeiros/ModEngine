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
  };
  return info; 
}

Voxels createVoxels(VoxelState initialState, std::function<void()> onVoxelBoundInfoChanged, unsigned int defaultTexture){
  int numWidth = initialState.numWidth;
  int numHeight = initialState.numHeight;
  int numDepth = initialState.numDepth;
  auto cubes = initialState.cubes;
  auto textures = initialState.textures;

  std::vector<VoxelAddress> selectedVoxels;
  Voxels vox = {
    .cubes = cubes,
    .textures = textures,
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

VoxelState parseVoxelState(std::string voxelState, std::string voxelTextures, unsigned int defaultTexture, std::function<Texture(std::string)> loadTexture){
  std::vector<std::vector<std::vector<int>>> cubes;
  std::vector<std::vector<std::vector<unsigned int>>> textures;

  auto voxelStrings = split(voxelState, '|');
  int numWidth = std::stoi(voxelStrings.at(0));
  int numHeight = std::stoi(voxelStrings.at(1));
  int numDepth = std::stoi(voxelStrings.at(2));
  std::string voxelData = voxelStrings.at(3);
  assert(voxelData.size() == (numWidth * numHeight * numDepth));

  auto textureStrings = split(voxelTextures, '|');
  auto textureList = split(textureStrings.at(0), ',');

  auto textureListString = textureStrings.at(0);
  std::vector<int> texturePerVoxel;
  for (char textureData : textureStrings.at(1)){
    texturePerVoxel.push_back(atoi(&textureData));
  }

  std::vector<int> textureValues;
  for (char textureData : voxelData){
    textureValues.push_back(atoi(&textureData));
  }

  assert(texturePerVoxel.size() == textureValues.size());

  for (int row = 0; row < numWidth; row++){
    std::vector<std::vector<int>> cubestack;
    std::vector<std::vector<unsigned int>> texturestack;
    for (int col = 0; col < numHeight; col++){
      std::vector<int> cuberow;
      std::vector<unsigned int> texturerow;
      for (int depth = 0; depth < numDepth; depth++){
        auto flattenedIndex = (row * numHeight * numDepth) + (col * numDepth) + depth;  
        auto value = textureValues.at(flattenedIndex);

        auto textureIndex = texturePerVoxel.at(flattenedIndex);
        if (textureIndex == 0){
          texturerow.push_back(defaultTexture);
        }else{
          auto textureForVoxel = textureList.at(textureIndex -1);
          auto textureId = loadTexture(textureForVoxel).textureId;
          texturerow.push_back(textureId);
        }
        cuberow.push_back(value == 0 ? 0 : 1);
      }
      cubestack.push_back(cuberow);
      texturestack.push_back(texturerow);
    }
    cubes.push_back(cubestack);
    textures.push_back(texturestack);
  }

  VoxelState state {
    .numWidth = numWidth,
    .numHeight = numHeight,
    .numDepth = numDepth,
    .cubes = cubes,
    .textures = textures,
  };
  return state;
}

VoxelSerialization serializeVoxelState(Voxels& voxels, std::function<std::string(int)> textureName){
  auto header = std::to_string(voxels.numWidth) + "|" + std::to_string(voxels.numHeight) + "|" + std::to_string(voxels.numDepth) + "|";

  std::vector<int> textureIds;
  std::set<int> realTexIds;
  std::map<int, int> texIdToLocalId; 

  std::string content = "";
  for (int row = 0; row < voxels.numWidth; row++){
    for (int col = 0; col < voxels.numHeight; col++){
      for (int depth = 0; depth < voxels.numDepth; depth++){
        int value = voxels.cubes.at(row).at(col).at(depth);
        content = content + std::to_string(value);
        auto texId = voxels.textures.at(row).at(col).at(depth);
        textureIds.push_back(texId);
        realTexIds.insert(texId);
      }
    }
  }

  int localId = 0;
  std::vector<std::string> texNames;
  for (auto id : realTexIds){
    if (id == voxels.defaultTextureId){
      continue;
    }
    texIdToLocalId[id] = localId + 1;
    localId++;
    texNames.push_back(textureName(id));
  }

  std::string textureContent = join(texNames, ',') + "|";
  for (auto textureId : textureIds){
    if (textureId == voxels.defaultTextureId){
      textureContent = textureContent + "0";
    }else{
      textureContent = textureContent + std::to_string(texIdToLocalId[textureId]);
    }
  }

  return  VoxelSerialization {
    .voxelState = header + content,
    .textureState = textureContent,
  };
}

void applyTextureToCube(Voxels& chunk, int x, int y, int z, int textureId){    
  std::cout << "need to apply to mesh texture to (" << x << ", " << y << ", " << z << ") -- " << textureId << std::endl;
  chunk.textures.at(x).at(y).at(z) = textureId;
}
void applyTextureToCube(Voxels& chunk, std::vector<VoxelAddress> voxels, int textureId){
  for (auto voxel : voxels){
    applyTextureToCube(chunk, voxel.x, voxel.y, voxel.z, textureId);
  }
}

void addVoxel(Voxels& chunk, int x, int y, int z, bool callOnChanged){    
  chunk.cubes.at(x).at(y).at(z) = 1;
  applyTextureToCube(chunk, x, y, z, chunk.textures.at(x).at(y).at(z));
  chunk.boundInfo = generateVoxelBoundInfo(chunk.cubes, chunk.numWidth, chunk.numHeight, chunk.numDepth);
  if (callOnChanged){
    chunk.onVoxelBoundInfoChanged();
  }
}
void addVoxel(Voxels& chunk, std::vector<VoxelAddress> voxels){
  for (auto voxel: voxels){
    addVoxel(chunk, voxel.x, voxel.y, voxel.z, chunk.textures.at(voxel.x).at(voxel.y).at(voxel.z));
  }
}

void removeVoxel(Voxels& chunk, int x, int y, int z){
  chunk.cubes.at(x).at(y).at(z) = 0;
  applyTextureToCube(chunk, x, y, z, chunk.defaultTextureId);
  chunk.boundInfo = generateVoxelBoundInfo(chunk.cubes, chunk.numWidth, chunk.numHeight, chunk.numDepth);
  chunk.onVoxelBoundInfoChanged();
}

void removeVoxel(Voxels& chunk, std::vector<VoxelAddress> voxels){
  for (auto voxel: voxels){
    removeVoxel(chunk, voxel.x, voxel.y, voxel.z);
  }
}

std::vector<VoxelAddress> raycastVoxels(Voxels& chunk, glm::vec3 rayPosition, glm::vec3 rayDirection){    
  // since the voxels start centered, add half an offset to adjusted the ray position as if it started from 0 
  auto adjustedRayPosition = rayPosition + glm::vec3(chunk.numWidth / 2.f, chunk.numHeight / 2.f, chunk.numDepth / 2.f);  

  float magnitudeLine = sqrt(chunk.numWidth * chunk.numWidth + chunk.numHeight * chunk.numHeight + chunk.numDepth * chunk.numDepth);
  glm::vec3 lineEnd = adjustedRayPosition + (rayDirection * magnitudeLine);

  glm::vec3 currentPosition = adjustedRayPosition;
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
  applyTextureToCube(voxel, voxel.selectedVoxels, voxel.defaultTextureId);
  voxel.selectedVoxels = expandVoxels(voxel, voxel.selectedVoxels, x, y, z);
  addVoxel(voxel, voxel.selectedVoxels);
}

std::vector<VoxelBody> getVoxelBodies(Voxels& voxels){
  std::vector<VoxelBody> bodies;

  auto widthOffset = voxels.numWidth / 2.f;
  auto heightOffset = voxels.numHeight / 2.f;
  auto depthOffset = voxels.numDepth / 2.f;
  for (int x = 0; x < voxels.numWidth; x++){
    for (int y = 0; y < voxels.numHeight; y++){
      for (int z = 0; z < voxels.numDepth; z++){
        if (voxels.cubes.at(x).at(y).at(z) == 1){
          VoxelBody body = {
            .position = glm::vec3(x - widthOffset, y - heightOffset, z - depthOffset),
            .textureId = voxels.textures.at(x).at(y).at(z),
          };
          bodies.push_back(body);
        }
      }
    }
  }
  return bodies;
}

VoxelChunkFragment getVoxelChunk(bool& _chunkHasBlocks, Voxels& fromVoxel, int chunksize, int chunkx, int chunky, int chunkz){
  _chunkHasBlocks = false; 
  std::vector<std::vector<std::vector<int>>> cubes;
  std::vector<std::vector<std::vector<unsigned int>>> textures;
  for (int x = 0; x < chunksize; x++){
    std::vector<std::vector<int>> yzplane;
    std::vector<std::vector<unsigned int>> yzplaneTexs;
    for (int y = 0; y < chunksize; y++){
      std::vector<int> zValues;
      std::vector<unsigned int> zValuesTexs;
      for (int z = 0; z < chunksize; z++){
        zValues.push_back(0);
        zValuesTexs.push_back(0);
      } 
      yzplane.push_back(zValues);
      yzplaneTexs.push_back(zValuesTexs);
    }
    cubes.push_back(yzplane);
    textures.push_back(yzplaneTexs);
  }

  for (int x = 0; x < chunksize; x++){
    for (int y = 0; y < chunksize; y++){
      for (int z = 0; z < chunksize; z++){
        auto voxelValue = fromVoxel.cubes.at(chunksize * chunkx).at(chunksize * chunky).at(chunksize * chunkz);
        _chunkHasBlocks = _chunkHasBlocks || (voxelValue != 0);
        cubes.at(x).at(y).at(z) = voxelValue;
        textures.at(x).at(y).at(z) = voxelValue;
      }
    }
  }
  auto numWidth = chunksize;
  auto numHeight = chunksize;
  auto numDepth = chunksize;
  Voxels voxel {
    .cubes = cubes,
    .textures = textures,
    .numWidth = numWidth,
    .numHeight = numHeight,
    .numDepth = numDepth,
    .boundInfo = generateVoxelBoundInfo(cubes, numWidth, numHeight, numDepth),
    .selectedVoxels = {},
    .onVoxelBoundInfoChanged = []() -> void {
      std::cout << "on voxel bound info change not implemented" << std::endl;
      assert(false);
    },
    .defaultTextureId = fromVoxel.defaultTextureId,

  };

  return VoxelChunkFragment{
    .x = chunkx,
    .y = chunky,
    .z = chunkz,
    .offsetx = 0,
    .offsety = 0,
    .offsetz = 0,
    .voxel = voxel,
  };
}

std::vector<VoxelChunkFragment> splitVoxel(Voxels& voxel, Transformation& voxelTransform, int chunksize){
  std::vector<VoxelChunkFragment> voxelFragments;
  // eg 16 / 4 = 4, 17 / 4 = 5
  auto widthChunks = (voxel.numWidth / chunksize) + ((voxel.numWidth % chunksize) == 0 ? 0 : 1);
  auto heightChunks = (voxel.numHeight / chunksize) + ((voxel.numHeight % chunksize) == 0 ? 0 : 1);
  auto depthChunks = (voxel.numDepth / chunksize) + ((voxel.numDepth % chunksize) == 0 ? 0 : 1);

  std::cout << "split voxel, numchunks: (" << widthChunks << ", " << heightChunks << ", " << depthChunks << ") - chunk size: " << chunksize << " - voxel size (" << voxel.numWidth << ", " << voxel.numHeight <<  ", " << voxel.numDepth << ")" << std::endl;

  for (int x = 0; x < widthChunks; x++){
    for (int y = 0; y < heightChunks; y++){
      for (int z = 0; z < depthChunks; z++){
        bool chunkHasBlocks = false;
        auto voxelFragment = getVoxelChunk(chunkHasBlocks, voxel, chunksize, x, y, z);
        std::cout << "chunk: (" << x << ", " << y << ", " << z << ") - hasblocks? " << chunkHasBlocks << std::endl;
        if (chunkHasBlocks){
          voxelFragments.push_back(voxelFragment);
        }
      }
    }
  }
  return voxelFragments;
}

std::string voxelHashAddress(int x, int y, int z){
  return std::to_string(x) + ":" + std::to_string(y) + ":" + std::to_string(z);
}

std::vector<VoxelChunkFragment> groupVoxelChunks(std::vector<VoxelChunkFragment>& fragments){
  std::map<std::string, std::vector<VoxelChunkFragment>> voxelMapping;
  for (auto &fragment : fragments){
    auto fragHash = voxelHashAddress(fragment.x, fragment.y, fragment.z);
    if (voxelMapping.find(fragHash) == voxelMapping.end()){
      voxelMapping[fragHash] = {};
    }
    voxelMapping.at(fragHash).push_back(fragment);
  }

  std::vector<VoxelChunkFragment> newVoxels;
  for (auto &[_, voxelFragments] : voxelMapping){
    std::vector<Transformation> transformations;
    std::vector<Voxels> voxels;
    for (auto &voxelFragment : voxelFragments){
      transformations.push_back(Transformation{
        .position = glm::vec3(voxelFragment.offsetx, voxelFragment.offsety, voxelFragment.offsetx),
        .scale = glm::vec3(1.f, 1.f, 1.f),
        .rotation = glm::identity<glm::quat>(),
      });
      voxels.push_back(voxelFragment.voxel);
    }
    newVoxels.push_back(VoxelChunkFragment{
      .x = voxelFragments.at(0).x,
      .y = voxelFragments.at(0).y,
      .z = voxelFragments.at(0).z,
      .offsetx = 0,
      .offsety = 0,
      .offsetz = 0,
      .voxel = joinVoxels(voxels, transformations),
    });
  }
  return newVoxels;
}

// for now just join along adjacent x axis as if they were oriented the same way
// in practice it should:
// calculate the relative offset (x,y,z)
// calculate the new size
// resolve potential conflicts by joining 
// constraint: orientation is the same (although could resolve 90 degree turns)
// constaint: scale is the same (although could resolve for multiples)
glm::ivec3 calcVoxelSize(glm::ivec3 offset, Voxels& voxel){
  return glm::ivec3(voxel.numWidth + offset.x, voxel.numHeight + offset.y, voxel.numDepth + offset.z); 
}

glm::ivec3 getTotalSize(std::vector<glm::ivec3>& offsets, std::vector<Voxels>& voxels){
  assert(offsets.size() == voxels.size() && offsets.size() > 0);
  auto totalSize = calcVoxelSize(offsets.at(0), voxels.at(0));
  for (int i = 1; i < offsets.size(); i++){
    auto voxSize = calcVoxelSize(offsets.at(i), voxels.at(i));
    if (voxSize.x > totalSize.x){
      totalSize.x = voxSize.x;
    }
    if (voxSize.y > totalSize.y){
      totalSize.y = voxSize.y;
    }
    if (voxSize.z > totalSize.z){
      totalSize.z = voxSize.z;
    }
  }
  return totalSize;
}

glm::ivec3 transformToPos(Transformation& transform){
  auto trans = glm::ivec3(transform.position.x, transform.position.y, transform.position.z);
  return trans;
}

struct RootTransformInfo {
  glm::ivec3 voxPosOffset;
  glm::vec3 pos;
  glm::vec3 scale;
};

RootTransformInfo getRootTransform(std::vector<Transformation>& transforms){
  assert(transforms.size() >= 0);
  auto rootTrans = transforms.at(0);
  auto voxPosOffset = transformToPos(rootTrans);
  auto rootScale = rootTrans.scale;
  auto rootPos = rootTrans.position;
  for (int i = 1; i < transforms.size(); i++){
    auto trans = transforms.at(i);
    auto transPos = transformToPos(trans);
    if (transPos.x < voxPosOffset.x){
      voxPosOffset.x = transPos.x;
      rootScale.x = trans.scale.x;
      rootPos.x = trans.position.x;
    }
    if (transPos.y < voxPosOffset.y){
      voxPosOffset.y = transPos.y;
      rootScale.y = trans.scale.y;
      rootPos.y = trans.position.y;
    }
    if (transPos.z < voxPosOffset.z){
      voxPosOffset.z = transPos.z;
      rootScale.z = trans.scale.z;
      rootPos.z = trans.position.z;
    }
  }
  return RootTransformInfo {
    .voxPosOffset = voxPosOffset,
    .pos = rootPos,
    .scale = rootScale,
  };
}

bool isUnitAligned(glm::vec3 pos1, glm::vec3 pos2){
  auto xDiff = fmod(abs(pos1.x - pos2.x), 1.0f);
  auto yDiff = fmod(abs(pos1.y - pos2.y), 1.0f); 
  auto zDiff = fmod(abs(pos1.z - pos2.z), 1.0f);
  auto xAligned = aboutEqual(xDiff, 0.f) || aboutEqual(xDiff, 1.f);  // need to check against 1.f since floating point messiness w/ fmod
  auto yAligned = aboutEqual(yDiff, 0.f) || aboutEqual(yDiff, 1.f);
  auto zAligned = aboutEqual(zDiff, 0.f) || aboutEqual(zDiff, 1.f);
  return xAligned && yAligned && zAligned;
}

glm::ivec3 getVoxelOffset(RootTransformInfo rootTransform, Transformation& transform){
  // make sure same rotation
  // If the voxels were different sizes, joining the cubes wouldn't work!
  // (but it would be cool to support multiple multiples of sizes and resampling so you could...)
  if (!aboutEqual(rootTransform.scale, transform.scale)){
    std::cout << "scales need to be equal" << std::endl;
    std::cout << "root scale: " << print(rootTransform.scale) << " but original scale: " << print(transform.scale) << std::endl;
    assert(false);
  }

  if (!isUnitAligned(rootTransform.pos, transform.position)){
    std::cout << "positions must be unit align along interval of mod(a-b, 1) == 0" << std::endl;
    std::cout << print(rootTransform.pos) << " vs " << print(transform.position) << std::endl;
    assert(false);
  }

  return transformToPos(transform) - rootTransform.voxPosOffset;
}


void createCubeContainer(
  glm::ivec3 totalSize,
  int defaultTextureId,
  std::vector<std::vector<std::vector<int>>>& cubes,
  std::vector<std::vector<std::vector<unsigned int>>>& textures
){
  for (int x = 0; x < totalSize.x /*voxel.numWidth + voxelOffset.x*/; x++){
    std::vector<std::vector<int>> yzplane;
    std::vector<std::vector<unsigned int>> yzplaneTexs;
    for (int y = 0; y <  totalSize.y /*voxel.numHeight*/; y++){
      std::vector<int> zvoxs;
      std::vector<unsigned int> zvoxsTexs;
      for (int z = 0; z < totalSize.z /* voxel.numDepth*/; z++){
          //glm::ivec3 offset(0, 0, 0);
        zvoxs.push_back(0);
        zvoxsTexs.push_back(defaultTextureId);
      }
      yzplane.push_back(zvoxs);
      yzplaneTexs.push_back(zvoxsTexs);
    }
    cubes.push_back(yzplane);
    textures.push_back(yzplaneTexs);
  } 
}

void placeVoxelInContainer(
  std::vector<std::vector<std::vector<int>>>& cubes,
  std::vector<std::vector<std::vector<unsigned int>>>& textures,
  Voxels& voxel, 
  glm::ivec3 offset
){
  for (int x = 0; x < voxel.numWidth; x++){
    for (int y = 0; y < voxel.numHeight; y++){
      for (int z = 0; z < voxel.numDepth; z++){
        int xoffset = offset.x + x;
        int yoffset = offset.y + y;
        int zoffset = offset.z + z;
        //std::cout << "setting: (" << xoffset << ", " << y << ", " << z << ") - " << voxel.cubes.at(x).at(y).at(z) << std::endl;
        bool noVoxelInSpace = cubes.at(xoffset).at(yoffset).at(zoffset) == 0;
        if (noVoxelInSpace){
          cubes.at(xoffset).at(yoffset).at(zoffset) = voxel.cubes.at(x).at(y).at(z);
          textures.at(xoffset).at(yoffset).at(zoffset) = voxel.textures.at(x).at(y).at(z);
        }
      }
    }
  }
}

Voxels joinVoxels(std::vector<Voxels>& voxels, std::vector<Transformation>& transforms){
  std::vector<glm::ivec3> offsets;
  auto rootTransform = getRootTransform(transforms);
  for (auto transform : transforms){
    offsets.push_back(getVoxelOffset(rootTransform, transform));
  }
  std::vector<std::vector<std::vector<int>>> cubes;
  std::vector<std::vector<std::vector<unsigned int>>> textures;
  auto totalSize = getTotalSize(offsets, voxels);
  //std::cout << "total size is: " << print(totalSize) << std::endl;
  createCubeContainer(totalSize, voxels.at(0).defaultTextureId, cubes, textures);
  for (int voxelId = 0; voxelId < voxels.size(); voxelId++){
    auto voxel = voxels.at(voxelId);
    placeVoxelInContainer(cubes, textures, voxel, offsets.at(voxelId));    
  }
  int numWidth = cubes.size();
  int numHeight = cubes.size() == 0 ? 0 : cubes.at(0).size();
  int numDepth = (cubes.size() == 0 || cubes.at(0).size() == 0) ? 0 : cubes.at(0).at(0).size();
  Voxels vox = {
    .cubes = cubes,
    .textures = textures,
    .numWidth = numWidth,
    .numHeight = numHeight,
    .numDepth = numDepth,
    .boundInfo = generateVoxelBoundInfo(cubes, numWidth, numHeight, numDepth),
    .selectedVoxels = {},
    .onVoxelBoundInfoChanged = []() -> void {
      assert(false);
    },
    .defaultTextureId = voxels.at(0).defaultTextureId,
  };
  return vox;
}

