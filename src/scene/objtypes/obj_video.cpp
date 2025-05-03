#include "./obj_video.h"

std::vector<AutoSerialize> videoAutoserializer {
  //AutoSerializeBool {
  //  .structOffset = offsetof(GameObjectCamera, enableDof),
  //  .field = "dof",
  //  .onString = "enabled",
  //  .offString = "disabled",
  //  .defaultValue = false,
  //},

};

GameObjectVideo createVideoObj(GameobjAttributes& attr, ObjectTypeUtil& util){
	return GameObjectVideo{};
}

void removeVideoObj(GameObjectVideo& navmeshObj, ObjectRemoveUtil& util){
   modassert(false, "remove video here");
}

std::vector<std::pair<std::string, std::string>> serializeVideo(GameObjectVideo& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&obj, videoAutoserializer, pairs);
  return pairs;
}

std::optional<AttributeValuePtr> getVideoAttribute(GameObjectVideo& obj, const char* field){
  return getAttributePtr((char*)&obj, videoAutoserializer, field);
}

bool setVideoAttribute(GameObjectVideo& obj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&){
  return autoserializerSetAttrWithTextureLoading((char*)&obj, videoAutoserializer, field, value, util);
}
