#include "./worldchunking.h"

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
void printVoxelInfo(World& world, Voxels& voxelData){
  std::cout << "serialized voxel: \n" << serializeVoxelDefault(world, voxelData) << std::endl;
}

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

void rechunkAllCells(World& world, DynamicLoading& loadingInfo, int newchunksize, SysInterface interface){
  std::cout << "Start: voxel rechunking" << std::endl;
  std::cout << "num chunk hashes: " << loadingInfo.mappingInfo.chunkHashToSceneFile.size() << std::endl;
  std::vector<ChunkAddress> chunks;

  for (auto &[chunkhash, scenefile] : loadingInfo.mappingInfo.chunkHashToSceneFile){
    offlineCopyScene(scenefile, outputFileForChunkHash(loadingInfo, chunkhash));
  } 

  for (auto &[chunkHash, scenefile] : loadingInfo.mappingInfo.chunkHashToSceneFile){
    bool valid = false;
    std::string chunkValue = chunkHash;
    auto chunk = decodeChunkHash(chunkValue, &valid);
    assert(valid);

    std::cout << "processing chunk hash: " << chunkHash << std::endl;
    auto voxelnamesInChunk = getVoxelsForChunkhash(loadingInfo, chunkValue);
    for (auto &voxelname : voxelnamesInChunk){
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
      auto voxelFragments = splitVoxel(voxelObj -> voxel, gameobjPair.gameobj.transformation, 2);
      //auto newVoxels = groupVoxelChunks(voxelFragments);
      std::cout << "Voxel fragments size: " << voxelFragments.size() << std::endl;
      for (auto &voxelFragment : voxelFragments){
        std::cout << "fragment info: " << voxelChunkFragmentInfoToString(voxelFragment) << std::endl;
        printVoxelInfo(world, voxelFragment.voxel);
        //serializeVoxelDefault(world, voxelFragment.voxel);

        auto fragmentSceneFile = scenefileForFragment(loadingInfo, chunkValue, voxelFragment);
        if (!fragmentSceneFile.alreadyExists){
          offlineNewScene(fragmentSceneFile.name);  
        }

        auto elementName = std::string("]voxelfragment_") + voxelname + "_from_" + chunkValue;
        auto elementAlreadyExists = offlineGetElement(fragmentSceneFile.name, elementName).size() > 0;
        auto name = elementAlreadyExists ? (elementName + "_" + std::to_string(getUniqueObjId())) : elementName;
        if (elementAlreadyExists){
          std::cout << "warning: " <<  elementName << " already exists in " << fragmentSceneFile.name << std::endl;
          std::cout << "source: (hash, file, fragment address): (" <<  chunkHash << ", " << scenefile << ", " << fragmentSceneFile.fragmenthash << ")" << std::endl; 
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
      offlineRemoveElement(outputFileForChunkHash(loadingInfo, chunkHash), voxelname);
    }
  }

  // then iterate over chunk hash scene files and the join voxels in all scenes

}