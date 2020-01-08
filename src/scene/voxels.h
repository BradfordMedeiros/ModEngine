#ifndef MOD_VOXELS
#define MOD_VOXELS

#include <vector>
#include <string>

// @TODO instancing/gpu rendering/marching cubes etc all good stuff, but just want an implementation for now

struct Voxels {
  std::vector<std::vector<std::vector<int>>> cubes;
};

struct VoxelRenderData {
  std::vector<float> verticesAndTexCoords;    // A voxel render must use one texture (assumption for now)
  std::vector<unsigned int> indicies;
  std::string textureFilePath;
};

Voxels createVoxels(int width, int height, int depth, int numWidth, int numHeight, int numDepth);
void addVoxel(Voxels& chunk, int x, int y, int z);
void removeVoxel(Voxels& chunk, int x, int y, int z);
void applyTexture(Voxels& chunk, int x, int y, int z, int face, int textureId);
int getTexture(Voxels& chunk, int x, int y, int z, int face);
VoxelRenderData generateRenderData(Voxels& chunk);
void getCollision(Voxels& chunk);

#endif