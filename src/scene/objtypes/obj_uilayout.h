#ifndef MOD_OBJ_UILAYOUT
#define MOD_OBJ_UILAYOUT

#include "../../common/util.h"
#include "../common/util/boundinfo.h"
#include "./obj_util.h"

enum UILayoutType { LAYOUT_HORIZONTAL, LAYOUT_VERTICAL };

enum UILayoutMinWidthType { UILayoutNone, UILayoutPercent };
struct UILayoutMinWidth {
  bool hasMinWidth;
  UILayoutMinWidthType type;
  float amount;
};
struct GameObjectUILayout {
  UILayoutType type;
  float spacing;
  std::vector<std::string> elements;
  BoundInfo boundInfo;
  glm::vec3 boundOrigin;
  bool showBackpanel;
  glm::vec3 tint;
  float margin;
  TextureInformation texture;
  UILayoutMinWidth minwidth;
};

GameObjectUILayout createUILayout(GameobjAttributes& attr, ObjectTypeUtil& util);
glm::mat4 layoutBackpanelModelTransform(GameObjectUILayout& layoutObj);      

#endif