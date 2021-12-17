#include "./worldchunking.h"


void rechunkAllCells(DynamicLoading& loadingInfo, int newchunksize){
  std::cout << "rechunking placeholder" << std::endl;
  std::cout << "num chunk hashes: " << loadingInfo.chunkHashToSceneId.size() << std::endl;
  std::vector<ChunkAddress> chunks;

  for (auto &[chunkHash, _] : loadingInfo.chunkHashToSceneId){
    bool valid = false;
    std::string chunkValue = chunkHash;
    auto chunk= decodeChunkHash(chunkValue, &valid);
    std::cout << "decoding: " << chunkHash << std::endl;
    assert(valid);
  }
}