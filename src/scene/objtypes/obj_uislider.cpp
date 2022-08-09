#include "./obj_uislider.h"

std::vector<AutoSerialize> uiSliderAutoserializer {
  AutoSerializeString {
    .structOffset = offsetof(GameObjectUISlider, onSlide),
    .field = "onslide",
    .defaultValue = "",
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUISlider, min),
    .field = "min",
    .defaultValue = 0.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUISlider, max),
    .field = "max",
    .defaultValue = 1.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUISlider, percentage),
    .field = "slideamount",
    .defaultValue = 1.f,
  },
  AutoSerializeTextureLoader {
    .structOffset = offsetof(GameObjectUISlider, texture),
    .field = "texture",
    .defaultValue = "./res/models/controls/slider.png",
  },
  AutoSerializeTextureLoader {
    .structOffset = offsetof(GameObjectUISlider, opacityTexture),
    .field = "opacity-texture",
    .defaultValue = "./res/models/controls/slider_opacity.png",
  },
};

GameObjectUISlider createUISlider(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectUISlider obj {};
  createAutoSerialize((char*)&obj, uiSliderAutoserializer, attr, util);

  attrSetCommon(attr, obj.common, util.meshes);
  attrSet(attr, &obj.backpanelTint, &obj.showBackpanel, glm::vec4(1.f, 1.f, 1.f, 1.f), "backpaneltint");
  //attrSetLoadTexture(attr, util.ensureTextureLoaded, &obj.opacityTexture, , );

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
  MODTODO("ui slider - get rest of attributes");
  _attributes.numAttributes["slideamount"] = sliderObj.percentage;
  _attributes.stringAttributes["onslide"] = sliderObj.onSlide;
  if (sliderObj.showBackpanel){
    _attributes.vecAttr.vec4["backpaneltint"] = sliderObj.backpanelTint;
  }
  _attributes.numAttributes["min"] = sliderObj.min;
  _attributes.numAttributes["max"] = sliderObj.max;
}


void setUISliderAttributes(GameObjectUISlider& sliderObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  MODTODO("ui slider - set rest of attributes");
  attrSet(attributes, &sliderObj.percentage, "slideamount");
  attrSet(attributes, &sliderObj.min, "min");
  attrSet(attributes, &sliderObj.max, "max");
}
