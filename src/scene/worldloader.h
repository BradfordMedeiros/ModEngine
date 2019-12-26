#ifndef MOD_WORLDLOADER
#define MOD_WORLDLOADER

#include <iostream>
#include <vector>
#include <map>
#include <set>

struct ChunkAddress {
  int x;
  int y;
  int z;
};
bool operator<(const ChunkAddress& lhs, const ChunkAddress& rhs);  // This is for so we can use ChunkAddress with set, and returns if any value is less than (so uniqueness).  

struct ChunkLoadingInfo {
  std::set<ChunkAddress> chunksToLoad;
  std::set<ChunkAddress> chunksToUnload;
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
  std::set<ChunkAddress> loadedChunks;
};

DynamicLoading createDynamicLoading();
ChunkLoadingInfo getChunkLoadingInfo(DynamicLoading& world);
std::string chunkAddressToSceneFile(ChunkAddress chunk);
void handleChunkLoading(DynamicLoading& loadingInfo, float x, float y, float z, short(*loadScene)(std::string sceneFile), void(*unloadScene)(short sceneId));

#endif
