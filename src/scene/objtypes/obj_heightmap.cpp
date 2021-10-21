#include "./obj_heightmap.h"

GameObjectHeightmap createHeightmap(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto mapName = attr.stringAttributes.find("map") != attr.stringAttributes.end() ? attr.stringAttributes.at("map") : "";
  auto dim = attr.numAttributes.find("dim") != attr.numAttributes.end() ? attr.numAttributes.at("dim") : -1;
  auto heightmap = loadAndAllocateHeightmap(mapName, dim);
  auto meshData = generateHeightmapMeshdata(heightmap);
  GameObjectHeightmap obj{
    .heightmap = heightmap,
    .mesh = util.loadMesh(meshData),
    .texture = texinfoFromFields(attr, util.ensureTextureLoaded),
  };
  return obj;
}

void removeHeightmap(GameObjectHeightmap& heightmapObj){
  delete[] heightmapObj.heightmap.data;
}
