#ifndef MOD_WORLDLOADER
#define MOD_WORLDLOADER

#include <iostream>
#include <vector>
#include <map>

struct DynamicLoading {
  float entityPosX;
  float entityPosY;
  float entityPosZ;

  float chunkXWidth;
  float chunkYHeight;
  float chunkZDepth;

  std::map<std::string, short> sceneFileToId; 
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

DynamicLoading createDynamicLoading();
ChunkLoadingInfo getChunkLoadingInfo(DynamicLoading& world);
std::string chunkAddressToSceneFile(ChunkAddress chunk);
void handleChunkLoading(DynamicLoading& loadingInfo, float x, float y, float z, short(*loadScene)(std::string sceneFile), void(*unloadScene)(short sceneId));

#endif
