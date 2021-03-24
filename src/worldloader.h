#ifndef MOD_WORLDLOADER
#define MOD_WORLDLOADER

#include <iostream>
#include <vector>
#include <functional>
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

struct ChunkMappingInfo {
  int chunkSize;
  std::string defaultScene;
  std::map<std::string, std::string> chunkHashToSceneFile;
};

struct DynamicLoading {
  ChunkMappingInfo mappingInfo;
  float chunkSize;  
  float chunkRadius;  

  std::map<std::string, objid> chunkHashToSceneId;
  std::vector<ChunkAddress> loadedChunks;

  std::map<objid, objid> loadingHandleToId;
  std::map<objid, int> idsLoadAround;
};

DynamicLoading createDynamicLoading();
ChunkLoadingInfo getChunkLoadingInfo(DynamicLoading& world);
void handleChunkLoading(DynamicLoading& loadingInfo, std::function<glm::vec3(objid)> getPos, objid(*loadScene)(std::string sceneFile), void(*unloadScene)(objid sceneId));
objid addLoadingAround(DynamicLoading& loadingInfo, objid id);
void removeLoadingAround(DynamicLoading& loadingInfo, objid loadingHandle);



#endif
