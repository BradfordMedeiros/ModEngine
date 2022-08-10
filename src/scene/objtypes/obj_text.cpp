#include "./obj_text.h"

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

std::vector<AutoSerialize> textAutoserializer {
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUIText, wrap.wrapamount),
    .field = "wrapamount",
    .defaultValue = -1,
  },
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectUIText, wrap.type),
    .enums = { WRAP_NONE, WRAP_CHARACTERS },
    .enumStrings = { "none", "char" },
    .field = "wraptype",
    .defaultValue = WRAP_NONE,
  },
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectUIText, align),
    .enums = { NEGATIVE_ALIGN, CENTER_ALIGN, POSITIVE_ALIGN },
    .enumStrings = { "left", "center", "right" },
    .field = "align",
    .defaultValue = CENTER_ALIGN,
  },
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectUIText, cursor.cursorIndexLeft),
    .field = "cursor-dir",
    .onString = "left",
    .offString = "right",
    .defaultValue = true,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUIText, virtualization.maxheight),
    .field = "maxheight",
    .defaultValue = -1,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUIText, virtualization.offsetx),
    .field = "offsetx",
    .defaultValue = 0,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUIText, virtualization.offsety),
    .field = "offsety",
    .defaultValue = 0,
  },
  AutoSerializeInt {
    .structOffset = offsetof(GameObjectUIText, virtualization.offset),
    .field = "offset",
    .defaultValue = 0,
  },
  AutoSerializeInt {
    .structOffset = offsetof(GameObjectUIText, cursor.cursorIndex),
    .field = "cursor",
    .defaultValue = -1,
  },
  AutoSerializeInt {
    .structOffset = offsetof(GameObjectUIText, cursor.highlightLength),
    .field = "cursor-highlight",
    .defaultValue = 0,
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObjectUIText, value),
    .field = "value",
    .defaultValue = "",
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUIText, deltaOffset),
    .field = "spacing",
    .defaultValue = 2,
  },
  AutoSerializeVec4 {
    .structOffset = offsetof(GameObjectUIText, tint),
    .structOffsetFiller = std::nullopt,
    .field = "tint",
    .defaultValue = glm::vec4(1.f, 1.f, 1.f, 1.f),
  },
  AutoSerializeInt {
    .structOffset = offsetof(GameObjectUIText, charlimit),
    .field = "charlimit",
    .defaultValue = -1,
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObjectUIText, fontFamily),
    .field = "font",
    .defaultValue = "",
  },
};

GameObjectUIText createUIText(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectUIText obj {};
  createAutoSerialize((char*)&obj, textAutoserializer, attr, util);
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
    attrSet(attributes, (int*)&textObj.align, { NEGATIVE_ALIGN, CENTER_ALIGN, POSITIVE_ALIGN }, { "left", "center", "right" }, CENTER_ALIGN, "align", true);
  }
  
  if (attributes.stringAttributes.find("wraptype") != attributes.stringAttributes.end()){
    attrSet(attributes, (int*)&textObj.wrap.type, { WRAP_NONE, WRAP_CHARACTERS }, { "none", "char" }, WRAP_NONE, "wraptype", true);
  }
  if (attributes.numAttributes.find("wrapamount") != attributes.numAttributes.end()){
    attrSet(attributes, &textObj.wrap.wrapamount, -1, "wrapamount");
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