#include "./worldloader.h"

// @TODO currently dynamic chunkloading assumes it has exclusive access to scene management (or at least that loadscene/unload scene gives it that).
// This causes the use case of, for example, a user loading a scene in code manually to have this unload the scene if the scene file happens to match that, which is questionable behavior
DynamicLoading createDynamicLoading(float chunkSize){
  DynamicLoading loading = {
    .chunkXWidth = chunkSize, 
    .chunkYHeight = chunkSize,
    .chunkZDepth = chunkSize,
    .chunkRadius = 2, 
  };
  return loading;
}

std::vector<ChunkAddress> getChunksShouldBeLoaded(DynamicLoading& world, ChunkAddress position, int chunkRadius){
  int chunkX = round(position.x / world.chunkXWidth);
  int chunkY = round(position.y / world.chunkYHeight);
  int chunkZ = round(position.z / world.chunkZDepth);

  int adjacentChunks = chunkRadius -1;   

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

std::string chunkloadingDebugInfo(ChunkLoadingInfo& info){
  if (info.chunksToLoad.size() == 0 && info.chunksToUnload.size() == 0){
    return "";
  }
  std::string debugInfo = "";
  debugInfo += "should unload: [";
  for (auto chunk : info.chunksToUnload){
    debugInfo += std::string("(") + std::to_string(chunk.x) + "," + std::to_string(chunk.y) + ", " + std::to_string(chunk.z) + ") ";
  }
  debugInfo += "] \n";
  debugInfo += "should load: [";
  for (auto chunk : info.chunksToLoad){
    debugInfo += std::string("(") + std::to_string(chunk.x) + "," + std::to_string(chunk.y) + ", " + std::to_string(chunk.z) + ") ";
  }
  debugInfo += "]\n";
  return debugInfo;
}

std::string chunkAddressToSceneFile(ChunkAddress chunk){
  return std::string("./res/scenes/chunks/") + std::to_string(chunk.x)  + "." + std::to_string(chunk.y) + "." + std::to_string(chunk.z);
}

void handleChunkLoading(DynamicLoading& loadingInfo, float x, float y, float z, objid(*loadScene)(std::string sceneFile), void(*unloadScene)(objid sceneId)){
  auto chunksShouldBeLoaded = getChunksShouldBeLoaded(
    loadingInfo, 
    ChunkAddress { .x = x, .y = y, .z = z }, 
    loadingInfo.chunkRadius
  );
  auto chunkLoading = getChunkDiff(loadingInfo.loadedChunks, chunksShouldBeLoaded);
  auto debugInfo = chunkloadingDebugInfo(chunkLoading);
  if (debugInfo != ""){
    std::cout << debugInfo;
  }

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

objid addLoadingAround(DynamicLoading& loadingInfo, objid id){
  auto loadHandle = getUniqueObjId();
  if (loadingInfo.loadingHandleToId.find(loadHandle) != loadingInfo.loadingHandleToId.end()){
    std::cout << "ERROR: load handle: " << loadHandle << " already exists" << std::endl;
    assert(false);
  }
  loadingInfo.loadingHandleToId[loadHandle] = id;

  if (loadingInfo.idsLoadAround.find(id) == loadingInfo.idsLoadAround.end()){
    loadingInfo.idsLoadAround[id] = 0;
  }
  loadingInfo.idsLoadAround.at(id) = loadingInfo.idsLoadAround.at(id) + 1;
  return loadHandle;
} 

void removeLoadingAround(DynamicLoading& loadingInfo, objid loadingHandle){
  auto id = loadingInfo.idsLoadAround.at(loadingHandle);
  loadingInfo.idsLoadAround.erase(loadingHandle);

  if (loadingInfo.idsLoadAround.find(id) == loadingInfo.idsLoadAround.end()){
    std::cout << "id: " << id << " is not loaded" << std::endl;
    assert(false);
  }
  auto newAmount = loadingInfo.idsLoadAround.at(id) - 1;
  loadingInfo.idsLoadAround.at(id) = newAmount;
  if (newAmount == 0){
    loadingInfo.idsLoadAround.erase(id);
  }
}