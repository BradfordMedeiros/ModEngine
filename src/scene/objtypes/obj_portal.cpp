#include "./obj_portal.h"

GameObjectPortal createPortal(GameobjAttributes& attr, ObjectTypeUtil& util){
  bool hasCamera = attr.stringAttributes.find("camera") != attr.stringAttributes.end();
  auto camera = hasCamera ? attr.stringAttributes.at("camera") : "";
  auto perspective = attr.stringAttributes.find("perspective") != attr.stringAttributes.end() ? attr.stringAttributes.at("perspective") == "true" : false;

  GameObjectPortal obj {
    .camera = camera,
    .perspective = perspective,
  };
  return obj;
}