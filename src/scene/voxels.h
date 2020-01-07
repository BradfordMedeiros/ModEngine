#ifndef MOD_VOXELS
#define MOD_VOXELS

#include <vector>

struct Voxels {
  // 3d array for the mapping
};

void createVoxels(int width, int height, int depth, int numWidth, int numHeight, int numDepth);
void addVoxel(Voxels chunk, int x, int y, int z);
void removeVoxel(Voxels chunk, int x, int y, int z);
void applyTexture(Voxels chunk, int x, int y, int z, int face, int textureId);
int getTexture(Voxels chunk, int x, int y, int z, int face);
std::vector<Voxels> getNestedVoxels(Voxels chunk, int x, int y, int z);
void renderVoxels(Voxels chunk);
void getCollision(Voxels chunk);

#endif