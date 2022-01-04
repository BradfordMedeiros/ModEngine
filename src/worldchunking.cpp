#include "./worldchunking.h"

std::vector<std::string> voxelsInScene(std::string sceneFile){
  std::vector<std::string> voxelsInScene;
  auto elementsInScene = offlineGetElements(sceneFile);
  for (auto &element : elementsInScene){
    auto objType = getType(element);
    if (objType == "voxel"){
      voxelsInScene.push_back(element);
    }
  }
  return voxelsInScene;
}

std::vector<std::string> getVoxelsForChunkhash(DynamicLoading& loadingInfo, std::string& chunkHash){
  auto hasSceneFile = loadingInfo.mappingInfo.chunkHashToSceneFile.find(chunkHash) != loadingInfo.mappingInfo.chunkHashToSceneFile.end();
  if (hasSceneFile){
    auto sceneFile = loadingInfo.mappingInfo.chunkHashToSceneFile.at(chunkHash);
    return voxelsInScene(sceneFile);
  }
  return {};
}

std::string outputFileForChunkHash(DynamicLoading& loadingInfo, std::string chunkhash){
  return loadingInfo.mappingInfo.chunkHashToSceneFile.at(chunkhash) + "_rechunked.rawscene";
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

fragmentSceneFile scenefileForFragment(DynamicLoading& loadingInfo, std::string& chunkHash, VoxelChunkFragment& fragment){
  bool isValid = false;
  auto chunkAddress = decodeChunkHash(chunkHash, &isValid);
  ChunkAddress addressWithFragment {
    .x = chunkAddress.x + fragment.x,
    .y = chunkAddress.y + fragment.y,
    .z = chunkAddress.z + fragment.z,
  };
  auto encodedHash = encodeChunkHash(addressWithFragment);
  assert(isValid);
  auto fragmentSceneFileExists = hashHasMapping(loadingInfo, encodedHash);
  auto name = fragmentSceneFileExists ? outputFileForChunkHash(loadingInfo, encodedHash) : newChunkFile(addressWithFragment);
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

void writeVoxelToSceneFile(World& world, std::string scenefile, std::string name, Voxels& voxel){
  auto serializedObj = serializeVoxelDefault(world, voxel);
  auto tokens = parseFormat(serializedObj);
  std::vector<std::pair<std::string, std::string>> attrs;
  for (auto &token : tokens){
    attrs.push_back({ token.attribute, token.payload });
  }
  offlineSetElementAttributes(scenefile, name, attrs);
}
void addFragmentToScene(World& world, fragmentSceneFile& fragmentSceneFile, VoxelChunkFragment& voxelFragment, std::string& voxelname, std::string& chunkValue){
  auto elementName = std::string("]voxelfragment_") + voxelname + "_from_" + chunkValue;
  auto elementAlreadyExists = offlineGetElement(fragmentSceneFile.name, elementName).size() > 0;
  auto name = elementAlreadyExists ? (elementName + "_" + std::to_string(getUniqueObjId())) : elementName;
  if (elementAlreadyExists){
    std::cout << "warning: " <<  elementName << " already exists in " << fragmentSceneFile.name << std::endl;
    //std::cout << "source: (hash, file, fragment address): (" <<  chunkHash << ", " << scenefile << ", " << fragmentSceneFile.fragmenthash << ")" << std::endl; 
    assert(offlineGetElement(fragmentSceneFile.name, name).size() == 0);
  }
  writeVoxelToSceneFile(world, fragmentSceneFile.name, name, voxelFragment.voxel);
}

GameObjPair getVoxelFromFile(World& world, SysInterface& interface, std::string& scenefile, std::string& voxelname){
  auto elementTokens = offlineGetElement(scenefile, voxelname);
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
  auto voxels = voxelsInScene(scenefile);
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
  writeVoxelToSceneFile(world, scenefile, "]voxel_joined", voxelData);

  for (auto &fragmentname : fragmentNames){
    offlineRemoveElement(scenefile, fragmentname);
  }
  // then add the joined object
}

void rechunkAllCells(World& world, DynamicLoading& loadingInfo, int newchunksize, SysInterface interface){
  std::cout << "Start: voxel rechunking" << std::endl;
  std::cout << "num chunk hashes: " << loadingInfo.mappingInfo.chunkHashToSceneFile.size() << std::endl;
  std::vector<ChunkAddress> chunks;

  for (auto &[chunkhash, scenefile] : loadingInfo.mappingInfo.chunkHashToSceneFile){
    offlineCopyScene(scenefile, outputFileForChunkHash(loadingInfo, chunkhash));
  } 

  std::set<std::string> filesWithFragments;  // includes elements that were split from the mapping only, maybe it should check bigger set of files
  for (auto &[chunkHash, scenefile] : loadingInfo.mappingInfo.chunkHashToSceneFile){
    std::string chunkValue = chunkHash;
    std::cout << "processing chunk hash: " << chunkHash << std::endl;
    auto voxelnamesInChunk = getVoxelsForChunkhash(loadingInfo, chunkValue);
    for (auto &voxelname : voxelnamesInChunk){
      auto voxelFragments = fragmentsForVoxelname(world, interface, scenefile, voxelname);
      auto sceneFilesForFragments = getSceneFilesForFragments(loadingInfo, chunkValue, voxelFragments);
      ensureAllTargetFilesExist(sceneFilesForFragments);
      std::cout << "Voxel fragments size: " << voxelFragments.size() << std::endl;
      for (int i = 0; i < voxelFragments.size(); i++){
        filesWithFragments.insert(sceneFilesForFragments.at(i).name);
        addFragmentToScene(world, sceneFilesForFragments.at(i), voxelFragments.at(i), voxelname, chunkValue);
      }
      offlineRemoveElement(outputFileForChunkHash(loadingInfo, chunkHash), voxelname);
    }
  }

  for (auto filesWithFragment : filesWithFragments){
    joinFragmentsInScenefile(world, interface, filesWithFragment);
  }
}
