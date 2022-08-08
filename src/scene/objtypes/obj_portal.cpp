#include "./obj_portal.h"

GameObjectPortal createPortal(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectPortal obj {};
  attrSet(attr, &obj.perspective, "true", "false", false, "perspective", false);
  attrSet(attr, &obj.camera, "", "camera");
  return obj;
}