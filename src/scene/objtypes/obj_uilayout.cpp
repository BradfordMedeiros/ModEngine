#include "./obj_uilayout.h"

UILayoutMinSize layoutMinSizeFromAttr(GameobjAttributes& attr, const char* attrname){
  bool hasMinSize = attr.numAttributes.find(attrname) != attr.numAttributes.end();
  float minSizeAmount = hasMinSize ? attr.numAttributes.at(attrname) : 0.f;
  UILayoutMinSize minsize {
    .hasMinSize = hasMinSize,
    .type = hasMinSize ? UILayoutPercent : UILayoutNone,
    .amount = minSizeAmount,
  };
  return minsize; 
}

std::vector<AutoSerialize> uiLayoutAutoserializer {
  AutoSerializeVec4 {
    .structOffset = offsetof(GameObjectUILayout, tint),
    .structOffsetFiller = std::nullopt,
    .field = "tint",
    .defaultValue = glm::vec4(1.f, 1.f, 1.f, 1.f),
  },

  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUILayout, spacing),
    .field = "spacing",
    .defaultValue = 0.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUILayout, minSpacing),
    .field = "min-spacing",
    .defaultValue = 0.f,
  },
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectUILayout, showBackpanel),
    .field = "backpanel",
    .onString = "true",
    .offString = "false",
    .defaultValue = false,
  },
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectUILayout, type),
    .enums = { LAYOUT_HORIZONTAL, LAYOUT_VERTICAL },
    .enumStrings = { "horizontal", "vertical" },
    .field = "type",
    .defaultValue = LAYOUT_HORIZONTAL,
  },
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectUILayout, contentSpacing),
    .enums = { LayoutContentSpacing_Pack, LayoutContentSpacing_SpaceForFirst, LayoutContentSpacing_SpaceForLast },
    .enumStrings = { "pack", "space-for-first", "space-for-last" },
    .field = "content-spacing",
    .defaultValue = LayoutContentSpacing_Pack,
  },
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectUILayout, contentAlign),
    .enums = { LayoutContentAlignment_Negative, LayoutContentAlignment_Neutral, LayoutContentAlignment_Positive },
    .enumStrings = { "neg", "center", "pos" },
    .field = "align-content",
    .defaultValue = LayoutContentAlignment_Neutral,
  },
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectUILayout, alignment.horizontal),
    .enums = { LayoutContentAlignment_Negative, LayoutContentAlignment_Neutral, LayoutContentAlignment_Positive },
    .enumStrings = { "left", "center", "right" },
    .field = "align-items-horizontal",
    .defaultValue = LayoutContentAlignment_Negative,
  },
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectUILayout, alignment.vertical),
    .enums = { LayoutContentAlignment_Negative, LayoutContentAlignment_Neutral, LayoutContentAlignment_Positive },
    .enumStrings = { "down", "center", "up" },
    .field = "align-items-vertical",
    .defaultValue = LayoutContentAlignment_Negative,
  },

  
};

GameObjectUILayout createUILayout(GameobjAttributes& attr, ObjectTypeUtil& util){  
  std::vector<std::string> elements = {};
  if (attr.stringAttributes.find("elements") != attr.stringAttributes.end()){
    elements = split(attr.stringAttributes.at("elements"), ',');
  }
  auto minwidth = layoutMinSizeFromAttr(attr, "minwidth");
  auto minheight = layoutMinSizeFromAttr(attr, "minheight");

  GameObjectUILayout obj{
    .elements = elements,
    .boundInfo = BoundInfo  {
      .xMin = 0, .xMax = 0,
      .yMin = 0, .yMax = 0,
      .zMin = 0, .zMax = 0,
    },
    .panelDisplayOffset = glm::vec3(0.f, 0.f, 0.f),
    .minwidth = minwidth,
    .minheight = minheight,
  };

  attrSet(
    attr, (int*)&obj.anchor.horizontal,
    { UILayoutFlowNone, UILayoutFlowNegative, UILayoutFlowPositive }, { "center", "left", "right"},
    UILayoutFlowNone, "anchor-dir-horizontal", true
  );
  attrSet(
    attr, (int*)&obj.anchor.vertical,
    { UILayoutFlowNone, UILayoutFlowNegative, UILayoutFlowPositive }, { "center", "down", "up"},
    UILayoutFlowNone, "anchor-dir-vertical", true
  );
  attrSet(attr, &obj.anchor.target, "", "anchor");
  attrSet(attr, &obj.anchor.offset, glm::vec3(0.f, 0.f, 0.f), "anchor-offset");

  attrSet(
    attr, (int*)&obj.horizontal,
    { UILayoutFlowNone, UILayoutFlowNegative, UILayoutFlowPositive }, { "center", "left", "right"},
    UILayoutFlowNone, "align-horizontal", true
  );
  attrSet(
    attr, (int*)&obj.vertical,
    { UILayoutFlowNone, UILayoutFlowNegative, UILayoutFlowPositive }, { "center", "down", "up"},
    UILayoutFlowNone, "align-vertical", true
  );

  attrSet(attr, &obj.marginValues.margin, &obj.marginValues.marginSpecified, 0.f, "margin");
  attrSet(attr, &obj.marginValues.marginLeft, &obj.marginValues.marginLeftSpecified, obj.marginValues.margin, "margin-left");
  attrSet(attr, &obj.marginValues.marginRight, &obj.marginValues.marginRightSpecified, obj.marginValues.margin, "margin-right");
  attrSet(attr, &obj.marginValues.marginTop, &obj.marginValues.marginTopSpecified, obj.marginValues.margin, "margin-top");
  attrSet(attr, &obj.marginValues.marginBottom, &obj.marginValues.marginBottomSpecified, obj.marginValues.margin, "margin-bottom");

  attrSet(attr, &obj.border.borderSize, &obj.border.hasBorder, 0.f, "border-size");
  attrSet(attr, &obj.border.borderColor, glm::vec4(1.f, 1.f, 1.f, 1.f), "border-color");

  setTextureInfo(attr, util.ensureTextureLoaded, obj.texture);

  createAutoSerialize((char*)&obj, uiLayoutAutoserializer, attr, util);
  assert(obj.border.borderSize <= 1.f);

  return obj;
}

glm::mat4 layoutBackpanelModelTransform(GameObjectUILayout& layoutObj, glm::vec3 minusScale, glm::vec3 layoutPos){
  auto boundWidth = layoutObj.boundInfo.xMax - layoutObj.boundInfo.xMin;
  auto boundheight = layoutObj.boundInfo.yMax - layoutObj.boundInfo.yMin;
  auto zFightingBias = glm::vec3(0.f, 0.f, -0.001f);  
  return glm::scale(glm::translate(glm::mat4(1.0f), layoutObj.panelDisplayOffset + layoutPos + zFightingBias), glm::vec3(boundWidth, boundheight, 1.f) - minusScale);
}

void setUILayoutAttributes(GameObjectUILayout& layoutObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  maybeSetVec4FromAttr(&layoutObj.tint, "tint", attributes);
  setTextureAttributes(layoutObj.texture, attributes, util);
}