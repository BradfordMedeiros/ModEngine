#ifndef MOD_WORLDLOADER
#define MOD_WORLDLOADER

#include <iostream>
#include <vector>
#include <map>
#include "./common/util.h"

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
  float chunkRadius;  

  std::map<std::string, objid> sceneFileToId; 
  std::vector<ChunkAddress> loadedChunks;
};

DynamicLoading createDynamicLoading(float chunkSize);
ChunkLoadingInfo getChunkLoadingInfo(DynamicLoading& world);
std::string chunkAddressToSceneFile(ChunkAddress chunk);
void handleChunkLoading(DynamicLoading& loadingInfo, float x, float y, float z, objid(*loadScene)(std::string sceneFile), void(*unloadScene)(objid sceneId));

#endif
