#include "./obj_uibutton.h"

std::vector<AutoSerialize> uiButtonAutoserializer {
  AutoSerializeVec4 {
    .structOffset = offsetof(GameObjectUIButton, tint),
    .structOffsetFiller = std::nullopt,
    .field = "tint",
    .defaultValue = glm::vec4(1.f, 1.f, 1.f, 1.f),
  },
  AutoSerializeVec4 {
    .structOffset = offsetof(GameObjectUIButton, onTint),
    .structOffsetFiller = offsetof(GameObjectUIButton, hasOnTint),
    .field = "ontint",
    .defaultValue = glm::vec4(1.f, 1.f, 1.f, 1.f),
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObjectUIButton, onToggleOn),
    .field = "on",
    .defaultValue = "",
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObjectUIButton, onToggleOff),
    .field = "off",
    .defaultValue = "",
  },
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectUIButton, toggleOn),
    .field = "state",
    .onString = "on",
    .offString = "off",
    .defaultValue = false,
  },
  AutoSerializeBool { 
    .structOffset = offsetof(GameObjectUIButton, initialState),
    .field = "state",
    .onString = "on",
    .offString = "off",
    .defaultValue = false,
  },
  AutoSerializeBool { 
    .structOffset = offsetof(GameObjectUIButton, canToggle),
    .field = "cantoggle",
    .onString = "true",
    .offString = "false",
    .defaultValue = true,
  },
  AutoSerializeTextureLoader {
    .structOffset = offsetof(GameObjectUIButton, onTexture),
    .structOffsetName = offsetof(GameObjectUIButton, onTextureString),
    .field = "ontexture",
    .defaultValue = "./res/models/controls/on.png",
  },
  AutoSerializeTextureLoader {
    .structOffset = offsetof(GameObjectUIButton, offTexture),
    .structOffsetName = offsetof(GameObjectUIButton, offTextureString),
    .field = "offtexture",
    .defaultValue = "./res/models/controls/off.png",
  },
};

GameObjectUIButton createUIButton(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectUIButton obj {};
  attrSetCommon(attr, obj.common, util.meshes);
  createAutoSerialize((char*)&obj, uiButtonAutoserializer, attr, util);
  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeButton(GameObjectUIButton& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  addSerializeCommon(pairs, obj.common);
  if (obj.canToggle != true){
    pairs.push_back(std::pair<std::string, std::string>("cantoggle", "false"));
  }
  if (obj.onTextureString != ""){
    pairs.push_back(std::pair<std::string, std::string>("ontexture", obj.onTextureString));
  }
  if (obj.offTextureString != ""){
    pairs.push_back(std::pair<std::string, std::string>("offtexture", obj.offTextureString));
  }
  if (obj.onToggleOn != ""){
    pairs.push_back(std::pair<std::string, std::string>("on", obj.onToggleOn));
  }
  if (obj.onToggleOff != ""){
    pairs.push_back(std::pair<std::string, std::string>("off", obj.onToggleOff));
  }
  if (obj.initialState == true){
    pairs.push_back(std::pair<std::string, std::string>("state", "on"));
  }
  if (obj.hasOnTint && !isIdentityVec(obj.onTint)){
    pairs.push_back(std::pair<std::string, std::string>("ontint", serializeVec(obj.onTint)));
  }
  if (!isIdentityVec(obj.tint)){
    pairs.push_back(std::pair<std::string, std::string>("tint", serializeVec(obj.tint)));
  }
  return pairs;
}

void getUIButtonAttributes(GameObjectUIButton& uiButton, GameobjAttributes& _attributes){
  autoserializerGetAttr((char*)&uiButton, uiButtonAutoserializer, _attributes);
}


void setUIButtonAttributes(GameObjectUIButton& buttonObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  bool setState = maybeSetBoolFromStrAttr(&buttonObj.toggleOn, "state", "on", "off", attributes);
  if (setState){
    MODTODO("set button state should call toggle on / off");
  }
  maybeSetVec4FromAttr(&buttonObj.tint, "tint", attributes);
}