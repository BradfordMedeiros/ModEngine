#include "./worldloader.h"

DynamicLoading createDynamicLoading(){
  DynamicLoading loading = {
    .entityPosX = 0.f, 
    .entityPosY = 0.f,
    .entityPosZ = 0.f,
    .chunkXWidth = 100.f, 
    .chunkYHeight = 100.f,
    .chunkZDepth = 100.f,
  };
  return loading;
}

ChunkLoadingInfo getChunkLoadingInfo(DynamicLoading& world){
  std::vector<ChunkAddress> chunksToLoad;
  std::vector<ChunkAddress> chunksToUnload;
  ChunkLoadingInfo info = {
    .chunksToLoad = chunksToLoad,
    .chunksToUnload = chunksToUnload,
  };
  return info;
}

std::string chunkAddressToSceneFile(ChunkAddress chunk){
  return std::string("./res/scenes/chunks/") + std::to_string(chunk.x)  + "." + std::to_string(chunk.y) + "." + std::to_string(chunk.z);
}

void handleChunkLoading(DynamicLoading& loadingInfo, float x, float y, float z, short(*loadScene)(std::string sceneFile), void(*unloadScene)(short sceneId)){
  loadingInfo.entityPosX = x;
  loadingInfo.entityPosY = y;
  loadingInfo.entityPosZ = z;

  auto chunkLoading = getChunkLoadingInfo(loadingInfo);

  for (auto &chunk : chunkLoading.chunksToLoad){
    std::cout << "INFO: load chunk: " << "(" << chunk.x << "," << chunk.y << ", " << chunk.z << ")" << std::endl;
    auto sceneFile = chunkAddressToSceneFile(chunk);
    auto sceneId = loadScene(sceneFile);
    loadingInfo.sceneFileToId[sceneFile] = sceneId;
  }

  for (auto &chunk : chunkLoading.chunksToUnload){
    std::cout << "INFO: unload chunk: " << "(" << chunk.x << "," << chunk.y << ", " << chunk.z << ")" << std::endl;
    auto sceneFile = chunkAddressToSceneFile(chunk);
    short sceneId = loadingInfo.sceneFileToId[sceneFile];
    unloadScene(sceneId);
    loadingInfo.sceneFileToId.erase(sceneFile);
  }
}
