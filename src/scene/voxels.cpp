#include "./voxels.h"

static const int numElements = 180;
static float cubes[numElements] = {
  // Front face
  -0.5f, 0.5f, -0.5f, 0, 0.5f, 
  -0.5f, -0.5f, -0.5f, 0, 0,
  0.5f, -0.5f, -0.5f, 0.5f, 0.f,
  -0.5f, 0.5f, -0.5f, 0, 0.5f,
  0.5f, -0.5f, -0.5f, 0.5f, 0,
  0.5f, 0.5f, -0.5f, 0.5f, 0.5f,

  // Back face
  -0.5f, 0.5f, 0.5f, 0, 0.5f, 
  -0.5f, -0.5f, 0.5f, 0, 0,
  0.5f, -0.5f, 0.5f, 0.5f, 0.f,
  -0.5f, 0.5f, 0.5f, 0, 0.5f,
  0.5f, -0.5f, 0.5f, 0.5f, 0,
  0.5f, 0.5f, 0.5f, 0.5f, 0.5f,

  // Left face
  -0.5f, 0.5f, -0.5f, 0, 0.5f, 
  -0.5f, -0.5f, -0.5f, 0, 0,
  -0.5f, -0.5f, 0.5f, 0.5f, 0.f,
  -0.5f, 0.5f,  -0.5f, 0, 0.5f,
  -0.5f, -0.5f, 0.5f, 0.5f, 0,
  -0.5f, 0.5f, 0.5f, 0.5f, 0.5f,

  // Right face
  0.5f, 0.5f, -0.5f, 0, 0.5f, 
  0.5f, -0.5f, -0.5f, 0, 0,
  0.5f, -0.5f, 0.5f, 0.5f, 0.f,
  0.5f, 0.5f,  -0.5f, 0, 0.5f,
  0.5f, -0.5f, 0.5f, 0.5f, 0,
  0.5f, 0.5f, 0.5f, 0.5f, 0.5f,

  // Top face
  0.5f, 0.5f, -0.5f, 0, 0.5f, 
  -0.5f, 0.5f, -0.5f, 0, 0,
  -0.5f, 0.5f, 0.5f, 0.5f, 0.f,
  0.5f, 0.5f,  -0.5f, 0, 0.5f,
  -0.5f, 0.5f, 0.5f, 0.5f, 0,
  0.5f, 0.5f, 0.5f, 0.5f, 0.5f,

  // Bottom face
  0.5f, -0.5f, -0.5f, 0, 0.5f, 
  -0.5f, -0.5f, -0.5f, 0, 0,
  -0.5f, -0.5f, 0.5f, 0.5f, 0.f,
  0.5f, -0.5f,  -0.5f, 0, 0.5f,
  -0.5f, -0.5f, 0.5f, 0.5f, 0,
  0.5f, -0.5f, 0.5f, 0.5f, 0.5f,
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
  Voxels vox = {
    .cubes = cubes,
  };
  return vox;
}

void addVoxel(Voxels& chunk, int x, int y, int z){
  chunk.cubes.at(x).at(y).at(z) = 1;
}
void removeVoxel(Voxels& chunk, int x, int y, int z){
  chunk.cubes.at(x).at(y).at(z) = 0;
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
  for (int i = 0; i < 10; i++){
    for (int j = 0; j < 3; j++){
      for (int k = 0; k < 10; k++){
        addCube(vertexData, indicies, i + 0.5f, j + 0.5f, k + 0.5f);
      }
    }
  }
  VoxelRenderData data = {
    .verticesAndTexCoords = vertexData,    // A voxel render must use one texture (assumption for now)
    .indicies = indicies,
  };
  return data;
}