#include "./obj_heightmap.h"

std::vector<AutoSerialize> heightmapAutoserializer {
  AutoSerializeString {
    .structOffset = offsetof(GameObjectHeightmap, mapName),
    .field = "map",
    .defaultValue = "",
  },
  AutoSerializeInt {
    .structOffset =  offsetof(GameObjectHeightmap, dim),
    .field = "dim",
    .defaultValue = -1,
  },
};

static auto _ = addTextureAutoserializer<GameObjectHeightmap>(heightmapAutoserializer);

GameObjectHeightmap createHeightmap(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectHeightmap obj{};
  createAutoSerialize((char*)&obj, heightmapAutoserializer, attr, util);
  auto heightmap = loadAndAllocateHeightmap(obj.mapName, obj.dim);
  auto meshData = generateHeightmapMeshdata(heightmap);
  obj.heightmap = heightmap;
  obj.mesh = util.loadMesh(meshData);
  return obj;
}

void removeHeightmap(GameObjectHeightmap& heightmapObj, ObjectRemoveUtil& util){
  delete[] heightmapObj.heightmap.data;
}

void heightmapObjAttr(GameObjectHeightmap& heightmapObj, GameobjAttributes& _attributes){
  autoserializerGetAttr((char*)&heightmapObj, heightmapAutoserializer, _attributes);
}
void setHeightmapAttributes(GameObjectHeightmap& heightmapObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  autoserializerSetAttr((char*)&heightmapObj, heightmapAutoserializer, attributes, util);
}
std::vector<std::pair<std::string, std::string>> serializeHeightmap(GameObjectHeightmap& heightmapObj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&heightmapObj, heightmapAutoserializer, pairs);
  return pairs;
}

