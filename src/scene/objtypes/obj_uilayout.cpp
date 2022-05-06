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

UILayoutFlowType layoutAlignFromAttr(GameobjAttributes& attr, const char* attrname, const char* neg, const char* pos){
  bool hasAlign = attr.stringAttributes.find(attrname) != attr.stringAttributes.end();
  auto alignType = UILayoutFlowNone;
  if (hasAlign){
    if (attr.stringAttributes.at(attrname) == neg){
      alignType = UILayoutFlowNegative;
    }else if (attr.stringAttributes.at(attrname) == pos){
      alignType = UILayoutFlowPositive;
    }else if (attr.stringAttributes.at(attrname) == "center"){
      alignType = UILayoutFlowNone;
    }else{
      std::cout << "invalid align type" << std::endl;
      assert(false);
    }
  }
  return alignType;
}

float getMargin(GameobjAttributes& attr, const char* attrname, float defaultMargin, bool* valueSpecified){
  bool marginTypeSpecified = attr.numAttributes.find(attrname) != attr.numAttributes.end();
  *valueSpecified = marginTypeSpecified;
  if (!marginTypeSpecified){
    return defaultMargin;
  }
  return attr.numAttributes.at(attrname);
}

LayoutContentAlignmentType layoutContentAlignment(GameobjAttributes& attr, const char* attrname, const char* neg, const char* pos){
  bool hasAlign = attr.stringAttributes.find(attrname) != attr.stringAttributes.end();
  auto alignType = LayoutContentAlignment_Negative;
  if (hasAlign){
    if (attr.stringAttributes.at(attrname) == neg){
      alignType = LayoutContentAlignment_Negative;
    }else if (attr.stringAttributes.at(attrname) == pos){
      alignType = LayoutContentAlignment_Positive;
    }else if (attr.stringAttributes.at(attrname) == "center"){
      alignType = LayoutContentAlignment_Neutral;
    }else{
      std::cout << "invalid align items type: " << attr.stringAttributes.at(attrname) << std::endl;
      assert(false);
    }
  }
  return alignType;
}

GameObjectUILayout createUILayout(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto spacing = attr.numAttributes.find("spacing") == attr.numAttributes.end() ? 0.f : attr.numAttributes.at("spacing");
  auto type = attr.stringAttributes.find("type") != attr.stringAttributes.end() && (attr.stringAttributes.at("type") == "vertical") ? LAYOUT_VERTICAL : LAYOUT_HORIZONTAL;
  
  std::vector<std::string> elements = {};
  if (attr.stringAttributes.find("elements") != attr.stringAttributes.end()){
    elements = split(attr.stringAttributes.at("elements"), ',');
  }
  auto showBackpanel = (attr.stringAttributes.find("backpanel") != attr.stringAttributes.end() && attr.stringAttributes.at("backpanel") == "true");
  auto tint = attr.vecAttributes.find("tint") == attr.vecAttributes.end() ? glm::vec3(1.f, 1.f, 1.f) : attr.vecAttributes.at("tint");
  
  bool marginSpecified = attr.numAttributes.find("margin") != attr.numAttributes.end();
  auto margin = !marginSpecified ? 0.f : attr.numAttributes.at("margin");

  bool marginLeftSpecified = false;
  float marginLeft = getMargin(attr, "margin-left", margin, &marginLeftSpecified);
  
  bool marginRightSpecified = false;
  float marginRight = getMargin(attr, "margin-right", margin, &marginRightSpecified);

  bool marginTopSpecified = false;
  float marginTop = getMargin(attr, "margin-top", margin, &marginTopSpecified);

  bool marginBottomSpecified = false;
  float marginBottom = getMargin(attr, "margin-bottom", margin, &marginBottomSpecified);

  LayoutMargin marginValues {
    .margin = margin,
    .marginLeft = marginLeft,
    .marginRight = marginRight,
    .marginBottom = marginBottom,
    .marginTop = marginTop,
    .marginSpecified = marginSpecified,
    .marginLeftSpecified = marginLeftSpecified,
    .marginRightSpecified = marginRightSpecified,
    .marginBottomSpecified = marginBottomSpecified,
    .marginTopSpecified = marginTopSpecified,
  };

  auto minwidth = layoutMinSizeFromAttr(attr, "minwidth");
  auto minheight = layoutMinSizeFromAttr(attr, "minheight");
  auto horizontal = layoutAlignFromAttr(attr, "align-horizontal", "left", "right");
  auto vertical = layoutAlignFromAttr(attr, "align-vertical", "down", "up");

  auto anchorTarget = attr.stringAttributes.find("anchor") == attr.stringAttributes.end() ? "" : attr.stringAttributes.at("anchor");
  auto anchorOffset = attr.vecAttributes.find("anchor-offset") == attr.vecAttributes.end() ? glm::vec3(0.f, 0.f, 0.f) : attr.vecAttributes.at("anchor-offset");
  auto anchorHorizontal = layoutAlignFromAttr(attr, "anchor-dir-horizontal", "left", "right");
  auto anchorVertical = layoutAlignFromAttr(attr, "anchor-dir-vertical", "down", "up");
  LayoutAnchor anchor = {
    .target = anchorTarget,
    .offset = anchorOffset,
    .horizontal = anchorHorizontal,
    .vertical = anchorVertical,
  };

  bool hasBorder = attr.numAttributes.find("border-size") != attr.numAttributes.end();
  auto borderSize = hasBorder ? attr.numAttributes.at("border-size") : 0.f;
  assert(borderSize <= 1.f);
  auto borderColor = attr.vecAttributes.find("border-color") == attr.vecAttributes.end() ? glm::vec3(1.f, 1.f, 1.f) : attr.vecAttributes.at("border-color");
  LayoutBorder border {
    .borderSize = borderSize,
    .borderColor = borderColor,
    .hasBorder = hasBorder,
  };

  LayoutContentAlignment alignment {
    .vertical = layoutContentAlignment(attr, "align-items-vertical", "down", "up"),
    .horizontal = layoutContentAlignment(attr, "align-items-horizontal", "left", "right"),
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
    .marginValues = marginValues,
    .anchor = anchor,
    .texture = texinfoFromFields(attr, util.ensureTextureLoaded),
    .minwidth = minwidth,
    .minheight = minheight,
    .horizontal = horizontal,
    .vertical = vertical,
    .border = border,
    .alignment = alignment,
  };
  return obj;
}

glm::mat4 layoutBackpanelModelTransform(GameObjectUILayout& layoutObj, glm::vec3 minusScale){
  auto boundWidth = layoutObj.boundInfo.xMax - layoutObj.boundInfo.xMin;
  auto boundheight = layoutObj.boundInfo.yMax - layoutObj.boundInfo.yMin;
  auto zFightingBias = glm::vec3(0.f, 0.f, -0.001f);  
  return glm::scale(glm::translate(glm::mat4(1.0f), layoutObj.boundOrigin + zFightingBias), glm::vec3(boundWidth, boundheight, 1.f) - minusScale);
}