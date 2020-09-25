#include "./heightmap.h"

HeightMapData loadAndAllocateHeightmap(std::string heightmapFilePath){
  std::cout << "INFO: LOADING HEIGHTMAP: " << heightmapFilePath << std::endl;
  int width = 20;
  int height = 20;
  float *data = new float[400];
  for (int i =0 ; i < 80; i++){
    for (int j =0; j < 5; j++){
      data[(i * 5) + j] = 0.2f + j * 0.1 + ((i * 5) + j) * 0.01;
    }
  }
  HeightMapData heightmapData {
    .data = data,
    .width = width,
    .height = height,
  };
  return heightmapData;
}
