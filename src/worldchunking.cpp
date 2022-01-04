#include "./worldchunking.h"

std::vector<std::string> getVoxelsForChunkhash(DynamicLoading& loadingInfo, std::string& chunkHash){
  std::vector<std::string> voxelsInScene;
  auto hasSceneFile = loadingInfo.mappingInfo.chunkHashToSceneFile.find(chunkHash) != loadingInfo.mappingInfo.chunkHashToSceneFile.end();
  if (hasSceneFile){
    auto sceneFile = loadingInfo.mappingInfo.chunkHashToSceneFile.at(chunkHash);
    auto elementsInScene = offlineGetElements(sceneFile);
    for (auto &element : elementsInScene){
      auto objType = getType(element);
      if (objType == "voxel"){
        voxelsInScene.push_back(element);
      }
    }
  }
  return voxelsInScene;
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
void addFragmentToScene(World& world, fragmentSceneFile& fragmentSceneFile, VoxelChunkFragment& voxelFragment, std::string& voxelname, std::string& chunkValue){
  auto elementName = std::string("]voxelfragment_") + voxelname + "_from_" + chunkValue;
  auto elementAlreadyExists = offlineGetElement(fragmentSceneFile.name, elementName).size() > 0;
  auto name = elementAlreadyExists ? (elementName + "_" + std::to_string(getUniqueObjId())) : elementName;
  if (elementAlreadyExists){
    std::cout << "warning: " <<  elementName << " already exists in " << fragmentSceneFile.name << std::endl;
    //std::cout << "source: (hash, file, fragment address): (" <<  chunkHash << ", " << scenefile << ", " << fragmentSceneFile.fragmenthash << ")" << std::endl; 
    assert(offlineGetElement(fragmentSceneFile.name, name).size() == 0);
  }
  auto serializedObj = serializeVoxelDefault(world, voxelFragment.voxel);
  auto tokens = parseFormat(serializedObj);
  std::vector<std::pair<std::string, std::string>> attrs;
  for (auto &token : tokens){
    attrs.push_back({ token.attribute, token.payload });
  }
  offlineSetElementAttributes(fragmentSceneFile.name, name, attrs);
}

std::vector<VoxelChunkFragment> fragmentsForVoxelname(World& world, SysInterface& interface, std::string& scenefile, std::string& voxelname){
  std::cout << "splitting: " << voxelname << std::endl;;
  auto elementTokens = offlineGetElement(scenefile, voxelname);
  auto sceneTokensSerialized = serializeSceneTokens(elementTokens);
  auto gameobjPair = createObjectForScene(world, -1, voxelname, sceneTokensSerialized, interface);
  auto voxelObj = std::get_if<GameObjectVoxel>(&gameobjPair.gameobjObj);
  if (voxelObj == NULL){
    std::cout << "is not a voxel" << std::endl;
    assert(false);
  }
  // todo -> probably should be full transformation right
  // but that depends on it being hooked up to scenegraph which this isn't...
  return splitVoxel(voxelObj -> voxel, gameobjPair.gameobj.transformation, 2);
}

void joinFragmentsInScenefile(std::string& scenefile){

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
    joinFragmentsInScenefile(filesWithFragment);
  }



  // then iterate over chunk hash scene files and the join voxels in all scenes

}

     /*   std::cout << "joined voxel data! size = " << voxels.size() << std::endl;
        std::vector<Voxels> voxelBodies;
        std::vector<Transformation> transforms;
        for (auto id : voxels){
          voxelBodies.push_back(getVoxel(world, id).value() -> voxel);
          transforms.push_back(getGameObject(world, id).transformation);
        }
        auto voxelData = joinVoxels(voxelBodies, transforms);*/