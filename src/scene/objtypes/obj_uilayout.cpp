#include "./obj_uilayout.h"

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

  ///
  AutoSerializeCustom {
    .structOffset = offsetof(GameObjectUILayout, elements),
    .field = "elements",
    .fieldType = ATTRIBUTE_STRING,
    .deserialize = [](ObjectTypeUtil& util, void* offset, void* fieldValue) -> void {
      std::vector<std::string>* elements = static_cast<std::vector<std::string>*>(offset);
      if (fieldValue == NULL){
        *elements = {}; 
      }else{
        std::string* attrValue = static_cast<std::string*>(fieldValue);
        *elements = split(*attrValue, ',');
      }
    },
    .setAttributes = [](ObjectSetAttribUtil& util, void* offset, void* fieldValue) -> void {
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
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUILayout, marginValues.margin),
    .structOffsetFiller = offsetof(GameObjectUILayout, marginValues.marginSpecified),
    .field = "margin",
    .defaultValue = 0.f,
  },
  AutoSerializeCustom {
    .structOffset = offsetof(GameObjectUILayout, marginValues),
    .field = "margin-left",
    .fieldType = ATTRIBUTE_FLOAT,
    .deserialize = [](ObjectTypeUtil& util, void* offset, void* fieldValue) -> void {
      LayoutMargin* margin = static_cast<LayoutMargin*>(offset);
      float* floatValue = static_cast<float*>(fieldValue);
      if (floatValue != NULL){
        std::cout << "margin-left value: " << *floatValue << std::endl; 
        margin -> marginLeft = *floatValue;
        margin -> marginLeftSpecified = true;
      }else{
        std::cout << "margin-left value: " << margin -> margin << std::endl; 
        margin -> marginLeft = margin -> margin;
        margin -> marginLeftSpecified = false;
      }
    },
    .setAttributes = [](ObjectSetAttribUtil& util, void* offset, void* fieldValue) -> void {
      LayoutMargin* margin = static_cast<LayoutMargin*>(offset);
      float* floatValue = static_cast<float*>(fieldValue);
      if (floatValue != NULL){
        margin -> marginLeft = *floatValue;
        margin -> marginLeftSpecified = true;
      }
    },
    .getAttribute = [](void* offset) -> AttributeValue {
      LayoutMargin* margin = static_cast<LayoutMargin*>(offset);
      return margin -> marginLeft;
    },
  },
  AutoSerializeCustom {
    .structOffset = offsetof(GameObjectUILayout, marginValues),
    .field = "margin-right",
    .fieldType = ATTRIBUTE_FLOAT,
    .deserialize = [](ObjectTypeUtil& util, void* offset, void* fieldValue) -> void {
      LayoutMargin* margin = static_cast<LayoutMargin*>(offset);
      float* floatValue = static_cast<float*>(fieldValue);
      if (floatValue != NULL){
        std::cout << "margin-right value: " << *floatValue << std::endl; 
        margin -> marginRight = *floatValue;
        margin -> marginRightSpecified = true;
      }else{
        std::cout << "margin-right value: " << margin -> margin << std::endl; 
        margin -> marginRight = margin -> margin;
        margin -> marginRightSpecified = false;
      }
    },
    .setAttributes = [](ObjectSetAttribUtil& util, void* offset, void* fieldValue) -> void {
      LayoutMargin* margin = static_cast<LayoutMargin*>(offset);
      float* floatValue = static_cast<float*>(fieldValue);
      if (floatValue != NULL){
        margin -> marginRight = *floatValue;
        margin -> marginRightSpecified = true;
      }
    },
    .getAttribute = [](void* offset) -> AttributeValue {
      LayoutMargin* margin = static_cast<LayoutMargin*>(offset);
      return margin -> marginRight;
    },
  },
  AutoSerializeCustom {
    .structOffset = offsetof(GameObjectUILayout, marginValues),
    .field = "margin-top",
    .fieldType = ATTRIBUTE_FLOAT,
    .deserialize = [](ObjectTypeUtil& util, void* offset, void* fieldValue) -> void {
      LayoutMargin* margin = static_cast<LayoutMargin*>(offset);
      float* floatValue = static_cast<float*>(fieldValue);
      if (floatValue != NULL){
        std::cout << "margin-top value: " << *floatValue << std::endl; 
        margin -> marginTop = *floatValue;
        margin -> marginTopSpecified = true;
      }else{
        std::cout << "margin-top value: " << margin -> margin << std::endl; 
        margin -> marginTop = margin -> margin;
        margin -> marginTopSpecified = false;
      }
    },
    .setAttributes = [](ObjectSetAttribUtil& util, void* offset, void* fieldValue) -> void {
      LayoutMargin* margin = static_cast<LayoutMargin*>(offset);
      float* floatValue = static_cast<float*>(fieldValue);
      if (floatValue != NULL){
        margin -> marginTop = *floatValue;
        margin -> marginTopSpecified = true;
      }
    },
    .getAttribute = [](void* offset) -> AttributeValue {
      LayoutMargin* margin = static_cast<LayoutMargin*>(offset);
      return margin -> marginTop;
    },
  },
  AutoSerializeCustom {
    .structOffset = offsetof(GameObjectUILayout, marginValues),
    .field = "margin-bottom",
    .fieldType = ATTRIBUTE_FLOAT,
    .deserialize = [](ObjectTypeUtil& util, void* offset, void* fieldValue) -> void {
      LayoutMargin* margin = static_cast<LayoutMargin*>(offset);
      float* floatValue = static_cast<float*>(fieldValue);
      if (floatValue != NULL){
        std::cout << "margin-bottom value: " << *floatValue << std::endl; 
        margin -> marginBottom = *floatValue;
        margin -> marginBottomSpecified = true;
      }else{
        std::cout << "margin-bottom value: " << margin -> margin << std::endl; 
        margin -> marginBottom = margin -> margin;
        margin -> marginBottomSpecified = false;
      }
    },
    .setAttributes = [](ObjectSetAttribUtil& util, void* offset, void* fieldValue) -> void {
      LayoutMargin* margin = static_cast<LayoutMargin*>(offset);
      float* floatValue = static_cast<float*>(fieldValue);
      if (floatValue != NULL){
        margin -> marginBottom = *floatValue;
        margin -> marginBottomSpecified = true;
      }
    },
    .getAttribute = [](void* offset) -> AttributeValue {
      LayoutMargin* margin = static_cast<LayoutMargin*>(offset);
      return margin -> marginBottom;
    },
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUILayout, minwidth),
    .structOffsetFiller = std::nullopt,
    .field = "minwidth",
    .defaultValue = -1.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUILayout, minheight),
    .structOffsetFiller = std::nullopt,
    .field = "minheight",
    .defaultValue = -1.f,
  },
};

static auto _ = addTextureAutoserializer<GameObjectUILayout>(uiLayoutAutoserializer);

GameObjectUILayout createUILayout(GameobjAttributes& attr, ObjectTypeUtil& util){  
  GameObjectUILayout obj{
    .boundInfo = BoundInfo  {
      .xMin = 0, .xMax = 0,
      .yMin = 0, .yMax = 0,
      .zMin = 0, .zMax = 0,
    },
    .panelDisplayOffset = glm::vec3(0.f, 0.f, 0.f),
  };
  createAutoSerializeWithTextureLoading((char*)&obj, uiLayoutAutoserializer, attr, util);
  assert(obj.border.borderSize <= 1.f);
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
  autoserializerSetAttrWithTextureLoading((char*)&layoutObj, uiLayoutAutoserializer, attributes, util);
}