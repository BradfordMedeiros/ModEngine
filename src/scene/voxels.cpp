#include "./voxels.h"

Voxels createVoxels(int width, int height, int depth, int numWidth, int numHeight, int numDepth){
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

static const int numElements = 180;
static float cubes[numElements] = {
  // Front face
  -1.f, 1.f, -1, 0, 1.f, 
  -1.f, -1.f, -1, 0, 0,
  1.f, -1.f, -1, 1.f, 0.f,
  -1.f, 1.f, -1, 0, 1.f,
  1.f, -1.f, -1, 1.f, 0,
  1.f, 1.f, -1, 1.f, 1.f,

  // Back face
  -1.f, 1.f, 1, 0, 1.f, 
  -1.f, -1.f, 1, 0, 0,
  1.f, -1.f, 1, 1.f, 0.f,
  -1.f, 1.f, 1, 0, 1.f,
  1.f, -1.f, 1, 1.f, 0,
  1.f, 1.f, 1, 1.f, 1.f,

  // Left face
  -1.f, 1.f, -1.f, 0, 1.f, 
  -1.f, -1.f, -1.f, 0, 0,
  -1.f, -1.f, 1.f, 1.f, 0.f,
  -1.f, 1.f,  -1.f, 0, 1.f,
  -1.f, -1.f, 1.f, 1.f, 0,
  -1.f, 1.f, 1.f, 1.f, 1.f,

  // Right face
  1.f, 1.f, -1.f, 0, 1.f, 
  1.f, -1.f, -1.f, 0, 0,
  1.f, -1.f, 1.f, 1.f, 0.f,
  1.f, 1.f,  -1.f, 0, 1.f,
  1.f, -1.f, 1.f, 1.f, 0,
  1.f, 1.f, 1.f, 1.f, 1.f,

  // Top face
  1.f, 1.f, -1.f, 0, 1.f, 
  -1.f, 1.f, -1.f, 0, 0,
  -1.f, 1.f, 1.f, 1.f, 0.f,
  1.f, 1.f,  -1.f, 0, 1.f,
  -1.f, 1.f, 1.f, 1.f, 0,
  1.f, 1.f, 1.f, 1.f, 1.f,

  // Bottom face
  1.f, -1.f, -1.f, 0, 1.f, 
  -1.f, -1.f, -1.f, 0, 0,
  -1.f, -1.f, 1.f, 1.f, 0.f,
  1.f, -1.f,  -1.f, 0, 1.f,
  -1.f, -1.f, 1.f, 1.f, 0,
  1.f, -1.f, 1.f, 1.f, 1.f,
};
void addCube(std::vector<float>& vertexData, std::vector<unsigned int>& indicies){
  for (int i = 0; i < numElements; i++){
    vertexData.push_back(cubes[i]);
  }
  for (int i = 0; i < vertexData.size(); i++){
    indicies.push_back(i);
  }
}

VoxelRenderData generateRenderData(Voxels& chunk){
  std::vector<float> vertexData;
  std::vector<unsigned int> indicies;
  addCube(vertexData, indicies);
  VoxelRenderData data = {
    .verticesAndTexCoords = vertexData,    // A voxel render must use one texture (assumption for now)
    .indicies = indicies,
  };
  return data;
}

