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
std::string scenefileForFragment(DynamicLoading& loadingInfo, std::string& chunkHash, VoxelChunkFragment& fragment){
  bool isValid = false;
  auto chunkAddress = decodeChunkHash(chunkHash, &isValid);
  ChunkAddress addressWithFragment {
    .x = chunkAddress.x + fragment.x,
    .y = chunkAddress.y + fragment.y,
    .z = chunkAddress.z + fragment.z,
  };
  auto encodedHash = encodeChunkHash(addressWithFragment);

  assert(isValid);
  if (!hashHasMapping(loadingInfo, encodedHash)){
    std::cout << "no mapping for: " << encodedHash << std::endl;
    assert(false);
  }
  auto fileForFragment = outputFileForChunkHash(loadingInfo, encodedHash);
  std::cout << "scenefile for fragment: chunkaddress: " << print(glm::vec3(chunkAddress.x, chunkAddress.y, chunkAddress.z)) << " - " << fileForFragment << std::endl;
  return fileForFragment;
}
std::string fragmentElementName(std::string& fromFile){
  return std::string("]voxelfragment_") + fromFile;
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
        offlineSetElementAttributes(
          scenefileForFragment(loadingInfo, chunkValue, voxelFragment), 
          fragmentElementName(scenefile), 
          {{ "test_attr", "test_value" }}
        );

      }
    }

      // then get the voxel elements + gameobj
      // then split
      // then serialize
      // then offline add to scene

    //

    //auto voxelFragments = splitVoxel(voxels -> voxel, getGameObject(world, objectId).transformation, 2);
  }

}