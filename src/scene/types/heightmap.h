#ifndef MOD_HEIGHTMAP
#define MOD_HEIGHTMAP

#include <string>

struct HeightMapData {
  float* data;
  int width;
  int height;
};

// have to free this data (probably should just use vectors)
HeightMapData loadAndAllocateHeightmap(std::string heightmapFilePath);  

#endif