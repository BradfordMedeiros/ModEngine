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

GameObjectUISlider createUISlider(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectUISlider obj {};
  createAutoSerialize((char*)&obj, uiSliderAutoserializer, attr, util);
  attrSetCommon(attr, obj.common, util.meshes);
  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeSlider(GameObjectUISlider& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  addSerializeCommon(pairs, obj.common);
  if (obj.onSlide != ""){
    pairs.push_back(std::pair<std::string, std::string>("onslide", obj.onSlide));
  }
  if (obj.showBackpanel){
    pairs.push_back(std::pair<std::string, std::string>("backpaneltint", serializeVec(obj.backpanelTint)));
  }
  return pairs;
}


void getUISliderAttributes(GameObjectUISlider& sliderObj, GameobjAttributes& _attributes){
  autoserializerGetAttr((char*)&sliderObj, uiSliderAutoserializer, _attributes);
}

void setUISliderAttributes(GameObjectUISlider& sliderObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  autoserializerSetAttr((char*)&sliderObj, uiSliderAutoserializer, attributes, util);
}
