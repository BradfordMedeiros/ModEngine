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

float valueFromAttr(GameobjAttributes& attr, const char* key, float defaultValue){
  if (attr.numAttributes.find(key) == attr.numAttributes.end()){
    return defaultValue;
  }
  return attr.numAttributes.at(key);
}
TextVirtualization virtualizationFromAttr(GameobjAttributes& attr){
  float maxwidth = valueFromAttr(attr, "maxwidth", -1);
  float maxheight = valueFromAttr(attr, "maxheight", -1);
  float offsetx = valueFromAttr(attr, "offsetx", 0);
  float offsety = valueFromAttr(attr, "offsety", 0);
  return TextVirtualization{
    .maxwidth = maxwidth,
    .maxheight = maxheight,
    .offsetx = offsetx,
    .offsety = offsety,
  };
}

void restrictWidth(GameObjectUIText& text){
  std::cout << "value: " << text.value << " - " << text.value.size() << std::endl;
  if (text.value.size() > text.maxwidth){
    text.value = text.value.substr(0, text.maxwidth);
  }
}

GameObjectUIText createUIText(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto value = attr.stringAttributes.find("value") != attr.stringAttributes.end() ? attr.stringAttributes.at("value") : "";
  auto deltaOffset = attr.numAttributes.find("spacing") != attr.numAttributes.end() ? attr.numAttributes.at("spacing") : 2;
  auto tint = attr.vecAttributes.find("tint") == attr.vecAttributes.end() ? glm::vec3(1.f, 1.f, 1.f) : attr.vecAttributes.at("tint");
  auto align = alignTypeFromAttr(attr);
  assert(align != POSITIVE_ALIGN);
  auto wrap = wrapTypeFromAttr(attr);
  auto maxwidth = attr.numAttributes.find("charlimit") == attr.numAttributes.end() ? -1 : attr.numAttributes.at("charlimit");
  GameObjectUIText obj {
    .value = value,
    .deltaOffset = deltaOffset,
    .tint = tint,
    .align = align,
    .wrap = wrap,
    .virtualization = virtualizationFromAttr(attr),
    .maxwidth = maxwidth,
  };
  restrictWidth(obj);
  return obj;
}

void textObjAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes){
  attributes.stringAttributes["value"] = textObj.value; 
  attributes.stringAttributes["spacing"] = std::to_string(textObj.deltaOffset);
  attributes.vecAttributes["tint"] = textObj.tint;
  attributes.stringAttributes["align"] = alignTypeToStr(textObj.align);
  attributes.stringAttributes["wraptype"] = wrapTypeToStr(textObj.wrap);
  attributes.stringAttributes["wrapamount"] = std::to_string(textObj.wrap.wrapamount);
  attributes.numAttributes["maxwidth"] = textObj.virtualization.maxwidth;
  attributes.numAttributes["maxheight"] = textObj.virtualization.maxheight;
  attributes.numAttributes["offsetx"] = textObj.virtualization.offsetx;
  attributes.numAttributes["offsety"] = textObj.virtualization.offsety;
  attributes.numAttributes["charlimit"] = textObj.maxwidth;
}

void setUITextAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  if (attributes.stringAttributes.find("value") != attributes.stringAttributes.end()){
    textObj.value = attributes.stringAttributes.at("value");
  }
  if (attributes.numAttributes.find("spacing") != attributes.numAttributes.end()){
    textObj.deltaOffset = attributes.numAttributes.at("spacing");
  }
  if (attributes.vecAttributes.find("tint") != attributes.vecAttributes.end()){
    textObj.tint = attributes.vecAttributes.at("tint");
  }
  if (attributes.stringAttributes.find("align") != attributes.stringAttributes.end()){
    textObj.align = alignTypeFromAttr(attributes);
  }
  
  auto wrap = wrapTypeFromAttr(attributes);
  if (attributes.stringAttributes.find("wraptype") != attributes.stringAttributes.end()){
    textObj.wrap.type = wrap.type;
  }
  if (attributes.stringAttributes.find("wrapamount") != attributes.stringAttributes.end()){
    textObj.wrap.wrapamount = wrap.wrapamount;
  }

  if (attributes.numAttributes.find("maxwidth") != attributes.numAttributes.end()){
    textObj.virtualization.maxwidth = attributes.numAttributes.at("maxwidth");
  }
  if (attributes.numAttributes.find("maxheight") != attributes.numAttributes.end()){
    textObj.virtualization.maxheight = attributes.numAttributes.at("maxheight");
  }
  if (attributes.numAttributes.find("offsetx") != attributes.numAttributes.end()){
    textObj.virtualization.offsetx = attributes.numAttributes.at("offsetx");
  }
  if (attributes.numAttributes.find("offsety") != attributes.numAttributes.end()){
    textObj.virtualization.offsety = attributes.numAttributes.at("offsety");
  }
  if (attributes.numAttributes.find("charlimit") != attributes.numAttributes.end()){
    textObj.maxwidth = attributes.numAttributes.at("charlimit");
  }
  restrictWidth(textObj);
}