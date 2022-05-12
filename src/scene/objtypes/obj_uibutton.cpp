#include "./obj_uibutton.h"

GameObjectUIButton createUIButton(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto onTexture = attr.stringAttributes.find("ontexture") != attr.stringAttributes.end() ? attr.stringAttributes.at("ontexture") : "";
  auto offTexture = attr.stringAttributes.find("offtexture") != attr.stringAttributes.end() ? attr.stringAttributes.at("offtexture") : "";
  auto toggleOn = attr.stringAttributes.find("state") != attr.stringAttributes.end() && attr.stringAttributes.at("state") == "on";
  auto canToggle = attr.stringAttributes.find("cantoggle") == attr.stringAttributes.end() || !(attr.stringAttributes.at("cantoggle") == "false");
  auto onToggleOn = attr.stringAttributes.find("on") != attr.stringAttributes.end() ? attr.stringAttributes.at("on") : "";
  auto onToggleOff = attr.stringAttributes.find("off") != attr.stringAttributes.end() ? attr.stringAttributes.at("off") : "";
  auto hasOnTint  = attr.vecAttr.vec4.find("ontint") != attr.vecAttr.vec4.end();
  auto onTint = hasOnTint ? attr.vecAttr.vec4.at("ontint") : glm::vec4(1.f, 1.f, 1.f, 1.f);

  GameObjectUIButton obj { 
    .common = parseCommon(attr, util.meshes),
    .initialState = toggleOn,
    .toggleOn = toggleOn,
    .canToggle = canToggle,
    .onTextureString = onTexture,
    .onTexture = util.ensureTextureLoaded(onTexture == "" ? "./res/models/controls/on.png" : onTexture).textureId,
    .offTextureString = offTexture,
    .offTexture = util.ensureTextureLoaded(offTexture == "" ? "./res/models/controls/off.png" : offTexture).textureId,
    .onToggleOn = onToggleOn,
    .onToggleOff = onToggleOff,
    .hasOnTint = hasOnTint,
    .onTint = onTint,
  };
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
  return pairs;
}
