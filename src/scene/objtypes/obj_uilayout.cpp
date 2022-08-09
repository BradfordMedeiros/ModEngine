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


GameObjectUILayout createUILayout(GameobjAttributes& attr, ObjectTypeUtil& util){  
  std::vector<std::string> elements = {};
  if (attr.stringAttributes.find("elements") != attr.stringAttributes.end()){
    elements = split(attr.stringAttributes.at("elements"), ',');
  }
  
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

  BoundInfo boundInfo {
    .xMin = 0, .xMax = 0,
    .yMin = 0, .yMax = 0,
    .zMin = 0, .zMax = 0,
  };

  GameObjectUILayout obj{
    .elements = elements,
    .boundInfo = boundInfo,
    .panelDisplayOffset = glm::vec3(0.f, 0.f, 0.f),
    .anchor = anchor,
    .minwidth = minwidth,
    .minheight = minheight,
    .horizontal = horizontal,
    .vertical = vertical,
  };
  attrSet(attr, &obj.marginValues.margin, &obj.marginValues.marginSpecified, 0.f, "margin");
  attrSet(attr, &obj.marginValues.marginLeft, &obj.marginValues.marginLeftSpecified, obj.marginValues.margin, "margin-left");
  attrSet(attr, &obj.marginValues.marginRight, &obj.marginValues.marginRightSpecified, obj.marginValues.margin, "margin-right");
  attrSet(attr, &obj.marginValues.marginTop, &obj.marginValues.marginTopSpecified, obj.marginValues.margin, "margin-top");
  attrSet(attr, &obj.marginValues.marginBottom, &obj.marginValues.marginBottomSpecified, obj.marginValues.margin, "margin-bottom");

  attrSet(attr, &obj.border.borderSize, &obj.border.hasBorder, 0.f, "border-size");
  attrSet(attr, &obj.border.borderColor, glm::vec4(1.f, 1.f, 1.f, 1.f), "border-color");
  assert(obj.border.borderSize <= 1.f);
  
  attrSet(
    attr, (int*)&obj.alignment.vertical, 
    { LayoutContentAlignment_Negative, LayoutContentAlignment_Neutral, LayoutContentAlignment_Positive }, { "down", "center", "up" }, 
    LayoutContentAlignment_Negative, "align-items-vertical", true
  );
  attrSet(
    attr, (int*)&obj.alignment.horizontal, 
    { LayoutContentAlignment_Negative, LayoutContentAlignment_Neutral, LayoutContentAlignment_Positive }, { "left", "center", "right" }, 
    LayoutContentAlignment_Negative, "align-items-horizontal", true
  );
  attrSet(
    attr, (int*)&obj.contentAlign, 
    { LayoutContentAlignment_Negative, LayoutContentAlignment_Neutral, LayoutContentAlignment_Positive }, { "neg", "center", "pos" }, 
    LayoutContentAlignment_Neutral, "align-content", true
  );

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