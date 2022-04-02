#include "./obj_custom.h"

GameObjectCustom createCustom(GameobjAttributes& attr, ObjectTypeUtil& util){
  util.onCreateCustomElement();
  return GameObjectCustom{};
}

void removeCustom(GameObjectCustom& customObj, ObjectRemoveUtil& util){
  util.onRemoveCustomElement();
}