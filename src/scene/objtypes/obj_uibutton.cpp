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
  AutoSerializeBool {  // mark as not serializable
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

  /* std::string onTextureString;
  int onTexture;
  std::string offTextureString;
  int offTexture;*/
  AutoSerializeTextureLoaderManual{
    .structOffset = offsetof(GameObjectUIButton, onTexture),
    .field = "ontexture",
    .defaultValue = "./res/models/controls/on.png",
  },
  AutoSerializeTextureLoaderManual {
    .structOffset = offsetof(GameObjectUIButton, offTexture),
    .field = "offtexture",
    .defaultValue = "./res/models/controls/off.png",
  },
};

static int _ = addCommonAutoserializer<GameObjectUIButton>(uiButtonAutoserializer);


GameObjectUIButton createUIButton(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectUIButton obj {};
  createAutoSerialize((char*)&obj, uiButtonAutoserializer, attr, util);
  obj.common.mesh = util.meshes.at("./res/models/controls/input.obj").mesh;
  obj.common.isFocused = false;

  autoserializeHandleTextureLoading((char*)&obj, uiButtonAutoserializer, util.ensureTextureLoaded, util.releaseTexture);

  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeButton(GameObjectUIButton& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&obj, uiButtonAutoserializer, pairs);
  return pairs;
}

void getUIButtonAttributes(GameObjectUIButton& uiButton, GameobjAttributes& _attributes){
  autoserializerGetAttr((char*)&uiButton, uiButtonAutoserializer, _attributes);
}

void setUIButtonAttributes(GameObjectUIButton& buttonObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  autoserializerSetAttr((char*)&buttonObj, uiButtonAutoserializer, attributes, util);
}