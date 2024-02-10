#include "./worldchunking.h"

std::string originalSceneFileName(DynamicLoading& loadingInfo, std::string chunkhash){
  return loadingInfo.mappingInfo.chunkHashToSceneFile.at(chunkhash);
}
std::string newOutputFileForScene(std::string originalSceneName){
  return originalSceneName + "_rechunked.rawscene";
}
bool hashHasMapping(DynamicLoading& loadingInfo, std::string chunkhash){
  return loadingInfo.mappingInfo.chunkHashToSceneFile.find(chunkhash) != loadingInfo.mappingInfo.chunkHashToSceneFile.end();
}

struct fragmentSceneFile {
  bool alreadyExists;
  std::string name;
  std::string fragmenthash;
};
std::string newChunkFile(ChunkAddress& address){
  return std::string("./res/scenes/world/genscene_" + std::to_string(address.x) + "." + std::to_string(address.y) + "." + std::to_string(address.z)) + "_rechunked.rawscene";
}

std::string finalOutputfileForChunkHash(DynamicLoading& loadingInfo, std::string encodedHash, ChunkAddress& address){
  auto fragmentSceneFileExists = hashHasMapping(loadingInfo, encodedHash);
  auto name = fragmentSceneFileExists ? newOutputFileForScene(originalSceneFileName(loadingInfo, encodedHash)) : newChunkFile(address);
  return name;
}

void ensureAllTargetFilesExist(std::vector<fragmentSceneFile>& sceneFilesForFragments){
  for (auto &fragmentSceneFile : sceneFilesForFragments){
    if (!fragmentSceneFile.alreadyExists){
      offlineNewScene(fragmentSceneFile.name);  
    }
  }
}


std::vector<glm::vec3> getPositionForElements(std::string scenefile, std::vector<std::string>& elements, ChunkAddress& fromChunk, int oldchunksize, std::function<std::string(std::string)> readFile){
  std::vector<glm::vec3> positions;
  glm::vec3 chunkOffset(fromChunk.x * oldchunksize, fromChunk.y * oldchunksize, fromChunk.z * oldchunksize);
  //std::cout << "Chunk offset: " << print(chunkOffset) << std::endl;
  for (auto element : elements){
    auto position = offlineGetElementAttr(scenefile, element, "position", readFile);
    glm::vec3 pos(0.f, 0.f, 0.f);
    bool isPosition = maybeParseVec(position, pos);
    assert(isPosition);
    positions.push_back(pos + chunkOffset);
  }
  return positions;
}

// based on the chunk size, determine the min and max positions based on the chunk size
struct ChunkPositionAddress {
  ChunkAddress address;
  glm::vec3 position;
};

ChunkPositionAddress chunkAddressForPosition(glm::vec3& position, int chunksize){
  auto chunkAddress = chunkAddressForPos(position, chunksize);
  float xPos = position.x - (chunkAddress.x * chunksize);
  float yPos = position.y - (chunkAddress.y * chunksize);
  float zPos = position.z - (chunkAddress.z * chunksize);
  return ChunkPositionAddress{
    .address = chunkAddress,
    .position = glm::vec3(xPos, yPos, zPos),
  };
}
std::vector<ChunkPositionAddress> chunkAddressForPosition(std::vector<glm::vec3>& positions, int chunksize){
  std::vector<ChunkPositionAddress> chunkaddresses;
  for (auto &position : positions){
    chunkaddresses.push_back(chunkAddressForPosition(position, chunksize));
  }
  return chunkaddresses;
}


void removeEmptyScenes(std::map<std::string, std::string>& chunkMapping, std::function<std::string(std::string)> readFile){
  std::vector<std::string> chunkHashToDelete;
  for (auto &[chunkHash, scenefile] : chunkMapping){
    auto hasNoElements = offlineGetElements(scenefile, readFile).size() == 0;
    if (hasNoElements){
      chunkHashToDelete.push_back(chunkHash);
    }
  }
  for (auto &chunkHash : chunkHashToDelete){
    auto sceneFile = chunkMapping.at(chunkHash);
    chunkMapping.erase(chunkHash);
    offlineDeleteScene(sceneFile);
  }
}

std::map<std::string, std::string> getHashesToConsolidateScenefiles(std::map<std::string, std::string>& chunkMapping, std::function<std::string(std::string)> readFile){
  std::map<size_t, std::vector<std::string>> hashedSceneToChunkHashs;
  for (auto &[chunkHash, scenefile] : chunkMapping){
    auto hashedScene = offlineHashSceneContent(scenefile, readFile);
    if (hashedSceneToChunkHashs.find(hashedScene) == hashedSceneToChunkHashs.end()){
      hashedSceneToChunkHashs[hashedScene] = {};
    }
    hashedSceneToChunkHashs.at(hashedScene).push_back(chunkHash);
  }
  std::map<std::string, std::string> chunkHashToNewSceneFiles;
  for (auto &[_, chunkhashs] : hashedSceneToChunkHashs){
    auto firstHashSceneFile = chunkMapping.at(chunkhashs.at(0));
    for (int i = 1; i < chunkhashs.size(); i++){
      chunkHashToNewSceneFiles[chunkhashs.at(i)] = firstHashSceneFile;
    }
  }
  return chunkHashToNewSceneFiles;
}

void consolidateSameScenes(std::map<std::string, std::string>& chunkMapping, std::function<std::string(std::string)> readFile){
  std::cout << "consolidating same scenes" << std::endl;
  auto chunkHashToNewSceneFiles = getHashesToConsolidateScenefiles(chunkMapping, readFile);
  for (auto &[chunkhash, newSceneFile] : chunkHashToNewSceneFiles){
    auto oldSceneFile = chunkMapping.at(chunkhash);
    offlineDeleteScene(oldSceneFile);  
    chunkMapping.at(chunkhash) = newSceneFile;
  }
}

//////////
// still needs if that chunk does not have a scene file, create it
// update the mapping in the mapping file 

void rechunkAllObjects(World& world, DynamicLoading& loadingInfo, int newchunksize){
  std::cout << "rechunk all objects from " << loadingInfo.mappingInfo.chunkSize << " to " << newchunksize << std::endl;

  std::map<std::string, std::string> newChunksMapping;
  for (auto &[chunkHash, scenefile] : loadingInfo.mappingInfo.chunkHashToSceneFile){
    bool valid = false;
    auto fileChunkAddress = decodeChunkHash(chunkHash, &valid);
    assert(valid);
    auto copiedSceneName = newChunkFile(fileChunkAddress);
    newChunksMapping[chunkHash] = copiedSceneName;
    offlineCopyScene(scenefile, copiedSceneName, world.interface.readFile);
  }

  auto oldchunksize = loadingInfo.mappingInfo.chunkSize;
  for (auto &[chunkHash, scenefile] : loadingInfo.mappingInfo.chunkHashToSceneFile){
    bool valid = false;
    auto fileChunkAddress = decodeChunkHash(chunkHash, &valid);
    auto outputSceneFile = newChunksMapping.at(chunkHash);
    assert(valid);
    auto elements = offlineGetElementsNoChildren(scenefile, world.interface.readFile);
    auto positions = getPositionForElements(scenefile, elements, fileChunkAddress, oldchunksize, world.interface.readFile);
    auto chunkPositionAddresses = chunkAddressForPosition(positions, newchunksize);
    for (int i = 0; i < chunkPositionAddresses.size(); i++){
      ChunkPositionAddress& chunkPositionAddress = chunkPositionAddresses.at(i);
      auto elementName = elements.at(i);
      auto chunksEqual = chunkAddressEqual(fileChunkAddress, chunkPositionAddress.address);
      if (!chunksEqual){
        auto encodedTargetChunkHash = encodeChunkHash(chunkPositionAddress.address);
        auto sceneFileToWrite = newChunkFile(chunkPositionAddress.address);
        std::cout << "RECHUNKING: " << scenefile << " moving element: " << elementName << " to " << sceneFileToWrite <<  std::endl;
        if (!offlineSceneExists(sceneFileToWrite)){
          offlineNewScene(sceneFileToWrite);
        }
        newChunksMapping[encodedTargetChunkHash] = sceneFileToWrite;        
        offlineMoveElementAndChildren(outputSceneFile, sceneFileToWrite, elementName, true, world.interface.readFile);
        offlineUpdateElementAttributes(sceneFileToWrite, elementName, {{ "position", serializeVec(chunkPositionAddress.position) }}, world.interface.readFile);
      }else{
        std::cout << "RECHUNKING: " << scenefile << " not moving element: " << elementName << std::endl;
      }
    }
  }



  DynamicLoading newLoading = loadingInfo;
  std::cout << "chunk size: " << newLoading.mappingInfo.chunkSize << std::endl;
  newLoading.mappingInfo.chunkSize = newchunksize;
  removeEmptyScenes(newChunksMapping, world.interface.readFile);
  consolidateSameScenes(newChunksMapping, world.interface.readFile);
  newLoading.mappingInfo.chunkHashToSceneFile = newChunksMapping;
  saveChunkMappingInfo(newLoading, "./res/scenes/chunk_copy.mapping");
  std::cout << "chunk size: " << newLoading.mappingInfo.chunkSize << std::endl;

  // and here probably need to update the names of the new outputs files so the mapping works correctly!
}
