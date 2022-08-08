#include "./obj_uislider.h"

GameObjectUISlider createUISlider(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto showBackpanel = attr.vecAttr.vec4.find("backpaneltint") != attr.vecAttr.vec4.end();
  GameObjectUISlider obj {
    .common = parseCommon(attr, util.meshes),
    .texture = util.ensureTextureLoaded("./res/models/controls/slider.png").textureId,
    .opacityTexture = util.ensureTextureLoaded("./res/models/controls/slider_opacity.png").textureId,
    .showBackpanel = showBackpanel,
  };

  attrSet(attr, &obj.onSlide, "", "onslide");
  attrSet(attr, &obj.backpanelTint, glm::vec4(1.f, 1.f, 1.f, 1.f), "backpaneltint");
  attrSet(attr, &obj.percentage, 1.f, "slideamount");
  attrSet(attr, &obj.min, 0.f, "min");
  attrSet(attr, &obj.max, 1.f, "max");
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
