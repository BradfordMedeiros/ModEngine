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


LayoutContentAlignmentType layoutContentAlignment(GameobjAttributes& attr, const char* attrname, const char* neg, const char* pos, LayoutContentAlignmentType defaultType){
  bool hasAlign = attr.stringAttributes.find(attrname) != attr.stringAttributes.end();
  auto alignType = defaultType;
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

LayoutContentSpacing layoutContentSpacing(GameobjAttributes& attr, const char* attrname){
  if (attr.stringAttributes.find(attrname) != attr.stringAttributes.end()){
    auto value = attr.stringAttributes.at(attrname);
    if (value == "pack"){
      return LayoutContentSpacing_Pack;
    }else if (value == "space-for-first"){
      return LayoutContentSpacing_SpaceForFirst;
    }else if (value == "space-for-last"){
      return LayoutContentSpacing_SpaceForLast;
    }
    modassert(false, "uilayout - content spacing - invalid payload: " + value);
    return LayoutContentSpacing_Pack;
  }
  return LayoutContentSpacing_Pack;
}

GameObjectUILayout createUILayout(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto type = attr.stringAttributes.find("type") != attr.stringAttributes.end() && (attr.stringAttributes.at("type") == "vertical") ? LAYOUT_VERTICAL : LAYOUT_HORIZONTAL;
  
  std::vector<std::string> elements = {};
  if (attr.stringAttributes.find("elements") != attr.stringAttributes.end()){
    elements = split(attr.stringAttributes.at("elements"), ',');
  }
  auto showBackpanel = (attr.stringAttributes.find("backpanel") != attr.stringAttributes.end() && attr.stringAttributes.at("backpanel") == "true");
  
  LayoutMargin marginValues {};
  attrSet(attr, &marginValues.margin, &marginValues.marginSpecified, 0.f, "margin");
  attrSet(attr, &marginValues.marginLeft, &marginValues.marginLeftSpecified, marginValues.margin, "margin-left");
  attrSet(attr, &marginValues.marginRight, &marginValues.marginRightSpecified, marginValues.margin, "margin-right");
  attrSet(attr, &marginValues.marginTop, &marginValues.marginTopSpecified, marginValues.margin, "margin-top");
  attrSet(attr, &marginValues.marginBottom, &marginValues.marginBottomSpecified, marginValues.margin, "margin-bottom");

  auto minwidth = layoutMinSizeFromAttr(attr, "minwidth");
  auto minheight = layoutMinSizeFromAttr(attr, "minheight");
  auto horizontal = layoutAlignFromAttr(attr, "align-horizontal", "left", "right");
  auto vertical = layoutAlignFromAttr(attr, "align-vertical", "down", "up");

  auto anchorHorizontal = layoutAlignFromAttr(attr, "anchor-dir-horizontal", "left", "right");
  auto anchorVertical = layoutAlignFromAttr(attr, "anchor-dir-vertical", "down", "up");
  LayoutAnchor anchor = {
    .horizontal = anchorHorizontal,
    .vertical = anchorVertical,
  };
  attrSet(attr, &anchor.target, "", "anchor");
  attrSet(attr, &anchor.offset, glm::vec3(0.f, 0.f, 0.f), "anchor-offset");

  LayoutBorder border {};
  attrSet(attr, &border.borderSize, &border.hasBorder, 0.f, "border-size");
  attrSet(attr, &border.borderColor, glm::vec4(1.f, 1.f, 1.f, 1.f), "border-color");
  assert(border.borderSize <= 1.f);

  LayoutContentAlignment alignment {
    .vertical = layoutContentAlignment(attr, "align-items-vertical", "down", "up", LayoutContentAlignment_Negative),
    .horizontal = layoutContentAlignment(attr, "align-items-horizontal", "left", "right", LayoutContentAlignment_Negative),
  };

  auto contentAlign = layoutContentAlignment(attr, "align-content", "pos", "neg", LayoutContentAlignment_Neutral);
  auto contentSpacing = layoutContentSpacing(attr, "content-spacing");

  BoundInfo boundInfo {
    .xMin = 0, .xMax = 0,
    .yMin = 0, .yMax = 0,
    .zMin = 0, .zMax = 0,
  };

  GameObjectUILayout obj{
    .type = type,
    .elements = elements,
    .boundInfo = boundInfo,
    .panelDisplayOffset = glm::vec3(0.f, 0.f, 0.f),
    .showBackpanel = showBackpanel,
    .marginValues = marginValues,
    .anchor = anchor,
    .texture = texinfoFromFields(attr, util.ensureTextureLoaded),
    .minwidth = minwidth,
    .minheight = minheight,
    .horizontal = horizontal,
    .vertical = vertical,
    .border = border,
    .alignment = alignment,
    .contentAlign = contentAlign,
    .contentSpacing = contentSpacing,
  };

  attrSet(attr, &obj.spacing, 0.f, "spacing");
  attrSet(attr, &obj.minSpacing, 0.f, "min-spacing");
  attrSet(attr, &obj.tint, glm::vec4(1.f, 1.f, 1.f, 1.f), "tint");
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