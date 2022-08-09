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
  

//
  attrSet(
    attr, 
    (int*)&alignType, 
    { LayoutContentAlignment_Negative, LayoutContentAlignment_Neutral, LayoutContentAlignment_Positive }, 
    { neg, "center", pos }, 
    defaultType, 
    attrname, 
    true
  );

//

  return alignType;
}

GameObjectUILayout createUILayout(GameobjAttributes& attr, ObjectTypeUtil& util){  
  std::vector<std::string> elements = {};
  if (attr.stringAttributes.find("elements") != attr.stringAttributes.end()){
    elements = split(attr.stringAttributes.at("elements"), ',');
  }
  
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

  BoundInfo boundInfo {
    .xMin = 0, .xMax = 0,
    .yMin = 0, .yMax = 0,
    .zMin = 0, .zMax = 0,
  };

  GameObjectUILayout obj{
    .elements = elements,
    .boundInfo = boundInfo,
    .panelDisplayOffset = glm::vec3(0.f, 0.f, 0.f),
    .marginValues = marginValues,
    .anchor = anchor,
    .minwidth = minwidth,
    .minheight = minheight,
    .horizontal = horizontal,
    .vertical = vertical,
    .border = border,
    .alignment = alignment,
    .contentAlign = contentAlign,
  };

  attrSet(
    attr, (int*)&obj.contentSpacing, 
    { LayoutContentSpacing_Pack, LayoutContentSpacing_SpaceForFirst, LayoutContentSpacing_SpaceForLast },  { "pack", "space-for-first", "space-for-last" }, 
    LayoutContentSpacing_Pack, "content-spacing", true
  );
  attrSet(attr, (int*)&obj.type, { LAYOUT_HORIZONTAL, LAYOUT_VERTICAL }, { "horizontal", "vertical" }, LAYOUT_HORIZONTAL, "type", false);
  setTextureInfo(attr, util.ensureTextureLoaded, obj.texture);
  attrSet(attr, &obj.showBackpanel, "true", "false", false, "backpanel", false);
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