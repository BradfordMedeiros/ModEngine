#ifndef MOD_WORLDLOADER
#define MOD_WORLDLOADER

#include <iostream>
#include <vector>
#include <map>
#include "./common/util/types.h"

struct ChunkAddress {
  int x;
  int y;
  int z;
};

struct ChunkLoadingInfo {
  std::vector<ChunkAddress> chunksToLoad;
  std::vector<ChunkAddress> chunksToUnload;
};

struct DynamicLoading {
  float entityPosX;
  float entityPosY;
  float entityPosZ;

  float chunkXWidth;
  float chunkYHeight;
  float chunkZDepth;
  float chunkRadius;    // how many adjacent chunks to load 0 -> no chunks, 1 -> just chunk you're in,  2 -> chunk you're in + perimenter, 3 -> one more perimeter after, etc 

  std::map<std::string, short> sceneFileToId; 
  std::vector<ChunkAddress> loadedChunks;
};

DynamicLoading createDynamicLoading();
ChunkLoadingInfo getChunkLoadingInfo(DynamicLoading& world);
std::string chunkAddressToSceneFile(ChunkAddress chunk);
void handleChunkLoading(DynamicLoading& loadingInfo, float x, float y, float z, short(*loadScene)(std::string sceneFile), void(*unloadScene)(short sceneId));

#endif
