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
  float offsetx = 0;
  float offsety = 0;
  return TextVirtualization{
    .maxwidth = maxwidth,
    .maxheight = maxheight,
    .offsetx = 0,
    .offsety = 0,
  };
}


GameObjectUIText createUIText(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto value = attr.stringAttributes.find("value") != attr.stringAttributes.end() ? attr.stringAttributes.at("value") : "";
  auto deltaOffset = attr.numAttributes.find("spacing") != attr.numAttributes.end() ? attr.numAttributes.at("spacing") : 2;
  auto tint = attr.vecAttributes.find("tint") == attr.vecAttributes.end() ? glm::vec3(1.f, 1.f, 1.f) : attr.vecAttributes.at("tint");
  auto align = alignTypeFromAttr(attr);
  assert(align != POSITIVE_ALIGN);
  auto wrap = wrapTypeFromAttr(attr);
  GameObjectUIText obj {
    .value = value,
    .deltaOffset = deltaOffset,
    .tint = tint,
    .align = align,
    .wrap = wrap,
    .virtualization = virtualizationFromAttr(attr),
  };
  return obj;
}

void textObjAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes){
  attributes.stringAttributes["value"] = textObj.value; 
  attributes.stringAttributes["spacing"] = std::to_string(textObj.deltaOffset);
  attributes.vecAttributes["tint"] = textObj.tint;
  attributes.stringAttributes["align"] = alignTypeToStr(textObj.align);
  attributes.stringAttributes["wraptype"] = wrapTypeToStr(textObj.wrap);
  attributes.stringAttributes["wrapamount"] = std::to_string(textObj.wrap.wrapamount);
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
}