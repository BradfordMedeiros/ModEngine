#include "./obj_uislider.h"

GameObjectUISlider createUISlider(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto onSlide = attr.stringAttributes.find("onslide") != attr.stringAttributes.end() ? attr.stringAttributes.at("onslide") : "";
  auto percentage = attr.numAttributes.find("slideamount") != attr.numAttributes.end() ? attr.numAttributes.at("slideamount") : 100.f;

  GameObjectUISlider obj {
    .common = parseCommon(attr, util.meshes),
    .min = 0.f,
    .max = 100.f,
    .percentage = percentage,
    .texture = util.ensureTextureLoaded("./res/models/controls/slider.png").textureId,
    .opacityTexture = util.ensureTextureLoaded("./res/models/controls/slider_opacity.png").textureId,
    .onSlide = onSlide,
  };
  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeSlider(GameObjectUISlider& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  addSerializeCommon(pairs, obj.common);
  if (obj.onSlide != ""){
    pairs.push_back(std::pair<std::string, std::string>("onslide", obj.onSlide));
  }
  return pairs;
}

void getUISliderAttributes(GameObjectUISlider& sliderObj, GameobjAttributes& _attributes){
  MODTODO("ui slider - get rest of attributes");
  _attributes.numAttributes["slideamount"] = sliderObj.percentage;
}

