#include "./worldloader.h"

DynamicLoading createDynamicLoading(){
  DynamicLoading loading = {
    .entityPosX = 0.f, 
    .entityPosY = 0.f,
    .entityPosZ = 0.f,
    .chunkXWidth = 100.f, 
    .chunkYHeight = 100.f,
    .chunkZDepth = 100.f,
    .chunkRadius = 2, 
  };
  return loading;
}

std::set<ChunkAddress> getChunksShouldBeLoaded(DynamicLoading& world){
  int chunkX = round(world.entityPosX / world.chunkXWidth);
  int chunkY = round(world.entityPosY / world.chunkYHeight);
  int chunkZ = round(world.entityPosZ / world.chunkZDepth);

  int adjacentChunks = world.chunkRadius -1;   

  std::set<ChunkAddress> chunks;
  for (int x = -adjacentChunks; x <= adjacentChunks; x++){
    for (int y = -adjacentChunks; y <= adjacentChunks; y++){
      for (int z = -adjacentChunks; z <= adjacentChunks; z++){
        ChunkAddress chunk = { .x = x + chunkX, .y = y + chunkY, .z = z + chunkZ };   
        chunks.insert(chunk);
      }
    }
  }

  return chunks;
}
ChunkLoadingInfo getChunkDiff(std::set<ChunkAddress> alreadyLoadedChunks, std::set<ChunkAddress> chunksShouldBeLoaded){  // give this 
  std::set<ChunkAddress> chunksToLoad = chunksShouldBeLoaded;
  std::set<ChunkAddress> chunksToUnload = alreadyLoadedChunks;

  for (auto &chunk : alreadyLoadedChunks){
    chunksToLoad.erase(chunk);
  }
  for (auto &chunk : chunksShouldBeLoaded){
    chunksToUnload.erase(chunk);    
  }

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

  auto chunksShouldBeLoaded = getChunksShouldBeLoaded(loadingInfo);
  auto chunkLoading = getChunkDiff(loadingInfo.loadedChunks, chunksShouldBeLoaded);

  for (auto &chunk : chunkLoading.chunksToLoad){
    std::cout << "INFO: CHUNK MANAGEMENT: load: " << "(" << chunk.x << "," << chunk.y << "," << chunk.z << ")" << std::endl;
    auto sceneFile = chunkAddressToSceneFile(chunk);
    auto sceneId = loadScene(sceneFile);
    loadingInfo.sceneFileToId[sceneFile] = sceneId;
  }

  for (auto &chunk : chunkLoading.chunksToUnload){
    std::cout << "INFO: CHUNK MANAGEMENT: unload: " << "(" << chunk.x << "," << chunk.y << "," << chunk.z << ")" << std::endl;
    auto sceneFile = chunkAddressToSceneFile(chunk);
    short sceneId = loadingInfo.sceneFileToId[sceneFile];
    unloadScene(sceneId);
    loadingInfo.sceneFileToId.erase(sceneFile);
  }

  loadingInfo.loadedChunks = chunksShouldBeLoaded;
}

bool operator<(const ChunkAddress& lhs, const ChunkAddress& rhs){
      return lhs.x < rhs.x || lhs.y < rhs.y || lhs.z < rhs.z;   // based on logic sets do !(a < b) and !(b < a) to test equality
}