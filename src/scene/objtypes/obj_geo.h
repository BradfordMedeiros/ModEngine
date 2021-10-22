#ifndef MOD_OBJ_GEO
#define MOD_OBJ_GEO

#include <vector>
#include "../../common/util.h"
#include "./obj_util.h"

enum GeoShapeType { GEODEFAULT, GEOSPHERE };
struct GameObjectGeo {
  std::vector<glm::vec3> points;
  GeoShapeType type;
};

GameObjectGeo createGeo(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<glm::vec3> parsePoints(std::string value);

void geoObjAttr(GameObjectGeo& geoObj, GameobjAttributes& _attributes);
void setGeoObjAttributes(GameObjectGeo& geoObj, GameobjAttributes& attributes);

#endif