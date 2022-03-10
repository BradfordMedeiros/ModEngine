#include "./obj_uilayout.h"

GameObjectUILayout createUILayout(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto spacing = attr.numAttributes.find("spacing") == attr.numAttributes.end() ? 0.f : attr.numAttributes.at("spacing");
  auto type = attr.stringAttributes.find("type") != attr.stringAttributes.end() && (attr.stringAttributes.at("type") == "vertical") ? LAYOUT_VERTICAL : LAYOUT_HORIZONTAL;
  
  std::vector<std::string> elements = {};
  if (attr.stringAttributes.find("elements") != attr.stringAttributes.end()){
    elements = split(attr.stringAttributes.at("elements"), ',');
  }
  auto showBackpanel = (attr.stringAttributes.find("backpanel") != attr.stringAttributes.end() && attr.stringAttributes.at("backpanel") == "true");
  auto tint = attr.vecAttributes.find("tint") == attr.vecAttributes.end() ? glm::vec3(1.f, 1.f, 1.f) : attr.vecAttributes.at("tint");
  auto margin = attr.numAttributes.find("margin") == attr.numAttributes.end() ? 0.f : attr.numAttributes.at("margin");

  bool hasMinWidth = attr.numAttributes.find("minwidth") != attr.numAttributes.end();
  float amount = hasMinWidth ? attr.numAttributes.at("minwidth") : 0.f;

  UILayoutMinWidth minwidth {
    .hasMinWidth = hasMinWidth,
    .type = hasMinWidth ? UILayoutPercent : UILayoutNone,
    .amount = amount,
  };
  
  BoundInfo boundInfo {
    .xMin = 0, .xMax = 0,
    .yMin = 0, .yMax = 0,
    .zMin = 0, .zMax = 0,
  };
  GameObjectUILayout obj{
    .type = type,
    .spacing = spacing,
    .elements = elements,
    .boundInfo = boundInfo,
    .boundOrigin = glm::vec3(0.f, 0.f, 0.f),
    .showBackpanel = showBackpanel,
    .tint = tint,
    .margin = margin,
    .texture = texinfoFromFields(attr, util.ensureTextureLoaded),
    .minwidth = minwidth,
  };
  return obj;
}

glm::mat4 layoutBackpanelModelTransform(GameObjectUILayout& layoutObj){
  auto boundWidth = layoutObj.boundInfo.xMax - layoutObj.boundInfo.xMin;
  auto boundheight = layoutObj.boundInfo.yMax - layoutObj.boundInfo.yMin;
  auto zFightingBias = glm::vec3(0.f, 0.f, -0.001f);  
  return glm::scale(glm::translate(glm::mat4(1.0f), layoutObj.boundOrigin + zFightingBias), glm::vec3(boundWidth, boundheight, 1.f));
}