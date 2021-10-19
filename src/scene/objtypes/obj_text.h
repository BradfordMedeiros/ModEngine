#ifndef MOD_OBJ_TEXT
#define MOD_OBJ_TEXT

#include "../../common/util.h"

struct GameObjectUIText {
  std::string value;
  float deltaOffset;
};

GameObjectUIText createUIText(GameobjAttributes& attr);
void textObjAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes);

#endif