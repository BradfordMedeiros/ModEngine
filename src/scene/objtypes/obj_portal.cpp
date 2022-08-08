#include "./obj_portal.h"

GameObjectPortal createPortal(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto perspective = attr.stringAttributes.find("perspective") != attr.stringAttributes.end() ? attr.stringAttributes.at("perspective") == "true" : false;

  GameObjectPortal obj {
    .perspective = perspective,
  };
  attrSet(attr, &obj.camera, "", "camera");
  return obj;
}