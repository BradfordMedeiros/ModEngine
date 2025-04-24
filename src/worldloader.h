#ifndef MOD_WORLDLOADER
#define MOD_WORLDLOADER

#include <iostream>
#include <functional>
#include <map>
#include "./common/util.h"
#include "./common/files.h"

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
  std::unordered_map<std::string, std::string> chunkHashToSceneFile;
};

struct DynamicLoading {
  ChunkMappingInfo mappingInfo;
  float chunkRadius;  

  std::unordered_map<std::string, objid> chunkHashToSceneId;
  std::vector<ChunkAddress> loadedChunks;

  std::unordered_map<objid, objid> loadingHandleToId;
  std::unordered_map<objid, int> idsLoadAround;
};

DynamicLoading createDynamicLoading(std::string chunkfile, std::function<std::string(std::string)> readFile);
ChunkLoadingInfo getChunkLoadingInfo(DynamicLoading& world);
std::string serializeChunkMappingInfo(ChunkMappingInfo& mappingInfo);
void saveChunkMappingInfo(DynamicLoading& world, std::string filepath);
void handleChunkLoading(
  DynamicLoading& loadingInfo, 
  std::function<glm::vec3(objid)> getPos, 
  objid(*loadScene)(std::string sceneFile, glm::vec3 offset, std::string parentNode), 
  void(*unloadScene)(objid sceneId),
  glm::vec3* additionalLoadAround
);
objid addLoadingAround(DynamicLoading& loadingInfo, objid id);
void removeLoadingAround(DynamicLoading& loadingInfo, objid loadingHandle);
ChunkAddress decodeChunkHash(std::string chunkhash, bool* out_validResult);
std::string encodeChunkHash(ChunkAddress chunk);
bool chunkAddressEqual(ChunkAddress& address1, ChunkAddress& address2);
ChunkAddress chunkAddressForPos(glm::vec3 pos, int chunksize);

#endif
