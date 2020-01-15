#include "./voxels.h"

// Currently assumes 1 5x5 voxel sheet (resolution independent)
// front(0) back(1) left(2) right(3) bottom(4) top(5)
static const int numElements = 180;     
static float cubes[numElements] = {    
  -0.5f, 0.5f, -0.5f, 0, 0.2f,     
  -0.5f, -0.5f, -0.5f, 0, 0,
  0.5f, -0.5f, -0.5f, 0.2f, 0.f,
  -0.5f, 0.5f, -0.5f, 0, 0.2f,
  0.5f, -0.5f, -0.5f, 0.2f, 0,
  0.5f, 0.5f, -0.5f, 0.2f, 0.2f,

  -0.5f, 0.5f, 0.5f, 0, 0.2f, 
  -0.5f, -0.5f, 0.5f, 0, 0,
  0.5f, -0.5f, 0.5f, 0.2f, 0.f,
  -0.5f, 0.5f, 0.5f, 0, 0.2f,
  0.5f, -0.5f, 0.5f, 0.2f, 0,
  0.5f, 0.5f, 0.5f, 0.2f, 0.2f,

  -0.5f, 0.5f, -0.5f, 0, 0.2f, 
  -0.5f, -0.5f, -0.5f, 0, 0,
  -0.5f, -0.5f, 0.5f, 0.2f, 0.f,
  -0.5f, 0.5f,  -0.5f, 0, 0.2f,
  -0.5f, -0.5f, 0.5f, 0.2f, 0,
  -0.5f, 0.5f, 0.5f, 0.2f, 0.2f,

  0.5f, 0.5f, -0.5f, 0, 0.2f, 
  0.5f, -0.5f, -0.5f, 0, 0,
  0.5f, -0.5f, 0.5f, 0.2f, 0.f,
  0.5f, 0.5f,  -0.5f, 0, 0.2f,
  0.5f, -0.5f, 0.5f, 0.2f, 0,
  0.5f, 0.5f, 0.5f, 0.2f, 0.2f,

  0.5f, 0.5f, -0.5f, 0, 0.2f, 
  -0.5f, 0.5f, -0.5f, 0, 0,
  -0.5f, 0.5f, 0.5f, 0.2f, 0.f,
  0.5f, 0.5f,  -0.5f, 0, 0.2f,
  -0.5f, 0.5f, 0.5f, 0.2f, 0,
  0.5f, 0.5f, 0.5f, 0.2f, 0.2f,

  0.5f, -0.5f, -0.5f, 0, 0.2f, 
  -0.5f, -0.5f, -0.5f, 0, 0,
  -0.5f, -0.5f, 0.5f, 0.2f, 0.f,
  0.5f, -0.5f,  -0.5f, 0, 0.2f,
  -0.5f, -0.5f, 0.5f, 0.2f, 0,
  0.5f, -0.5f, 0.5f, 0.2f, 0.2f,
};

Voxels createVoxels(int numWidth, int numHeight, int numDepth){
  std::vector<std::vector<std::vector<int>>> cubes;         // @TODO this initialization could be done faster 

  for (int row = 0; row < numWidth; row++){
    std::vector<std::vector<int>> cubestack;
    for (int col = 0; col < numHeight; col++){
      std::vector<int> cuberow;
      for (int depth = 0; depth < numDepth; depth++){
        cuberow.push_back(0);
      }
      cubestack.push_back(cuberow);
    }
    cubes.push_back(cubestack);
  }

  VoxelAddress address = {
    .x = 0,
    .y = 0,
    .z = 0,
    .face = 0,
  };
  Voxels vox = {
    .cubes = cubes,
    .numWidth = numWidth,
    .numHeight = numHeight,
    .numDepth = numDepth,
  };
  return vox;
}

void addCube(std::vector<float>& vertexData, std::vector<unsigned int>& indicies, float offsetX, float offsetY, float offsetZ){
  int originalVertexLength  = vertexData.size();
  for (int i = 0; i < numElements; i++){
    vertexData.push_back(cubes[i]);
  }
  for (int i = originalVertexLength; i < (originalVertexLength + numElements); i+=5){
    vertexData[i] += offsetX;
  }
  for (int i = 1 + originalVertexLength; i < (originalVertexLength + numElements); i+=5){
    vertexData[i] += offsetY;
  }
  for (int i = 2 + originalVertexLength; i < (originalVertexLength + numElements); i+=5){
    vertexData[i] += offsetZ;
  }

  int originalIndexLength = indicies.size();
  for (int i = originalIndexLength; i < (originalIndexLength + numElements); i++){
    indicies.push_back(i);
  }
}


VoxelRenderData generateRenderData(Voxels& chunk){
  std::vector<float> vertexData;
  std::vector<unsigned int> indicies;
  for (int x = 0; x < chunk.numWidth; x++){
    for (int y = 0; y < chunk.numHeight; y++){
      for (int z = 0; z < chunk.numDepth; z++){
        addCube(vertexData, indicies, x + 0.5f, y + 0.5f, z + 0.5f);
      }
    }
  }
  VoxelRenderData data = {
    .verticesAndTexCoords = vertexData,    
    .indicies = indicies,
    .textureFilePath = "./res/textures/voxelsheet.png",
  };
  return data;
}

int getVoxelLinearIndex (Voxels& voxels, int x, int y, int z){
  return (x * voxels.numHeight * voxels.numDepth) + (y * voxels.numDepth) + z;
}

Mesh generateVoxelMesh(VoxelRenderData& renderData){
  return loadMeshFrom3Vert2TexCoords(renderData.textureFilePath, renderData.verticesAndTexCoords, renderData.indicies);
}
void applyTexture(Voxels& chunk, Mesh& voxelMesh, int x, int y, int z, int face, int textureId){
  assert(x < chunk.numWidth);
  assert(y < chunk.numHeight);
  assert(z < chunk.numDepth);
  assert(face < 6);
  assert(textureId < 25);

  glBindBuffer(GL_ARRAY_BUFFER, voxelMesh.VBOPointer);
  int voxelNumber = getVoxelLinearIndex(chunk, x, y, z);
  int voxelOffset = voxelNumber * (sizeof(float) * numElements);
  int faceOffset = (sizeof(float) * 5 * 6) * face;
  int textureOffset = (sizeof(float) * 3);
  int fullOffset = voxelOffset + faceOffset + textureOffset;

  float textureX = textureId % 5;
  float textureY = textureId / 5;
  float textureNdiX = textureX * 0.2f;
  float textureNdiY = textureY * 0.2f;

  float newTextureCoords0[2] = { textureNdiX, textureNdiY + 0.2f };
  float newTextureCoords1[2] = { textureNdiX, textureNdiY };
  float newTextureCoords2[2] = { textureNdiX + 0.2f, textureNdiY };
  float newTextureCoords3[2] = { textureNdiX, textureNdiY + 0.2f };
  float newTextureCoords4[2] = { textureNdiX + 0.2f, textureNdiY };
  float newTextureCoords5[2] = { textureNdiX + 0.2f, textureNdiY + 0.2f };

  glBufferSubData(GL_ARRAY_BUFFER, fullOffset, sizeof(newTextureCoords0), &newTextureCoords0); 
  glBufferSubData(GL_ARRAY_BUFFER, fullOffset + sizeof(float) * 5, sizeof(newTextureCoords1), &newTextureCoords1); 
  glBufferSubData(GL_ARRAY_BUFFER, fullOffset + sizeof(float) * 10, sizeof(newTextureCoords2), &newTextureCoords2);
  glBufferSubData(GL_ARRAY_BUFFER, fullOffset + sizeof(float) * 15, sizeof(newTextureCoords3), &newTextureCoords3); 
  glBufferSubData(GL_ARRAY_BUFFER, fullOffset + sizeof(float) * 20, sizeof(newTextureCoords4), &newTextureCoords4); 
  glBufferSubData(GL_ARRAY_BUFFER, fullOffset + sizeof(float) * 25, sizeof(newTextureCoords5), &newTextureCoords5); 
}
void applyTextureToCube(Voxels& chunk, Mesh& voxelMesh, int x, int y, int z, int textureId){
  for (int i = 0; i < 6; i++){
    applyTexture(chunk, voxelMesh, x, y, z, i, textureId);
  }
}

void addVoxel(Voxels& chunk, Mesh& voxelMesh, int x, int y, int z){    
  chunk.cubes.at(x).at(y).at(z) = 1;
  applyTextureToCube(chunk, voxelMesh, x, y, z, 1);
}
void removeVoxel(Voxels& chunk, Mesh& voxelMesh, int x, int y, int z){
  chunk.cubes.at(x).at(y).at(z) = 0;
  applyTextureToCube(chunk, voxelMesh, x, y, z, 0);
}

// This is effectively line drawing.  This has error, but I don't know if it matters
// @todo this function sucks, is imprecise, and can cause infinite loop
std::vector<VoxelAddress> raycastVoxels(Voxels& chunk, glm::vec3 rayPosition, glm::vec3 rayDirection){    
  float magnitudeLine = sqrt(chunk.numWidth * chunk.numWidth + chunk.numHeight * chunk.numHeight + chunk.numDepth * chunk.numDepth);
  glm::vec3 lineEnd = rayPosition + (rayDirection * magnitudeLine);

  glm::vec3 currentPosition = rayPosition;
  glm::vec3 rayIncrement = glm::normalize(rayDirection);

  std::vector<VoxelAddress> addresses;

  int maxIterations = maxvalue(chunk.numWidth, chunk.numHeight, chunk.numDepth);   // hackey hackey see below
  int numIterations = 0;

  while (currentPosition.x < chunk.numWidth && currentPosition.y < chunk.numHeight && currentPosition.z < chunk.numDepth){
    if ((currentPosition.x > 0 && currentPosition.y > 0 && currentPosition.z > 0) && chunk.cubes.at(currentPosition.x).at(currentPosition.y).at(currentPosition.z) == 1){
      VoxelAddress voxel = {
        .x = (int)(currentPosition.x),
        .y = (int)(currentPosition.y),
        .z = (int)(currentPosition.z)
      };
      addresses.push_back(voxel);
    } 
    currentPosition.x += rayIncrement.x;
    currentPosition.y += rayIncrement.y;
    currentPosition.z += rayIncrement.z;
    
    numIterations++;
    if (numIterations > maxIterations){     // hackey code, happens because of negative ray directions that needs to be fixed
      break;
    }
  }
  return addresses;
}