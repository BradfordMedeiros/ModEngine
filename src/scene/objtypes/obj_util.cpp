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

void setTextureAttributes(TextureInformation& info, GameobjAttributes& attr, ObjectSetAttribUtil& util){
  if (attr.stringAttributes.find("textureoffset") != attr.stringAttributes.end()){
    //std::cout << "setting texture offset" << std::endl;
    info.textureoffset = parseVec2(attr.stringAttributes.at("textureoffset"));
  }
  if (attr.stringAttributes.find("texture") != attr.stringAttributes.end()){
    util.releaseTexture(info.textureOverloadId);
    auto textureOverloadName = attr.stringAttributes.at("texture");
    int textureOverloadId = util.ensureTextureLoaded(textureOverloadName).textureId;
    info.textureOverloadName = textureOverloadName;
    info.textureOverloadId = textureOverloadId;
  }
}

void attrSet(GameobjAttributes& attr, bool* value, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    *value = attr.stringAttributes.at(field) == "enabled";
  }
}
void attrSet(GameobjAttributes& attr, std::string* value, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    *value = attr.stringAttributes.at(field);
  }
}
void attrSet(GameobjAttributes& attr, std::string* value, std::string defaultValue, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    *value = attr.stringAttributes.at(field);
  }else{
    *value = defaultValue;
  } 
}
void attrSet(GameobjAttributes& attr, float* value, const char* field){
  if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *value = attr.numAttributes.at(field);
  }
}
void attrSet(GameobjAttributes& attr, float* _value, float defaultValue, const char* field){
  if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *_value = attr.numAttributes.at(field);
  }else{
    *_value = defaultValue;
  }
}
void attrSet(GameobjAttributes& attr, unsigned int* value, const char* field){
  if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *value = static_cast<unsigned int>(attr.numAttributes.at(field));
  }
}


void attrSet(GameobjAttributes& attr, int* _value, const char* field){
  if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *_value = static_cast<int>(attr.numAttributes.at(field));
  }
}

void attrSet(GameobjAttributes& attr, int* _value, int defaultValue, const char* field){
  if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *_value = static_cast<int>(attr.numAttributes.at(field));
  }else{
    *_value = defaultValue;
  }
}

void attrSet(GameobjAttributes& attr, glm::vec3* _value, glm::vec3 defaultValue, const char* field){
  if (attr.vecAttr.vec3.find(field) != attr.vecAttr.vec3.end()){
    *_value = attr.vecAttr.vec3.at(field);
  }else{
    *_value = defaultValue;
  }
  
}

void attrSet(GameobjAttributes& attr, glm::vec4* _value, const char* field){
  if (attr.vecAttr.vec4.find(field) != attr.vecAttr.vec4.end()){
    *_value = attr.vecAttr.vec4.at(field);
  }
}

void attrSet(GameobjAttributes& attr, glm::vec4* _value, glm::vec4 defaultValue, const char* field){
  if (attr.vecAttr.vec4.find(field) != attr.vecAttr.vec4.end()){
    *_value = attr.vecAttr.vec4.at(field);
  }else{
    *_value = defaultValue;
  }
}