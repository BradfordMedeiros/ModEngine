#ifndef MOD_VOXELS
#define MOD_VOXELS

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <iostream>
#include <functional>
#include "../common/mesh.h"
#include "../../common/util.h"

// @TODO instancing/gpu rendering/marching cubes etc all good stuff, but just want an implementation for now

struct VoxelAddress {
  int x;
  int y; 
  int z;
  int face;
};

struct Voxels {
  std::vector<std::vector<std::vector<int>>> cubes;
  std::vector<std::vector<std::vector<unsigned int>>> textures;
  int numWidth;
  int numHeight;
  int numDepth;
  BoundInfo boundInfo;
  std::vector<VoxelAddress> selectedVoxels;
  std::function<void()> onVoxelBoundInfoChanged;
  unsigned int defaultTextureId;
};

struct VoxelState {
  int numWidth;
  int numHeight;
  int numDepth;
  std::vector<std::vector<std::vector<int>>> cubes;
  std::vector<std::vector<std::vector<unsigned int>>> textures;
};

Voxels createVoxels(VoxelState initialState, std::function<void()> onVoxelBoundInfoChanged, unsigned int defaultTexture);
VoxelState parseVoxelState(std::string voxelState, unsigned int defaultTexture);
void addVoxel(Voxels& chunk, int x, int y, int z, bool callOnChanged = true);
void removeVoxel(Voxels& chunk, std::vector<VoxelAddress> voxels);   
void applyTextureToCube(Voxels& chunk, std::vector<VoxelAddress> voxels, int textureId);
std::vector<VoxelAddress> raycastVoxels(Voxels& chunk, glm::vec3 rayPosition, glm::vec3 rayDirection);
void expandVoxels(Voxels& voxel, int x, int y, int z);
std::vector<VoxelBody> getVoxelBodies(Voxels& voxels);

#endif