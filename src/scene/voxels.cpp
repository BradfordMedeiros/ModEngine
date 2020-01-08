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

VoxelRenderData generateRenderData(Voxels& chunk){
  std::vector<float> vertexData;
  vertexData.push_back(-1.0f);
  vertexData.push_back(1.0f);    
  vertexData.push_back(0.0f);
  vertexData.push_back(0.0f);
  vertexData.push_back(1.0f);

  vertexData.push_back(-1.0f);
  vertexData.push_back(-1.0f);
  vertexData.push_back(0.0f);
  vertexData.push_back(0.0f);
  vertexData.push_back(0.0f);
 
  vertexData.push_back(1.0f);
  vertexData.push_back(-1.0f);
  vertexData.push_back(0.0f);
  vertexData.push_back(1.0f);
  vertexData.push_back(0.0f);
 
  vertexData.push_back(1.0f);
  vertexData.push_back(-1.0f);
  vertexData.push_back(0.0f);
  vertexData.push_back(1.0f);
  vertexData.push_back(0.0f);
 
  vertexData.push_back(1.0f);
  vertexData.push_back(1.0f);
  vertexData.push_back(0.0f);
  vertexData.push_back(1.0f);
  vertexData.push_back(1.0f);

  std::vector<unsigned int> indicies;
  indicies.push_back(0);
  indicies.push_back(1);
  indicies.push_back(2);
  indicies.push_back(0);
  indicies.push_back(2);
  indicies.push_back(3);
  
  VoxelRenderData data = {
    .verticesAndTexCoords = vertexData,    // A voxel render must use one texture (assumption for now)
    .indicies = indicies,
  };
  return data;
}

