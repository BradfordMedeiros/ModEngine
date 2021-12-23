#include "./worldchunking.h"


void rechunkAllCells(DynamicLoading& loadingInfo, int newchunksize){
  std::cout << "rechunking placeholder" << std::endl;
  std::cout << "num chunk hashes: " << loadingInfo.chunkHashToSceneId.size() << std::endl;
  std::vector<ChunkAddress> chunks;

  for (auto &[chunkHash, _] : loadingInfo.chunkHashToSceneId){
    bool valid = false;
    std::string chunkValue = chunkHash;
    auto chunk = decodeChunkHash(chunkValue, &valid);
    std::cout << "decoding: " << chunkHash << std::endl;
    assert(valid);

    auto hasSceneFile = loadingInfo.mappingInfo.chunkHashToSceneFile.find(chunkHash) != loadingInfo.mappingInfo.chunkHashToSceneFile.end();
    if (hasSceneFile){
      auto sceneFile = loadingInfo.mappingInfo.chunkHashToSceneFile.at(chunkHash);
      auto elementsInScene = offlineGetElements(sceneFile);
      std::vector<std::string> voxelsInScene;
      for (auto &element : elementsInScene){
        auto objType = getType(element);
        if (objType == "voxel"){
          voxelsInScene.push_back(element);

        }
      }
      // filter elements that are voxels
      // then get the voxel elements + gameobj
      // then split
      // then serialize
      // then offline add to scene
    }
    //

    //auto voxelFragments = splitVoxel(voxels -> voxel, getGameObject(world, objectId).transformation, 2);

  }
}