#ifndef MOD_OBJ_UIBUTTON
#define MOD_OBJ_UIBUTTON

#include "../../common/util.h"
#include "./obj_util.h"

struct GameObjectUIButton {
  GameObjectUICommon common;
  bool initialState;
  bool toggleOn;
  bool canToggle;
  std::string onTextureString;
  int onTexture;
  std::string offTextureString;
  int offTexture;
  std::string onToggleOn;
  std::string onToggleOff;
  bool hasOnTint;
  glm::vec3 onTint;
};

GameObjectUIButton createUIButton(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeButton(GameObjectUIButton& obj, ObjectSerializeUtil& util);

#endif