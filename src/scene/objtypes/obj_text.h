#ifndef MOD_OBJ_TEXT
#define MOD_OBJ_TEXT

#include "../../common/util.h"
#include "./obj_util.h"

struct UiTextCursor {
  int cursorIndex;
  bool cursorIndexLeft;
  int highlightLength;
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
  std::string fontFamily;
};

GameObjectUIText createUIText(GameobjAttributes& attr, ObjectTypeUtil& util);
void textObjAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes);
bool setUITextAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);
std::vector<std::pair<std::string, std::string>> serializeText(GameObjectUIText& obj, ObjectSerializeUtil& util);

#endif