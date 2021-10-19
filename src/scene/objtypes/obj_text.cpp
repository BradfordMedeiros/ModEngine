#include "./obj_text.h"

GameObjectUIText createUIText(GameobjAttributes& attr){
  auto value = attr.stringAttributes.find("value") != attr.stringAttributes.end() ? attr.stringAttributes.at("value") : "";
  auto deltaOffset = attr.numAttributes.find("spacing") != attr.numAttributes.end() ? attr.numAttributes.at("spacing") : 2;
  GameObjectUIText obj {
    .value = value,
    .deltaOffset = deltaOffset,
  };
  return obj;
}

void textObjAttributes(GameObjectUIText& textObj, GameobjAttributes& attributes){
  attributes.stringAttributes["value"] = textObj.value;
  attributes.stringAttributes["spacing"] = std::to_string(textObj.deltaOffset);
}