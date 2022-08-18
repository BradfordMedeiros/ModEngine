#include "./obj_uislider.h"

std::vector<AutoSerialize> uiSliderAutoserializer {
  AutoSerializeString {
    .structOffset = offsetof(GameObjectUISlider, onSlide),
    .field = "onslide",
    .defaultValue = "",
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUISlider, min),
    .structOffsetFiller = std::nullopt,
    .field = "min",
    .defaultValue = 0.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUISlider, max),
    .structOffsetFiller = std::nullopt,
    .field = "max",
    .defaultValue = 1.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUISlider, percentage),
    .structOffsetFiller = std::nullopt,
    .field = "slideamount",
    .defaultValue = 1.f,
  },
  AutoSerializeTextureLoader {
    .structOffset = offsetof(GameObjectUISlider, texture),
    .structOffsetName = std::nullopt,
    .field = "texture",
    .defaultValue = "./res/models/controls/slider.png",
  },
  AutoSerializeTextureLoader {
    .structOffset = offsetof(GameObjectUISlider, opacityTexture),
    .structOffsetName = std::nullopt,
    .field = "opacity-texture",
    .defaultValue = "./res/models/controls/slider_opacity.png",
  },
  AutoSerializeVec4 {
    .structOffset = offsetof(GameObjectUISlider, backpanelTint),
    .structOffsetFiller = offsetof(GameObjectUISlider, showBackpanel),
    .field = "backpaneltint",
    .defaultValue = glm::vec4(1.f, 1.f, 1.f, 1.f),
  },
};

static int _ = addCommonAutoserializer<GameObjectUISlider>(uiSliderAutoserializer);

GameObjectUISlider createUISlider(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectUISlider obj {};
  createAutoSerializeWithTextureLoading((char*)&obj, uiSliderAutoserializer, attr, util);
  obj.common.mesh = util.meshes.at("./res/models/controls/input.obj").mesh;
  obj.common.isFocused = false;
  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeSlider(GameObjectUISlider& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&obj, uiSliderAutoserializer, pairs);
  return pairs;
}


void getUISliderAttributes(GameObjectUISlider& sliderObj, GameobjAttributes& _attributes){
  autoserializerGetAttr((char*)&sliderObj, uiSliderAutoserializer, _attributes);
}

void setUISliderAttributes(GameObjectUISlider& sliderObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  autoserializerSetAttrWithTextureLoading((char*)&sliderObj, uiSliderAutoserializer, attributes, util);
}
