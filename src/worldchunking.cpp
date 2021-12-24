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

void rechunkAllCells(DynamicLoading& loadingInfo, int newchunksize){
  std::cout << "rechunking placeholder" << std::endl;
  std::cout << "num chunk hashes: " << loadingInfo.chunkHashToSceneId.size() << std::endl;
  std::vector<ChunkAddress> chunks;

  for (auto &[chunkHash, scenefile] : loadingInfo.mappingInfo.chunkHashToSceneFile){
    offlineCopyScene(scenefile, scenefile + "_rechunked.rawscene");
  } 

  std::cout << "voxels: ( ";
  for (auto &[chunkHash, _] : loadingInfo.mappingInfo.chunkHashToSceneFile){
    bool valid = false;
    std::string chunkValue = chunkHash;
    auto chunk = decodeChunkHash(chunkValue, &valid);
    assert(valid);

    auto voxelnamesInChunk = getVoxelsForChunkhash(loadingInfo, chunkValue);
    for (auto &voxelname : voxelnamesInChunk){
      std::cout << voxelname << " ";
    }

      
    
      // then get the voxel elements + gameobj
      // then split
      // then serialize
      // then offline add to scene

    //

    //auto voxelFragments = splitVoxel(voxels -> voxel, getGameObject(world, objectId).transformation, 2);
  }
  std::cout << " )" << std::endl;

}