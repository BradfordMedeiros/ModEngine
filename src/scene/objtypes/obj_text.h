#ifndef MOD_OBJ_TEXT
#define MOD_OBJ_TEXT

#include "../../common/util.h"
#include "./obj_util.h"

struct UiTextCursor {
  int cursorIndex;
  bool cursorIndexLeft;
};

struct GameObjectUIText {
  std::string value;
  float deltaOffset;
  glm::vec4 tint;
  AlignType align;
  TextWrap wrap;
  TextVirtualization virtualization;
  int charlimit;
  UiTextCursor cursor;
};

GameObjectUIText createUIText(GameobjAttributes& attr, ObjectTypeUtil& util);
void textObjAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes);
void setUITextAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

#endif