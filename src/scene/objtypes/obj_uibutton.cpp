#include "./obj_uibutton.h"

GameObjectUIButton createUIButton(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto onTexture = attr.stringAttributes.find("ontexture") != attr.stringAttributes.end() ? attr.stringAttributes.at("ontexture") : "";
  auto offTexture = attr.stringAttributes.find("offtexture") != attr.stringAttributes.end() ? attr.stringAttributes.at("offtexture") : "";

  GameObjectUIButton obj { 
    .common = parseCommon(attr, util.meshes),
    .onTextureString = onTexture,
    .onTexture = util.ensureTextureLoaded(onTexture == "" ? "./res/models/controls/on.png" : onTexture).textureId,
    .offTextureString = offTexture,
    .offTexture = util.ensureTextureLoaded(offTexture == "" ? "./res/models/controls/off.png" : offTexture).textureId,
  };

  attrSet(attr, &obj.canToggle, "true", "false", true, "cantoggle", false);
  attrSet(attr, &obj.initialState, "on", "off", false, "state", false);
  attrSet(attr, &obj.toggleOn, "on", "off", false, "state", false);
  attrSet(attr, &obj.onToggleOn, "", "on");
  attrSet(attr, &obj.onToggleOff, "", "off");
  attrSet(attr, &obj.onTint, &obj.hasOnTint, glm::vec4(1.f, 1.f, 1.f, 1.f), "ontint");
  attrSet(attr, &obj.tint, glm::vec4(1.f, 1.f, 1.f, 1.f), "tint");

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

void getUIButtonAttributes(GameObjectUIButton& textObj, GameobjAttributes& _attributes){
  MODTODO("ui button - get rest of attributes");
  _attributes.vecAttr.vec4["tint"] = textObj.tint;
}


void setUIButtonAttributes(GameObjectUIButton& buttonObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  bool setState = maybeSetBoolFromStrAttr(&buttonObj.toggleOn, "state", "on", "off", attributes);
  if (setState){
    MODTODO("set button state should call toggle on / off");
  }
  maybeSetVec4FromAttr(&buttonObj.tint, "tint", attributes);
}