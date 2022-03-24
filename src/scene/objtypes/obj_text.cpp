#include "./obj_text.h"

AlignType alignTypeFromAttr(GameobjAttributes& attr){
  auto hasAlign = attr.stringAttributes.find("align") != attr.stringAttributes.end();
  auto align = CENTER_ALIGN;
  if (hasAlign){
    auto alignValue = attr.stringAttributes.at("align");
    if (alignValue == "left"){
      alignValue = NEGATIVE_ALIGN;
    }else if (alignValue == "center"){
      alignValue = CENTER_ALIGN;
    }else if (alignValue == "right"){
      alignValue = POSITIVE_ALIGN;
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

GameObjectUIText createUIText(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto value = attr.stringAttributes.find("value") != attr.stringAttributes.end() ? attr.stringAttributes.at("value") : "";
  auto deltaOffset = attr.numAttributes.find("spacing") != attr.numAttributes.end() ? attr.numAttributes.at("spacing") : 2;
  auto tint = attr.vecAttributes.find("tint") == attr.vecAttributes.end() ? glm::vec3(1.f, 1.f, 1.f) : attr.vecAttributes.at("tint");
  auto align = alignTypeFromAttr(attr);
  GameObjectUIText obj {
    .value = value,
    .deltaOffset = deltaOffset,
    .tint = tint,
    .align = align,
  };
  return obj;
}

void textObjAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes){
  attributes.stringAttributes["value"] = textObj.value;
  attributes.stringAttributes["spacing"] = std::to_string(textObj.deltaOffset);
  attributes.vecAttributes["tint"] = textObj.tint;
  attributes.stringAttributes["align"] = alignTypeToStr(textObj.align);
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
}