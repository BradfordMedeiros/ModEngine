#ifndef MOD_HEIGHTMAP
#define MOD_HEIGHTMAP

#include <string>
#include <iostream>
#include <stb_image.h>
#include <cassert>
#include "../common/mesh.h"

struct HeightMapData {
  float* data;
  int width;
  int height;
};

// have to free this data (probably should just use vectors)
HeightMapData loadAndAllocateHeightmap(std::string heightmapFilePath, int dim);  
MeshData generateHeightmapMeshdata(HeightMapData& heightmap);

#endif