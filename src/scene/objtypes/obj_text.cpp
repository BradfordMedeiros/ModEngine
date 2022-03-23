#include "./obj_text.h"

GameObjectUIText createUIText(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto value = attr.stringAttributes.find("value") != attr.stringAttributes.end() ? attr.stringAttributes.at("value") : "";
  auto deltaOffset = attr.numAttributes.find("spacing") != attr.numAttributes.end() ? attr.numAttributes.at("spacing") : 2;
  auto tint = attr.vecAttributes.find("tint") == attr.vecAttributes.end() ? glm::vec3(1.f, 1.f, 1.f) : attr.vecAttributes.at("tint");
  GameObjectUIText obj {
    .value = value,
    .deltaOffset = deltaOffset,
    .tint = tint,
  };
  return obj;
}

void textObjAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes){
  attributes.stringAttributes["value"] = textObj.value;
  attributes.stringAttributes["spacing"] = std::to_string(textObj.deltaOffset);
  attributes.vecAttributes["tint"] = textObj.tint;
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
}