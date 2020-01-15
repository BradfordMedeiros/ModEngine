#ifndef MOD_VOXELS
#define MOD_VOXELS

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <iostream>
#include "./common/mesh.h"
#include "../common/util.h"

// @TODO instancing/gpu rendering/marching cubes etc all good stuff, but just want an implementation for now

struct VoxelAddress {
  int x;
  int y; 
  int z;
  int face;
};

struct Voxels {
  std::vector<std::vector<std::vector<int>>> cubes;
  int numWidth;
  int numHeight;
  int numDepth;
};

struct VoxelRenderData {
  std::vector<float> verticesAndTexCoords;    
  std::vector<unsigned int> indicies;
  std::string textureFilePath;
};

Voxels createVoxels(int numWidth, int numHeight, int numDepth);
void addVoxel(Voxels& chunk, Mesh& voxelMesh, int x, int y, int z);
void removeVoxel(Voxels& chunk, Mesh& voxelMesh, int x, int y, int z);   
void applyTexture(Voxels& chunk, Mesh& voxelMesh, int x, int y, int z, int face, int textureId);
void applyTextureToCube(Voxels& chunk, Mesh& voxelMesh, int x, int y, int z, int textureId);
VoxelRenderData generateRenderData(Voxels& chunk);
Mesh generateVoxelMesh(VoxelRenderData& renderData);

std::vector<VoxelAddress> raycastVoxels(Voxels& chunk, glm::vec3 rayPosition, glm::vec3 rayDirection);

#endif