#include "./obj_custom.h"

GameObjectCustom createCustom(GameobjAttributes& attr, ObjectTypeUtil& util){
  util.onCreateCustomElement(util.id, "native:basic_test");
  return GameObjectCustom{};
}

void removeCustom(GameObjectCustom& customObj, ObjectRemoveUtil& util){
  util.onRemoveCustomElement(util.id);
}