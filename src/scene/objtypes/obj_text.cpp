#include "./obj_text.h"

void restrictWidth(GameObjectUIText& text){
  std::cout << "value: " << text.value << " - " << text.value.size() << std::endl;
  if (text.value.size() > text.charlimit){
    text.value = text.value.substr(0, text.charlimit);
  }
}

std::vector<AutoSerialize> textAutoserializer {
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUIText, wrap.wrapamount),
    .structOffsetFiller = std::nullopt,
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
    .structOffsetFiller = std::nullopt,
    .field = "maxheight",
    .defaultValue = -1,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUIText, virtualization.offsetx),
    .structOffsetFiller = std::nullopt,
    .field = "offsetx",
    .defaultValue = 0,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectUIText, virtualization.offsety),
    .structOffsetFiller = std::nullopt,
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
  AutoSerializeForceString {
    .structOffset = offsetof(GameObjectUIText, value),
    .field = "value",
    .defaultValue = "",
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
  createAutoSerializeWithTextureLoading((char*)&obj, textAutoserializer, attr, util);
  restrictWidth(obj);
  return obj;
}

void textObjAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes){
  autoserializerGetAttr((char*)&textObj, textAutoserializer, attributes);
}

std::vector<std::pair<std::string, std::string>> serializeText(GameObjectUIText& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&obj, textAutoserializer, pairs);
  return pairs;
}

std::optional<AttributeValuePtr> getTextAttribute(GameObjectUIText& obj, const char* field){
  return getAttributePtr((char*)&obj, textAutoserializer, field);
}

bool setTextAttribute(GameObjectUIText& textObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags& options){
  auto setAttr = autoserializerSetAttrWithTextureLoading((char*)&textObj, textAutoserializer, field, value, util);
  restrictWidth(textObj);
  options.rebuildPhysics = true;
  return setAttr;
}
