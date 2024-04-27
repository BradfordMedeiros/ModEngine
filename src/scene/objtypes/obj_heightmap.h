#ifndef MOD_OBJ_HEIGHTMAP
#define MOD_OBJ_HEIGHTMAP

#include "../../common/util.h"
#include "../types/heightmap.h"
#include "./obj_util.h"

struct GameObjectHeightmap{
  std::string mapName;
  int dim;
  
  HeightMapData heightmap;
  Mesh mesh;
  TextureInformation texture;
};

GameObjectHeightmap createHeightmap(GameobjAttributes& attr, ObjectTypeUtil& util);
void removeHeightmap(GameObjectHeightmap& heightmapObj, ObjectRemoveUtil& util);

std::optional<AttributeValuePtr> getHeightmapAttribute(GameObjectHeightmap& obj, const char* field);
bool setHeightmapAttribute(GameObjectHeightmap& heightmapObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&);

std::vector<std::pair<std::string, std::string>> serializeHeightmap(GameObjectHeightmap& obj, ObjectSerializeUtil& util);


#endif