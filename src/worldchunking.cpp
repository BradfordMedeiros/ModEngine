#include "./worldchunking.h"

std::vector<std::string> voxelsInScene(std::string sceneFile, std::function<std::string(std::string)> readFile){
  std::vector<std::string> voxelsInScene;
  auto elementsInScene = offlineGetElements(sceneFile, readFile);
  for (auto &element : elementsInScene){
    auto objType = getType(element);
    if (objType == "voxel"){
      voxelsInScene.push_back(element);
    }
  }
  return voxelsInScene;
}

std::vector<std::string> getVoxelsForChunkhash(DynamicLoading& loadingInfo, std::string& chunkHash, std::function<std::string(std::string)> readFile){
  auto hasSceneFile = loadingInfo.mappingInfo.chunkHashToSceneFile.find(chunkHash) != loadingInfo.mappingInfo.chunkHashToSceneFile.end();
  if (hasSceneFile){
    auto sceneFile = loadingInfo.mappingInfo.chunkHashToSceneFile.at(chunkHash);
    return voxelsInScene(sceneFile, readFile);
  }
  return {};
}

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

fragmentSceneFile scenefileForFragment(DynamicLoading& loadingInfo, std::string& chunkHash, VoxelChunkFragment& fragment){
  bool isValid = false;
  auto chunkAddress = decodeChunkHash(chunkHash, &isValid);
  assert(isValid);
  ChunkAddress addressWithFragment {
    .x = chunkAddress.x + fragment.x,
    .y = chunkAddress.y + fragment.y,
    .z = chunkAddress.z + fragment.z,
  };
  auto encodedHash = encodeChunkHash(addressWithFragment);
  auto name = finalOutputfileForChunkHash(loadingInfo, encodedHash, addressWithFragment);
  return fragmentSceneFile {
    .alreadyExists = offlineSceneExists(name),
    .name = name,
    .fragmenthash = encodedHash,
  };
}
std::vector<fragmentSceneFile> getSceneFilesForFragments(DynamicLoading& loadingInfo, std::string& chunkHash, std::vector<VoxelChunkFragment>& fragments){
  std::vector<fragmentSceneFile> scenefiles;
  for (auto &fragment : fragments){
    scenefiles.push_back(scenefileForFragment(loadingInfo, chunkHash, fragment));
  }
  return scenefiles;
}

void ensureAllTargetFilesExist(std::vector<fragmentSceneFile>& sceneFilesForFragments){
  for (auto &fragmentSceneFile : sceneFilesForFragments){
    if (!fragmentSceneFile.alreadyExists){
      offlineNewScene(fragmentSceneFile.name);  
    }
  }
}

std::string serializeVoxelDefault(World& world, Voxels& voxelData){
 GameObjectVoxel vox {
    .voxel = voxelData,
  };
  auto gameobj = gameObjectFromFields("]default_voxel", -1, {});
  std::vector<std::string> children;
  ObjectSerializeUtil util {
    .textureName = [&world](int id) -> std::string {
      return getTextureById(world, id); // can be not loaded...
    }
  };
  auto additionalFields = serializeVoxel(vox, util);
  auto serializedObj = serializeObjectSandbox(gameobj, -1, -1, additionalFields, children, false, "");
  return serializedObj;
}

void writeVoxelToSceneFile(World& world, std::string scenefile, std::string name, Voxels& voxel, std::function<std::string(std::string)> readFile){
  auto serializedObj = serializeVoxelDefault(world, voxel);
  auto tokens = parseFormat(serializedObj);
  std::vector<std::pair<std::string, std::string>> attrs;
  for (auto &token : tokens){
    attrs.push_back({ token.attribute, token.payload });
  }
  offlineSetElementAttributes(scenefile, name, attrs, readFile);
}
void addFragmentToScene(World& world, fragmentSceneFile& fragmentSceneFile, VoxelChunkFragment& voxelFragment, std::string& voxelname, std::string& chunkValue, std::function<std::string(std::string)> readFile){
  auto elementName = std::string("]voxelfragment_") + voxelname + "_from_" + chunkValue;
  auto elementAlreadyExists = offlineGetElement(fragmentSceneFile.name, elementName, readFile).size() > 0;
  auto name = elementAlreadyExists ? (elementName + "_" + std::to_string(getUniqueObjId())) : elementName;
  if (elementAlreadyExists){
    std::cout << "warning: " <<  elementName << " already exists in " << fragmentSceneFile.name << std::endl;
    //std::cout << "source: (hash, file, fragment address): (" <<  chunkHash << ", " << scenefile << ", " << fragmentSceneFile.fragmenthash << ")" << std::endl; 
    assert(offlineGetElement(fragmentSceneFile.name, name, readFile).size() == 0);
  }
  writeVoxelToSceneFile(world, fragmentSceneFile.name, name, voxelFragment.voxel, readFile);
}

GameObjPair getVoxelFromFile(World& world, SysInterface& interface, std::string& scenefile, std::string& voxelname){
  auto elementTokens = offlineGetElement(scenefile, voxelname, interface.readFile);
  auto sceneTokensSerialized = serializeSceneTokens(elementTokens);
  auto gameobjPair = createObjectForScene(world, -1, voxelname, sceneTokensSerialized, interface);
  return gameobjPair;
}
std::vector<VoxelChunkFragment> fragmentsForVoxelname(World& world, SysInterface& interface, std::string& scenefile, std::string& voxelname){
  std::cout << "splitting: " << voxelname << std::endl;;
  auto gameobjPair = getVoxelFromFile(world, interface, scenefile, voxelname);
  auto voxelObj = std::get_if<GameObjectVoxel>(&gameobjPair.gameobjObj);
  if (voxelObj == NULL){
    std::cout << "is not a voxel" << std::endl;
    assert(false);
  }
  // todo -> probably should be full transformation right
  // but that depends on it being hooked up to scenegraph which this isn't...
  return splitVoxel(voxelObj -> voxel, gameobjPair.gameobj.transformation, 2);
}

void joinFragmentsInScenefile(World& world, SysInterface& interface, std::string& scenefile){
  auto voxels = voxelsInScene(scenefile, interface.readFile);
  std::cout << "joining frags in: " << scenefile << std::endl;

  std::vector<std::string> fragmentNames;
  std::vector<Voxels> voxelBodies;
  std::vector<Transformation> transforms;
  for (auto &voxel : voxels){
    auto voxelPrefix = voxel.substr(0, 15);
    auto isFragment = voxelPrefix == "]voxelfragment_";
    std::cout << "voxel frag: " << voxelPrefix << " : " << isFragment << std::endl;
    if (isFragment){
      fragmentNames.push_back(voxel);
    }
    auto gameobjPair = getVoxelFromFile(world, interface, scenefile, voxel);
    auto voxelObj = std::get_if<GameObjectVoxel>(&gameobjPair.gameobjObj);
    assert(voxelObj != NULL);
    voxelBodies.push_back(voxelObj -> voxel);
    transforms.push_back(gameobjPair.gameobj.transformation);
  }

  auto voxelData = joinVoxels(voxelBodies, transforms);
  writeVoxelToSceneFile(world, scenefile, "]voxel_joined", voxelData, interface.readFile);

  for (auto &fragmentname : fragmentNames){
    offlineRemoveElement(scenefile, fragmentname, interface.readFile);
  }
}

void rechunkAllVoxels(World& world, DynamicLoading& loadingInfo, int newchunksize, SysInterface interface){
  std::cout << "Start: voxel rechunking" << std::endl;
  std::cout << "num chunk hashes: " << loadingInfo.mappingInfo.chunkHashToSceneFile.size() << std::endl;
  std::vector<ChunkAddress> chunks;

  for (auto &[chunkhash, scenefile] : loadingInfo.mappingInfo.chunkHashToSceneFile){
    offlineCopyScene(scenefile, newOutputFileForScene(originalSceneFileName(loadingInfo, chunkhash)), interface.readFile);
  } 

  std::set<std::string> filesWithFragments;  // includes elements that were split from the mapping only, maybe it should check bigger set of files
  for (auto &[chunkHash, scenefile] : loadingInfo.mappingInfo.chunkHashToSceneFile){
    std::string chunkValue = chunkHash;
    std::cout << "processing chunk hash: " << chunkHash << std::endl;
    auto voxelnamesInChunk = getVoxelsForChunkhash(loadingInfo, chunkValue, interface.readFile);
    for (auto &voxelname : voxelnamesInChunk){
      auto voxelFragments = fragmentsForVoxelname(world, interface, scenefile, voxelname);
      auto sceneFilesForFragments = getSceneFilesForFragments(loadingInfo, chunkValue, voxelFragments);
      ensureAllTargetFilesExist(sceneFilesForFragments);
      std::cout << "Voxel fragments size: " << voxelFragments.size() << std::endl;
      for (int i = 0; i < voxelFragments.size(); i++){
        filesWithFragments.insert(sceneFilesForFragments.at(i).name);
        addFragmentToScene(world, sceneFilesForFragments.at(i), voxelFragments.at(i), voxelname, chunkValue, interface.readFile);
      }
      offlineRemoveElement(newOutputFileForScene(originalSceneFileName(loadingInfo, chunkHash)), voxelname, interface.readFile);
    }
  }

  for (auto filesWithFragment : filesWithFragments){
    joinFragmentsInScenefile(world, interface, filesWithFragment);
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

void rechunkAllObjects(World& world, DynamicLoading& loadingInfo, int newchunksize, SysInterface interface){
  std::cout << "rechunk all objects from " << loadingInfo.mappingInfo.chunkSize << " to " << newchunksize << std::endl;

  std::map<std::string, std::string> newChunksMapping;
  for (auto &[chunkHash, scenefile] : loadingInfo.mappingInfo.chunkHashToSceneFile){
    bool valid = false;
    auto fileChunkAddress = decodeChunkHash(chunkHash, &valid);
    assert(valid);
    auto copiedSceneName = newChunkFile(fileChunkAddress);
    newChunksMapping[chunkHash] = copiedSceneName;
    offlineCopyScene(scenefile, copiedSceneName, interface.readFile);
  }

  auto oldchunksize = loadingInfo.mappingInfo.chunkSize;
  for (auto &[chunkHash, scenefile] : loadingInfo.mappingInfo.chunkHashToSceneFile){
    bool valid = false;
    auto fileChunkAddress = decodeChunkHash(chunkHash, &valid);
    auto outputSceneFile = newChunksMapping.at(chunkHash);
    assert(valid);
    auto elements = offlineGetElementsNoChildren(scenefile, interface.readFile);
    auto positions = getPositionForElements(scenefile, elements, fileChunkAddress, oldchunksize, interface.readFile);
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
        offlineMoveElementAndChildren(outputSceneFile, sceneFileToWrite, elementName, true, interface.readFile);
        offlineUpdateElementAttributes(sceneFileToWrite, elementName, {{ "position", serializeVec(chunkPositionAddress.position) }}, interface.readFile);
      }else{
        std::cout << "RECHUNKING: " << scenefile << " not moving element: " << elementName << std::endl;
      }
    }
  }



  DynamicLoading newLoading = loadingInfo;
  std::cout << "chunk size: " << newLoading.mappingInfo.chunkSize << std::endl;
  newLoading.mappingInfo.chunkSize = newchunksize;
  removeEmptyScenes(newChunksMapping, interface.readFile);
  consolidateSameScenes(newChunksMapping, interface.readFile);
  newLoading.mappingInfo.chunkHashToSceneFile = newChunksMapping;
  saveChunkMappingInfo(newLoading, "./res/scenes/chunk_copy.mapping");
  std::cout << "chunk size: " << newLoading.mappingInfo.chunkSize << std::endl;

  // and here probably need to update the names of the new outputs files so the mapping works correctly!
}
