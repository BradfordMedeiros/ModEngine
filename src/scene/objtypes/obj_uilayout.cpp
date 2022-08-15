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
    .structOffsetFiller = std::nullopt,
    .field = "spacing",
    .defaultValue = 0.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUILayout, minSpacing),
    .structOffsetFiller = std::nullopt,
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
  AutoSerializeVec4 {
    .structOffset = offsetof(GameObjectUILayout, border.borderColor),
    .structOffsetFiller = std::nullopt,
    .field = "border-color",
    .defaultValue = glm::vec4(1.f, 1.f, 1.f, 1.f),
  },
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectUILayout, anchor.horizontal),
    .enums = { UILayoutFlowNone, UILayoutFlowNegative, UILayoutFlowPositive },
    .enumStrings = { "center", "left", "right"},
    .field = "anchor-dir-horizontal",
    .defaultValue = UILayoutFlowNone,
  },
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectUILayout, anchor.vertical),
    .enums = { UILayoutFlowNone, UILayoutFlowNegative, UILayoutFlowPositive },
    .enumStrings = { "center", "down", "up"},
    .field = "anchor-dir-vertical",
    .defaultValue = UILayoutFlowNone,
  },
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectUILayout, horizontal),
    .enums = { UILayoutFlowNone, UILayoutFlowNegative, UILayoutFlowPositive },
    .enumStrings = { "center", "left", "right"},
    .field = "align-horizontal",
    .defaultValue = UILayoutFlowNone,
  },
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectUILayout, vertical),
    .enums = { UILayoutFlowNone, UILayoutFlowNegative, UILayoutFlowPositive },
    .enumStrings = { "center", "down", "up"},
    .field = "align-vertical",
    .defaultValue = UILayoutFlowNone,
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObjectUILayout, anchor.target),
    .field = "anchor",
    .defaultValue = "",
  },
  AutoSerializeVec3 {
    .structOffset = offsetof(GameObjectUILayout, anchor.offset),
    .structOffsetFiller = std::nullopt,
    .field = "anchor-offset",
    .defaultValue = glm::vec3(0.f, 0.f, 0.f),
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUILayout, border.borderSize),
    .structOffsetFiller = offsetof(GameObjectUILayout, border.hasBorder),
    .field = "border-size",
    .defaultValue = 0.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUILayout, marginValues.margin),
    .structOffsetFiller = offsetof(GameObjectUILayout, marginValues.marginSpecified),
    .field = "margin",
    .defaultValue = 0.f,
  },
  AutoSerializeCustom {
    .structOffset = offsetof(GameObjectUILayout, elements),
    .field = "elements",
    .fieldType = ATTRIBUTE_STRING,
    .deserialize = [](void* offset, void* fieldValue) -> void {
      std::vector<std::string>* elements = static_cast<std::vector<std::string>*>(offset);
      if (fieldValue == NULL){
        *elements = {}; 
      }else{
        std::string* attrValue = static_cast<std::string*>(fieldValue);
        *elements = split(*attrValue, ',');
      }
    },
    .setAttributes = [](void* offset, void* fieldValue) -> void {
      std::vector<std::string>* elements = static_cast<std::vector<std::string>*>(offset);
      if (fieldValue != NULL){
        std::string* attrValue = static_cast<std::string*>(fieldValue);
        *elements = split(*attrValue, ',');
      }
    },
    .getAttribute = [](void* offset) -> AttributeValue {
      std::vector<std::string>* elements = static_cast<std::vector<std::string>*>(offset);
      return join(*elements, ',');
    },
  },
};

GameObjectUILayout createUILayout(GameobjAttributes& attr, ObjectTypeUtil& util){  
  auto minwidth = layoutMinSizeFromAttr(attr, "minwidth");
  auto minheight = layoutMinSizeFromAttr(attr, "minheight");

  GameObjectUILayout obj{
    .boundInfo = BoundInfo  {
      .xMin = 0, .xMax = 0,
      .yMin = 0, .yMax = 0,
      .zMin = 0, .zMax = 0,
    },
    .panelDisplayOffset = glm::vec3(0.f, 0.f, 0.f),
    .minwidth = minwidth,
    .minheight = minheight,
  };

  createAutoSerialize((char*)&obj, uiLayoutAutoserializer, attr, util);
  assert(obj.border.borderSize <= 1.f);

  attrSet(attr, &obj.marginValues.marginLeft, &obj.marginValues.marginLeftSpecified, obj.marginValues.margin, "margin-left");
  attrSet(attr, &obj.marginValues.marginRight, &obj.marginValues.marginRightSpecified, obj.marginValues.margin, "margin-right");
  attrSet(attr, &obj.marginValues.marginTop, &obj.marginValues.marginTopSpecified, obj.marginValues.margin, "margin-top");
  attrSet(attr, &obj.marginValues.marginBottom, &obj.marginValues.marginBottomSpecified, obj.marginValues.margin, "margin-bottom");

  setTextureInfo(attr, util.ensureTextureLoaded, obj.texture);

  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeLayout(GameObjectUILayout& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&obj, uiLayoutAutoserializer, pairs);
  return pairs;
}

glm::mat4 layoutBackpanelModelTransform(GameObjectUILayout& layoutObj, glm::vec3 minusScale, glm::vec3 layoutPos){
  auto boundWidth = layoutObj.boundInfo.xMax - layoutObj.boundInfo.xMin;
  auto boundheight = layoutObj.boundInfo.yMax - layoutObj.boundInfo.yMin;
  auto zFightingBias = glm::vec3(0.f, 0.f, -0.001f);  
  return glm::scale(glm::translate(glm::mat4(1.0f), layoutObj.panelDisplayOffset + layoutPos + zFightingBias), glm::vec3(boundWidth, boundheight, 1.f) - minusScale);
}


void getUILayoutAttributes(GameObjectUILayout& layoutObj, GameobjAttributes& _attributes){
  autoserializerGetAttr((char*)&layoutObj, uiLayoutAutoserializer, _attributes);
}

void setUILayoutAttributes(GameObjectUILayout& layoutObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  setTextureAttributes(layoutObj.texture, attributes, util);
  autoserializerSetAttr((char*)&layoutObj, uiLayoutAutoserializer, attributes, util);
}