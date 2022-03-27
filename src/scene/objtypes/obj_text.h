#ifndef MOD_OBJ_TEXT
#define MOD_OBJ_TEXT

#include "../../common/util.h"
#include "./obj_util.h"

struct GameObjectUIText {
  std::string value;
  float deltaOffset;
  glm::vec3 tint;
  AlignType align;
  TextWrap wrap;
  TextVirtualization virtualization;
  int maxwidth;
};

GameObjectUIText createUIText(GameobjAttributes& attr, ObjectTypeUtil& util);
void textObjAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes);
void setUITextAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

#endif