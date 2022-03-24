#ifndef MOD_OBJ_TEXT
#define MOD_OBJ_TEXT

#include "../../common/util.h"
#include "./obj_util.h"

enum UITextWrapType { WRAP_NONE, WRAP_CHARACTERS };
struct UITextWrap {
  UITextWrapType type;
  float wrapamount;
};
struct GameObjectUIText {
  std::string value;
  float deltaOffset;
  glm::vec3 tint;
  AlignType align;
  UITextWrap wrap;
};

GameObjectUIText createUIText(GameobjAttributes& attr, ObjectTypeUtil& util);
void textObjAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes);
void setUITextAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

#endif