#ifndef MOD_OBJ_UILAYOUT
#define MOD_OBJ_UILAYOUT

#include "../../common/util.h"
#include "../common/util/boundinfo.h"
#include "./obj_util.h"

enum UILayoutType { LAYOUT_HORIZONTAL, LAYOUT_VERTICAL };
struct GameObjectUILayout {
  UILayoutType type;
  float spacing;
  std::vector<std::string> elements;
  int order;
  BoundInfo boundInfo;
  glm::vec3 boundOrigin;
  bool showBackpanel;
  glm::vec3 tint;
  float margin;
};

GameObjectUILayout createUILayout(GameobjAttributes& attr, ObjectTypeUtil& util);

#endif