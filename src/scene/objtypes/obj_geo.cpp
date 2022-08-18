#include "./obj_geo.h"

std::vector<glm::vec3> parsePoints(std::string value){
  std::vector<glm::vec3> points;
  auto pointsArr = filterWhitespace(split(value, '|'));
  for (auto point : pointsArr){
    glm::vec3 pointValue(0.f, 0.f, 0.f);
    auto isVec = maybeParseVec(point, pointValue);
    assert(isVec);
    points.push_back(pointValue);
  }
  return points;
}

std::string pointsToString(std::vector<glm::vec3>& points){
  std::string value = "";
  for (int i = 0; i < points.size(); i++){
    auto point = points.at(i);
    value = value + print(point);
    if (i != (points.size() - 1)){
      value = value + "|";
    }
  }
  return value;
}

std::vector<AutoSerialize> geoAutoserializer {
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectGeo, type),
    .enums = { GEODEFAULT, GEOSPHERE },
    .enumStrings = { "default", "sphere" },
    .field = "shape",
    .defaultValue = GEODEFAULT,
  },
  AutoSerializeCustom {
    .structOffset = offsetof(GameObjectGeo, points),
    .field = "points",
    .fieldType = ATTRIBUTE_STRING,
    .deserialize = [](void* offset, void* fieldValue) -> void {
      std::vector<glm::vec3>* points = static_cast<std::vector<glm::vec3>*>(offset);
      std::string* pointStr = static_cast<std::string*>(fieldValue);
      if (pointStr == NULL){
        *points = {};
      }else{
        *points = parsePoints(*pointStr);
      }
    },
    .setAttributes = [](void* offset, void* fieldValue) -> void {
      std::vector<glm::vec3>* points = static_cast<std::vector<glm::vec3>*>(offset);
      std::string* pointStr = static_cast<std::string*>(fieldValue);
      if (pointStr != NULL){
        *points = parsePoints(*pointStr);
      }
    },
    .getAttribute = [](void* offset) -> AttributeValue {
      std::vector<glm::vec3>* points = static_cast<std::vector<glm::vec3>*>(offset);
      return pointsToString(*points);
    },
  }
};

GameObjectGeo createGeo(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectGeo geo{};
  createAutoSerializeWithTextureLoading((char*)&geo, geoAutoserializer, attr, util);
  return geo;
}

void geoObjAttr(GameObjectGeo& geoObj, GameobjAttributes& _attributes){
  autoserializerGetAttr((char*)&geoObj, geoAutoserializer, _attributes);
}

void setGeoObjAttributes(GameObjectGeo& geoObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  autoserializerSetAttrWithTextureLoading((char*)&geoObj, geoAutoserializer, attributes, util);
}
std::vector<std::pair<std::string, std::string>> serializeGeo(GameObjectGeo obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&obj, geoAutoserializer, pairs);
  return pairs;
}