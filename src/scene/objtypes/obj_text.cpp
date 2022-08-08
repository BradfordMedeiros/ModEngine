#include "./obj_text.h"

AlignType alignTypeFromAttr(GameobjAttributes& attr){
  auto hasAlign = attr.stringAttributes.find("align") != attr.stringAttributes.end();
  auto align = CENTER_ALIGN;
  if (hasAlign){
    auto alignValue = attr.stringAttributes.at("align");
    if (alignValue == "left"){
      align = NEGATIVE_ALIGN;
    }else if (alignValue == "center"){
      align = CENTER_ALIGN;
    }else if (alignValue == "right"){
      align = POSITIVE_ALIGN;
    }else{
      std::cout << "text: invalid align value: " << alignValue << std::endl;
      assert(false);
    }
  }
  return align;  
}

std::string alignTypeToStr(AlignType type){
  if (type == NEGATIVE_ALIGN){
    return "left";
  }
  if (type == CENTER_ALIGN){
    return "center";
  }
  if (type == POSITIVE_ALIGN){
    return "right";
  }
  assert(false);
  return "";
}

TextWrap wrapTypeFromAttr(GameobjAttributes& attr){
  auto hasWrapType = attr.stringAttributes.find("wraptype") != attr.stringAttributes.end();
  auto wrapType = WRAP_NONE;
  int wrapamount = -1;
  if (hasWrapType){
    auto wrapContent = attr.stringAttributes.at("wraptype");
    if (wrapContent == "char"){
      wrapType = WRAP_CHARACTERS;
    }else if (wrapContent == "none"){
      // do nothing
    }else{
      std::cout << "invalid wrap type: " << wrapContent << std::endl;
      assert(false);
    }
  }

  auto hasWrapAmount = attr.numAttributes.find("wrapamount") != attr.numAttributes.end();
  if (hasWrapAmount){
    wrapamount = attr.numAttributes.at("wrapamount");
  }
 
  return TextWrap {
    .type = wrapType,
    .wrapamount = wrapamount,
  };
}
std::string wrapTypeToStr(TextWrap wrap){
  if (wrap.type  == WRAP_NONE){
    return "none";
  }
  if (wrap.type == WRAP_CHARACTERS){
    return "char";
  }
  assert(false);
  return "";
}

void restrictWidth(GameObjectUIText& text){
  std::cout << "value: " << text.value << " - " << text.value.size() << std::endl;
  if (text.value.size() > text.charlimit){
    text.value = text.value.substr(0, text.charlimit);
  }
}

GameObjectUIText createUIText(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto align = alignTypeFromAttr(attr);
  auto wrap = wrapTypeFromAttr(attr);
  auto cursorIndexLeftStr = attr.stringAttributes.find("cursor-dir") != attr.stringAttributes.end() ? attr.stringAttributes.at("cursor-dir") : "left";
  modassert(cursorIndexLeftStr == "left" || cursorIndexLeftStr == "right", "invalid value for cursorIndexLeftStr");
  auto cursorIndexLeft = cursorIndexLeftStr == "left" ? true : false;

  GameObjectUIText obj {
    .align = align,
    .wrap = wrap,
    .cursor = UiTextCursor {
      .cursorIndexLeft = cursorIndexLeft,
    },
  };
  
  attrSet(attr, &obj.virtualization.maxheight, -1, "maxheight");
  attrSet(attr, &obj.virtualization.offsetx, 0, "offsetx");
  attrSet(attr, &obj.virtualization.offsety, 0, "offsety");
  attrSet(attr, &obj.virtualization.offset, 0, "offset");

  attrSet(attr, &obj.cursor.cursorIndex, -1, "cursor");
  attrSet(attr, &obj.cursor.highlightLength, 0, "cursor-highlight");
  attrSet(attr, &obj.value, "", "value");
  attrSet(attr, &obj.deltaOffset, 2, "spacing");
  attrSet(attr, &obj.tint, glm::vec4(1.f, 1.f, 1.f, 1.f), "tint");
  attrSet(attr, &obj.charlimit, -1, "charlimit");
  attrSet(attr, &obj.fontFamily, "", "font");

  restrictWidth(obj);
  return obj;
}

void textObjAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes){
  attributes.stringAttributes["value"] = textObj.value; 
  attributes.stringAttributes["spacing"] = std::to_string(textObj.deltaOffset);
  attributes.vecAttr.vec4["tint"] = textObj.tint;
  attributes.stringAttributes["align"] = alignTypeToStr(textObj.align);
  attributes.stringAttributes["wraptype"] = wrapTypeToStr(textObj.wrap);
  attributes.numAttributes["wrapamount"] = textObj.wrap.wrapamount;
  attributes.numAttributes["maxheight"] = textObj.virtualization.maxheight;
  attributes.numAttributes["offsetx"] = textObj.virtualization.offsetx;
  attributes.numAttributes["offsety"] = textObj.virtualization.offsety;
  attributes.numAttributes["offset"] = textObj.virtualization.offset;
  attributes.numAttributes["charlimit"] = textObj.charlimit;
  attributes.numAttributes["cursor"] = textObj.cursor.cursorIndex;
  attributes.stringAttributes["cursor-dir"] = textObj.cursor.cursorIndexLeft ? "left" : "right";
  attributes.numAttributes["cursor-highlight"] = textObj.cursor.highlightLength;
  attributes.stringAttributes["font"] = textObj.fontFamily;
}

void setUITextAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  attrSet(attributes, &textObj.value, "value");
  attrSet(attributes, &textObj.deltaOffset, "spacing");
  attrSet(attributes, &textObj.tint, "tint");

  if (attributes.stringAttributes.find("align") != attributes.stringAttributes.end()){
    textObj.align = alignTypeFromAttr(attributes);
  }
  
  auto wrap = wrapTypeFromAttr(attributes);
  if (attributes.stringAttributes.find("wraptype") != attributes.stringAttributes.end()){
    textObj.wrap.type = wrap.type;
  }
  if (attributes.numAttributes.find("wrapamount") != attributes.numAttributes.end()){
    textObj.wrap.wrapamount = wrap.wrapamount;
  }


  attrSet(attributes, &textObj.virtualization.maxheight, "maxheight");
  attrSet(attributes, &textObj.virtualization.offsetx, "offsetx");
  attrSet(attributes, &textObj.virtualization.offsety, "offsety");
  attrSet(attributes, &textObj.virtualization.offset, "offset");
  attrSet(attributes, &textObj.charlimit, "charlimit");
  attrSet(attributes, &textObj.cursor.cursorIndex ,"cursor");


  if (attributes.stringAttributes.find("cursor-dir") != attributes.stringAttributes.end()){
    auto value = attributes.stringAttributes.at("cursor-dir");
    modassert(value == "left" || value == "right", "cursor-dir : invalid dir " + value);
    textObj.cursor.cursorIndexLeft = value == "left";
  }

  attrSet(attributes, &textObj.cursor.highlightLength, "cursor-highlight");
  attrSet(attributes, &textObj.fontFamily, "font");


  restrictWidth(textObj);
}