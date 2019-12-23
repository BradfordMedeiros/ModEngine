#ifndef MOD_WORLDLOADER
#define MOD_WORLDLOADER

#include <iostream>
#include <vector>

struct DynamicWorld {
  float entityPosX;
  float entityPosY;
  float entityPosZ;

  float chunkXWidth;
  float chunkYHeight;
  float chunkZDepth;
};

struct ChunkAddress {
  int x;
  int y;
  int z;
};

struct ChunkLoadingInfo {
  std::vector<ChunkAddress> chunksToLoad;
  std::vector<ChunkAddress> chunksToUnload;
};

void updateEntityPos(DynamicWorld& world, float posx, float posy, float posz);
ChunkLoadingInfo getChunkLoadingInfo(DynamicWorld& world); 

#endif
