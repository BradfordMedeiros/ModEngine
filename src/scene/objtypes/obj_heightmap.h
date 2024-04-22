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
void heightmapObjAttr(GameObjectHeightmap& geoObj, GameobjAttributes& _attributes);

bool setHeightmapAttributes(GameObjectHeightmap& geoObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);
std::vector<std::pair<std::string, std::string>> serializeHeightmap(GameObjectHeightmap& obj, ObjectSerializeUtil& util);


#endif