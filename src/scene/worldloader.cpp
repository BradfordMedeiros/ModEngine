#include "./worldloader.h"

DynamicLoading createDynamicLoading(){
  DynamicLoading loading = {
    .entityPosX = 0.f, 
    .entityPosY = 0.f,
    .entityPosZ = 0.f,
    .chunkXWidth = 100.f, 
    .chunkYHeight = 100.f,
    .chunkZDepth = 100.f,
    .chunkRadius = 1, 
  };
  return loading;
}

std::vector<Line> getChunkingLines(DynamicLoading& world){
  std::vector<Line> lines;
  lines.push_back({ 
    fromPos: glm::vec3(0.f, 0.f, 0.f), 
    toPos: glm::vec3(100.f, 100.f, 0.f) 
  });
  return lines;
}

std::set<ChunkAddress> getChunksShouldBeLoaded(DynamicLoading& world){
  int chunkX = world.entityPosX / world.chunkXWidth;
  int chunkY = world.entityPosY / world.chunkYHeight;
  int chunkZ = world.entityPosZ / world.chunkZDepth;

  int chunkOffset = world.chunkRadius / 2;

  std::set<ChunkAddress> chunks;
  for (int x = 0; x < world.chunkRadius; x++){
    for (int y = 0; y < world.chunkRadius; y++){
      for (int z = 0; z < world.chunkRadius; z++){
        ChunkAddress chunk = { .x = x + chunkX - chunkOffset, .y = y + chunkY - chunkOffset, .z = z + chunkZ - chunkOffset };    // not quite right, but want to think about whether or not to center?
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