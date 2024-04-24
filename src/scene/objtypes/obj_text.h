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
std::vector<std::pair<std::string, std::string>> serializeText(GameObjectUIText& obj, ObjectSerializeUtil& util);
std::optional<AttributeValuePtr> getTextAttribute(GameObjectUIText& obj, const char* field);
bool setTextAttribute(GameObjectUIText& obj, const char* field, AttributeValue value, ObjectSetAttribUtil& util);

#endif