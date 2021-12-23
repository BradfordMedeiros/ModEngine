#ifndef MOD_OBJ_HEIGHTMAP
#define MOD_OBJ_HEIGHTMAP

#include "../../common/util.h"
#include "../types/heightmap.h"
#include "./obj_util.h"

struct GameObjectHeightmap{
  HeightMapData heightmap;
  Mesh mesh;
  TextureInformation texture;
};

GameObjectHeightmap createHeightmap(GameobjAttributes& attr, ObjectTypeUtil& util);
void removeHeightmap(GameObjectHeightmap& heightmapObj, ObjectRemoveUtil& util);

#endif