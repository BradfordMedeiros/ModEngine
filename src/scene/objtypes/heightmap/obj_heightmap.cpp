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
  createAutoSerializeWithTextureLoading((char*)&obj, heightmapAutoserializer, attr, util);
  auto heightmap = loadAndAllocateHeightmap(obj.mapName, obj.dim);
  auto meshData = generateHeightmapMeshdata(heightmap);
  obj.heightmap = heightmap;
  obj.mesh = util.loadMesh(meshData);
  return obj;
}

void removeHeightmap(GameObjectHeightmap& heightmapObj, ObjectRemoveUtil& util){
  delete[] heightmapObj.heightmap.data;
}

std::optional<AttributeValuePtr> getHeightmapAttribute(GameObjectHeightmap& obj, const char* field){
  modassert(false, "getHeightmapAttribute not yet implemented");
  return std::nullopt;
}
//void heightmapObjAttr(GameObjectHeightmap& heightmapObj, GameobjAttributes& _attributes){
//  autoserializerGetAttr((char*)&heightmapObj, heightmapAutoserializer, _attributes);
//}

/*bool setHeightmapAttributes(GameObjectHeightmap& heightmapObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  auto oldMapName = heightmapObj.mapName;
  auto oldDim = heightmapObj.dim;
  autoserializerSetAttrWithTextureLoading((char*)&heightmapObj, heightmapAutoserializer, attributes, util);
  auto newMapName = heightmapObj.mapName;
  auto newDim = heightmapObj.dim;
  if ((oldDim != newDim) || (oldMapName != newMapName)){
    util.unloadMesh(heightmapObj.mesh);
    delete[] heightmapObj.heightmap.data;
    auto heightmap = loadAndAllocateHeightmap(heightmapObj.mapName, heightmapObj.dim);
    auto meshData = generateHeightmapMeshdata(heightmap);
    heightmapObj.heightmap = heightmap;
    heightmapObj.mesh = util.loadMesh(meshData);
  }
  return true;
}*/

bool setHeightmapAttribute(GameObjectHeightmap& heightmapObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags& options){
  modassert(false, "set heightmap attribute not hooked up properly");
  options.rebuildPhysics = true;
  return autoserializerSetAttrWithTextureLoading((char*)&heightmapObj, heightmapAutoserializer, field, value, util);
}


std::vector<std::pair<std::string, std::string>> serializeHeightmap(GameObjectHeightmap& heightmapObj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&heightmapObj, heightmapAutoserializer, pairs);
  return pairs;
}

