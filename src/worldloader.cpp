#include "./worldloader.h"

ChunkAddress decodeChunkHash(std::string chunkhash, bool* out_validResult){
  bool isValidResult = false;
  auto parts = split(chunkhash, '.');

  auto chunkX = 0;
  auto chunkY = 0;
  auto chunkZ = 0;
  if (parts.size() == 3){
    chunkX = std::atoi(parts.at(0).c_str());
    chunkY = std::atoi(parts.at(1).c_str());
    chunkZ = std::atoi(parts.at(2).c_str());
    isValidResult = true;
  }
  *out_validResult = isValidResult;
  return ChunkAddress { .x = chunkX, .y = chunkY, .z = chunkZ };
}
std::string encodeChunkHash(ChunkAddress chunk){
  return std::to_string(chunk.x) + "." + std::to_string(chunk.y) + "." + std::to_string(chunk.z);
}

std::string sceneFileForChunk(ChunkMappingInfo& mappingInfo, ChunkAddress& chunk){
  auto chunkHash = encodeChunkHash(chunk);
  if (mappingInfo.chunkHashToSceneFile.find(chunkHash) == mappingInfo.chunkHashToSceneFile.end()){
    return mappingInfo.defaultScene;
  }
  return mappingInfo.chunkHashToSceneFile.at(chunkHash);
}


enum ChunkMappingTokenType { CHUNK_NONE, CHUNK_SIZE, CHUNK_DEFAULT_FILE, CHUNK_MAPPING };
struct ChunkMappingCommand {
  ChunkMappingTokenType type;
  std::string content;
  ChunkAddress chunkAddress;
};

ChunkMappingCommand parseMappingToken(std::string token){
  auto tokens = split(token, ':');
  assert(tokens.size() >= 2);

  auto type = CHUNK_NONE;
  bool isChunkHash = false;
  auto chunkAddress = decodeChunkHash(tokens.at(0), &isChunkHash);
  if (isChunkHash){
    type = CHUNK_MAPPING;
  }else if (tokens.at(0) == "default") {
    type = CHUNK_DEFAULT_FILE;
  }
  else if (tokens.at(0) == "chunksize"){
    type = CHUNK_SIZE;
  }

  ChunkMappingCommand command {
    .type = type,
    .content = tokens.at(1),
    .chunkAddress = chunkAddress,
  };
  return command;
}

ChunkMappingInfo parseChunkMapping(std::string filepath){
  if (filepath == ""){
    return ChunkMappingInfo {};
  }
  auto fileContent = loadFile(filepath);
  std::map<std::string, std::string> chunkHashToSceneFile;

  std::vector<std::string> lines = filterWhitespace(split(fileContent, '\n'));

  int numDefaultFiles = 0;
  std::string defaultScene = ""; 

  int numChunkSizes = 0;
  int chunkSize = 100;

  for(std::string line : lines){
    auto mappingCommand = parseMappingToken(line);
    if (mappingCommand.type == CHUNK_NONE){
      std::cout << "invalid mapping command in: " << filepath << " " << line << std::endl;
      assert(false);
    }else if (mappingCommand.type == CHUNK_SIZE){
      numChunkSizes++;
      chunkSize = std::atoi(mappingCommand.content.c_str());
    }else if (mappingCommand.type == CHUNK_DEFAULT_FILE){
      numDefaultFiles++;
      defaultScene = mappingCommand.content; // should I check that this file exists?
    }else if (mappingCommand.type == CHUNK_MAPPING){
      auto chunkHash = encodeChunkHash(mappingCommand.chunkAddress);
      std::cout << "chunkmapping: " << chunkHash << " - " << mappingCommand.content << std::endl;
      chunkHashToSceneFile[chunkHash] = mappingCommand.content;
    }else{
      assert(false); // default case should never happen
    }
  }

  if (numDefaultFiles != 1){
    std::cout << "mapping file needs a default entry got " << numDefaultFiles << std::endl;
    assert(false);
  }
  if (numChunkSizes != 0 && numChunkSizes != 1){
    std::cout << "mapping file needs exactly 0 or 1 defaultings, got " << numChunkSizes << std::endl;
    assert(false);
  }

  ChunkMappingInfo chunkMapping {
    .chunkSize = chunkSize,
    .defaultScene = defaultScene,
    .chunkHashToSceneFile = chunkHashToSceneFile,
  };
  return chunkMapping;
}

// @TODO currently dynamic chunkloading assumes it has exclusive access to scene management (or at least that loadscene/unload scene gives it that).
// This causes the use case of, for example, a user loading a scene in code manually to have this unload the scene if the scene file happens to match that, which is questionable behavior
DynamicLoading createDynamicLoading(std::string chunkfile){
  auto mappingInfo = parseChunkMapping(chunkfile);
  DynamicLoading loading = {
    .mappingInfo = mappingInfo,
    .chunkRadius = 1, 
  };
  return loading;
}

std::vector<ChunkAddress> getChunksShouldBeLoaded(DynamicLoading& world, int chunkRadius, std::vector<ChunkAddress> loadingPos){
  std::vector<ChunkAddress> allChunks;
  for (auto position : loadingPos){
    int chunkX = position.x;
    int chunkY = position.y;
    int chunkZ = position.z;    
    
    int adjacentChunks = chunkRadius;   

    std::vector<ChunkAddress> chunks;
    for (int x = -adjacentChunks; x <= adjacentChunks; x++){
      for (int y = -adjacentChunks; y <= adjacentChunks; y++){
        for (int z = -adjacentChunks; z <= adjacentChunks; z++){
          ChunkAddress chunk = { .x = x + chunkX, .y = y + chunkY, .z = z + chunkZ };   
          chunks.push_back(chunk);
        }
      }
    }
    for (auto chunk : chunks){
      bool foundChunk = false;
      for (auto alreadyAddedChunk : allChunks){
        if (chunk.x == alreadyAddedChunk.x && chunk.y == alreadyAddedChunk.y && chunk.z == alreadyAddedChunk.z){
          foundChunk = true;
          break;
        }
      }
      if (!foundChunk){
        allChunks.push_back(chunk);
      }
    }
  }
  return allChunks;
}

bool chunksEqual(ChunkAddress chunk1, ChunkAddress chunk2){
  return chunk1.x == chunk2.x && chunk1.y == chunk2.y && chunk1.z == chunk2.z;
}
ChunkLoadingInfo getChunkDiff(std::vector<ChunkAddress> alreadyLoadedChunks, std::vector<ChunkAddress> chunksShouldBeLoaded){  
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

// instead of keeping in memory here which chunks are loaded or not, it's probably better for this to requery the 
// main scene system for a list of loaded chunks
void handleChunkLoading(
  DynamicLoading& loadingInfo, 
  std::function<glm::vec3(objid)> getPos, 
  objid(*loadScene)(std::string sceneFile, glm::vec3 offset, std::string parentNode), 
  void(*unloadScene)(objid sceneId)
){
  std::vector<ChunkAddress> loadingPos;

  for (auto &[id, _] : loadingInfo.idsLoadAround){
    auto pos = getPos(id);
    loadingPos.push_back(ChunkAddress{
      .x = round(pos.x / loadingInfo.mappingInfo.chunkSize), 
      .y = round(pos.y / loadingInfo.mappingInfo.chunkSize), 
      .z = round(pos.z / loadingInfo.mappingInfo.chunkSize),
    });
  }

  auto chunksShouldBeLoaded = getChunksShouldBeLoaded(loadingInfo, loadingInfo.chunkRadius, loadingPos); 
  auto chunkLoading = getChunkDiff(loadingInfo.loadedChunks, chunksShouldBeLoaded);
  auto debugInfo = chunkloadingDebugInfo(chunkLoading);
  if (debugInfo != ""){
    std::cout << debugInfo;
  }

  for (auto &chunk : chunkLoading.chunksToUnload){
    std::cout << "INFO: CHUNK MANAGEMENT: unload: " << "(" << chunk.x << "," << chunk.y << "," << chunk.z << ")" << std::endl;
    auto chunkHash = encodeChunkHash(chunk);
    objid sceneId = loadingInfo.chunkHashToSceneId.at(chunkHash);
    std::cout << "INFO: CHUNK MANAGEMENT: want to unload id: " << sceneId << std::endl;
    unloadScene(sceneId);
    loadingInfo.chunkHashToSceneId.erase(chunkHash);
  }
  for (auto &chunk : chunkLoading.chunksToLoad){
    std::cout << "INFO: CHUNK MANAGEMENT: load: " << "(" << chunk.x << "," << chunk.y << "," << chunk.z << ")" << std::endl;
    auto sceneFile = sceneFileForChunk(loadingInfo.mappingInfo, chunk);
    auto sceneId = loadScene(
      sceneFile, 
      glm::vec3(
        chunk.x * loadingInfo.mappingInfo.chunkSize, 
        chunk.y * loadingInfo.mappingInfo.chunkSize, 
        chunk.z * loadingInfo.mappingInfo.chunkSize
      ),
      std::to_string(chunk.x) + ", " + std::to_string(chunk.y) + ", " + std::to_string(chunk.z)
    );
    std::cout << "INFO: CHUNK MANAGEMENT: want to load id: " << sceneId << std::endl;
    loadingInfo.chunkHashToSceneId[encodeChunkHash(chunk)] = sceneId;
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

