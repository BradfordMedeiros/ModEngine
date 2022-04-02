#include "./obj_custom.h"

GameObjectCustom createCustom(GameobjAttributes& attr, ObjectTypeUtil& util){
  bool hasCScript = attr.stringAttributes.find("cscript") != attr.stringAttributes.end();
  if (hasCScript){
    util.onCreateCustomElement(util.id, attr.stringAttributes.at("cscript").c_str());
  }
  return GameObjectCustom{};
}

void removeCustom(GameObjectCustom& customObj, ObjectRemoveUtil& util){
  util.onRemoveCustomElement(util.id);
}