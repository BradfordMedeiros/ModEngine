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