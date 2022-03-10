#include "./obj_util.h"

GameObjectUICommon parseCommon(GameobjAttributes& attr, std::map<std::string, MeshRef>& meshes){
  auto onFocus = attr.stringAttributes.find("focus") != attr.stringAttributes.end() ? attr.stringAttributes.at("focus") : "";
  auto onBlur = attr.stringAttributes.find("blur") != attr.stringAttributes.end() ? attr.stringAttributes.at("blur") : "";
  GameObjectUICommon common {
    .mesh = meshes.at("./res/models/controls/input.obj").mesh,
    .isFocused = false,
    .onFocus = onFocus,
    .onBlur = onBlur,
  };
  return common;
}

void addSerializeCommon(std::vector<std::pair<std::string, std::string>>& pairs, GameObjectUICommon& common){
  if (common.onFocus != ""){
    pairs.push_back(std::pair<std::string, std::string>("focus", common.onFocus));
  }
  if (common.onBlur != ""){
    pairs.push_back(std::pair<std::string, std::string>("blur", common.onBlur));
  }
}

TextureInformation texinfoFromFields(GameobjAttributes& attr, std::function<Texture(std::string)> ensureTextureLoaded){
  glm::vec2 textureoffset = attr.stringAttributes.find("textureoffset") == attr.stringAttributes.end() ? glm::vec2(0.f, 0.f) : parseVec2(attr.stringAttributes.at("textureoffset"));
  glm::vec2 texturetiling = attr.stringAttributes.find("texturetiling") == attr.stringAttributes.end() ? glm::vec2(1.f, 1.f) : parseVec2(attr.stringAttributes.at("texturetiling"));
  glm::vec2 texturesize = attr.stringAttributes.find("texturesize") == attr.stringAttributes.end() ? glm::vec2(1.f, 1.f) : parseVec2(attr.stringAttributes.at("texturesize"));
  std::string textureOverloadName = attr.stringAttributes.find("texture") == attr.stringAttributes.end() ? "" : attr.stringAttributes.at("texture");
  int textureOverloadId = textureOverloadName == "" ? -1 : ensureTextureLoaded(textureOverloadName).textureId;

  TextureInformation info {
    .textureoffset = textureoffset,
    .texturetiling = texturetiling,
    .texturesize = texturesize,
    .textureOverloadName = textureOverloadName,
    .textureOverloadId = textureOverloadId,
  };
  return info;
}

void addSerializedTextureInformation(std::vector<std::pair<std::string, std::string>>& pairs, TextureInformation& texture){
  if (texture.textureoffset.x != 0.f && texture.textureoffset.y != 0.f){
    pairs.push_back(std::pair<std::string, std::string>("textureoffset", serializeVec(texture.textureoffset)));
  }
  if (texture.textureOverloadName != ""){
    pairs.push_back(std::pair<std::string, std::string>("texture", texture.textureOverloadName));
  }
  if (texture.texturesize.x != 1.f && texture.texturesize.y != 1.f){
    pairs.push_back(std::pair<std::string, std::string>("texturesize", serializeVec(texture.texturesize)));
  }
}