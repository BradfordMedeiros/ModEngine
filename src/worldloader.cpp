#include "./worldloader.h"

// @TODO currently dynamic chunkloading assumes it has exclusive access to scene management (or at least that loadscene/unload scene gives it that).
// This causes the use case of, for example, a user loading a scene in code manually to have this unload the scene if the scene file happens to match that, which is questionable behavior
DynamicLoading createDynamicLoading(float chunkSize){
  DynamicLoading loading = {
    .entityPosX = 0.f, 
    .entityPosY = 0.f,
    .entityPosZ = 0.f,
    .chunkXWidth = chunkSize, 
    .chunkYHeight = chunkSize,
    .chunkZDepth = chunkSize,
    .chunkRadius = 2, 
  };
  return loading;
}

std::vector<ChunkAddress> getChunksShouldBeLoaded(DynamicLoading& world){
  int chunkX = round(world.entityPosX / world.chunkXWidth);
  int chunkY = round(world.entityPosY / world.chunkYHeight);
  int chunkZ = round(world.entityPosZ / world.chunkZDepth);

  int adjacentChunks = world.chunkRadius -1;   

  std::vector<ChunkAddress> chunks;
  for (int x = -adjacentChunks; x <= adjacentChunks; x++){
    for (int y = -adjacentChunks; y <= adjacentChunks; y++){
      for (int z = -adjacentChunks; z <= adjacentChunks; z++){
        ChunkAddress chunk = { .x = x + chunkX, .y = y + chunkY, .z = z + chunkZ };   
        chunks.push_back(chunk);
      }
    }
  }

  return chunks;
}

bool chunksEqual(ChunkAddress chunk1, ChunkAddress chunk2){
  return chunk1.x == chunk2.x && chunk1.y == chunk2.y && chunk1.z == chunk2.z;
}
ChunkLoadingInfo getChunkDiff(std::vector<ChunkAddress> alreadyLoadedChunks, std::vector<ChunkAddress> chunksShouldBeLoaded){  // give this 
  std::vector<ChunkAddress> chunksToLoad;
  std::vector<ChunkAddress> chunksToUnload;

  for (auto &chunk : chunksShouldBeLoaded){                   // chunksToLoad is set(chunksShouldBeLoaded) - set(alreadyLoadedChunks)
    bool foundChunk = false;
    for (auto &alreadyLoadedChunk : alreadyLoadedChunks){
      if (chunksEqual(alreadyLoadedChunk, chunk)){
        foundChunk = true;
        break;
      }
    }
    if (!foundChunk){
      chunksToLoad.push_back(chunk);
    }
  }

  for (auto &chunk : alreadyLoadedChunks){                   // chunksToUnload is set(alreadyLoadedChunks) - set(chunksThatShouldBeLoadedChunks)
    bool foundChunk = false;
    for (auto &shouldBeLoadedChunk : chunksShouldBeLoaded){
      if (chunksEqual(shouldBeLoadedChunk, chunk)){
        foundChunk = true;
        break;
      }
    }
    if (!foundChunk){
      chunksToUnload.push_back(chunk);
    }
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

void handleChunkLoading(DynamicLoading& loadingInfo, float x, float y, float z, objid(*loadScene)(std::string sceneFile), void(*unloadScene)(objid sceneId)){
  loadingInfo.entityPosX = x;
  loadingInfo.entityPosY = y;
  loadingInfo.entityPosZ = z;

  auto chunksShouldBeLoaded = getChunksShouldBeLoaded(loadingInfo);
  auto chunkLoading = getChunkDiff(loadingInfo.loadedChunks, chunksShouldBeLoaded);

  for (auto &chunk : chunkLoading.chunksToUnload){
    std::cout << "INFO: CHUNK MANAGEMENT: unload: " << "(" << chunk.x << "," << chunk.y << "," << chunk.z << ")" << std::endl;
    auto sceneFile = chunkAddressToSceneFile(chunk);
    objid sceneId = loadingInfo.sceneFileToId.at(sceneFile);
    std::cout << "INFO: CHUNK MANAGEMENT: want to unload id: " << sceneId << std::endl;
    unloadScene(sceneId);
    loadingInfo.sceneFileToId.erase(sceneFile);
  }
  for (auto &chunk : chunkLoading.chunksToLoad){
    std::cout << "INFO: CHUNK MANAGEMENT: load: " << "(" << chunk.x << "," << chunk.y << "," << chunk.z << ")" << std::endl;
    auto sceneFile = chunkAddressToSceneFile(chunk);
    auto sceneId = loadScene(sceneFile);
    std::cout << "INFO: CHUNK MANAGEMENT: want to load id: " << sceneId << std::endl;
    loadingInfo.sceneFileToId[sceneFile] = sceneId;
  }

  loadingInfo.loadedChunks = chunksShouldBeLoaded;
}

objid addLoadingAround(objid){
  std::cout << "add loading around placeholder" << std::endl;
  return 0;
} 

void removeLoadingAround(objid){
  std::cout << "remove loading around placeholder" << std::endl;
}