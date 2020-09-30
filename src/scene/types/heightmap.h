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
  float minHeight;
  float maxHeight;
};

// have to free this data (probably should just use vectors)
HeightMapData loadAndAllocateHeightmap(std::string heightmapFilePath, int dim);  
MeshData generateHeightmapMeshdata(HeightMapData& heightmap);

struct HeightmapMask {
  float* values;
  int width;
  int height;
};
void applyMasking(
  HeightMapData& heightmap, 
  int x, 
  int y, 
  HeightmapMask mask, 
  float amount, 
  std::function<void()> recalcPhysics,
  Mesh& mesh 
);

#endif