#ifndef MOD_OBJ_UIBUTTON
#define MOD_OBJ_UIBUTTON

#include "../../common/util.h"
#include "./obj_util.h"

struct GameObjectUIButton {
  GameObjectUICommon common;
  bool initialState;
  bool toggleOn;
  bool canToggle;
  TextureLoadingData onTexture;
  TextureLoadingData offTexture;
  std::string onToggleOn;
  std::string onToggleOff;
  bool hasOnTint;
  glm::vec4 onTint;
  glm::vec4 tint;
};

GameObjectUIButton createUIButton(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeButton(GameObjectUIButton& obj, ObjectSerializeUtil& util);

void getUIButtonAttributes(GameObjectUIButton& textObj, GameobjAttributes& _attributes);
bool setUIButtonAttributes(GameObjectUIButton& buttonObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

#endif