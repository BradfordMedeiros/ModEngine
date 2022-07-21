#include "./obj_uislider.h"

GameObjectUISlider createUISlider(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto onSlide = attr.stringAttributes.find("onslide") != attr.stringAttributes.end() ? attr.stringAttributes.at("onslide") : "";
  auto percentage = attr.numAttributes.find("slideamount") != attr.numAttributes.end() ? attr.numAttributes.at("slideamount") : 1.f;

  auto backpanelTint = attr.vecAttr.vec4.find("backpaneltint") == attr.vecAttr.vec4.end() ? glm::vec4(1.f, 1.f, 1.f, 1.f) : attr.vecAttr.vec4.at("backpaneltint");
  auto showBackpanel = attr.vecAttr.vec4.find("backpaneltint") != attr.vecAttr.vec4.end();

  GameObjectUISlider obj {
    .common = parseCommon(attr, util.meshes),
    .min = 0.f,
    .max = 100.f,
    .percentage = percentage,
    .texture = util.ensureTextureLoaded("./res/models/controls/slider.png").textureId,
    .opacityTexture = util.ensureTextureLoaded("./res/models/controls/slider_opacity.png").textureId,
    .onSlide = onSlide,
    .showBackpanel = showBackpanel,
    .backpanelTint = backpanelTint,
  };
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
  if (sliderObj.showBackpanel){
    _attributes.vecAttr.vec4["backpaneltint"] = sliderObj.backpanelTint;
  }
}

void setUISliderAttributes(GameObjectUISlider& sliderObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  MODTODO("ui slider - set rest of attributes");
  if (attributes.numAttributes.find("slideamount") != attributes.numAttributes.end()){
    sliderObj.percentage = attributes.numAttributes.at("slideamount");
  }
}
