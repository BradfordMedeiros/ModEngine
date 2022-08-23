#ifndef MOD_OBJ_UISLIDER
#define MOD_OBJ_UISLIDER

#include "../../common/util.h"
#include "./obj_util.h"

struct GameObjectUISlider {
  GameObjectUICommon common;
  float min;
  float max;
  float percentage;
  TextureLoadingData texture;
  TextureLoadingData opacityTexture;
  std::string onSlide;
  bool showBackpanel;
  glm::vec4 backpanelTint;
  
  std::optional<glm::vec2> uvCoord;
};

GameObjectUISlider createUISlider(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeSlider(GameObjectUISlider& obj, ObjectSerializeUtil& util);
void getUISliderAttributes(GameObjectUISlider& sliderObj, GameobjAttributes& _attributes);
bool setUISliderAttributes(GameObjectUISlider& sliderObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

#endif