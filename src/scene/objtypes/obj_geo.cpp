#include "./obj_geo.h"

std::vector<AutoSerialize> geoAutoserializer {
 
};

GameObjectGeo createGeo(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto points = parsePoints(
    attr.stringAttributes.find("points") != attr.stringAttributes.end() ? 
    attr.stringAttributes.at("points") : 
    ""
  );

  GameObjectGeo geo{
    .points = points,
  };

  createAutoSerialize((char*)&geo, geoAutoserializer, attr, util);

  attrSet(attr, (int*)&geo.type, { GEODEFAULT, GEOSPHERE }, { "default", "sphere" }, GEODEFAULT, "shape", true);
  return geo;
}

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

void geoObjAttr(GameObjectGeo& geoObj, GameobjAttributes& _attributes){
  _attributes.stringAttributes["points"] = pointsToString(geoObj.points);
  if (geoObj.type == GEOSPHERE){   // should show for any shape
   _attributes.stringAttributes["shape"] = "sphere";
  }
}

void setGeoObjAttributes(GameObjectGeo& geoObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  if (attributes.stringAttributes.find("points") != attributes.stringAttributes.end()){
    geoObj.points = parsePoints(attributes.stringAttributes.at("points"));
  } 
}