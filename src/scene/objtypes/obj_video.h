#ifndef MOD_OBJ_VIDEO
#define MOD_OBJ_VIDEO

#include "../../common/util.h"
#include "./obj_util.h"
#include "../../webm.h"
#include "./sound/sound.h"

struct GameObjectVideo {
  VideoContent videoContent;
  BufferedAudio sound;
  Texture texture;
  bool drawMesh;
  std::string texturePath;
};

GameObjectVideo createVideoObj(GameobjAttributes& attr, ObjectTypeUtil& util);
void removeVideoObj(GameObjectVideo& navmeshObj, ObjectRemoveUtil& util);

std::vector<std::pair<std::string, std::string>> serializeVideo(GameObjectVideo& obj, ObjectSerializeUtil& util);
std::optional<AttributeValuePtr> getVideoAttribute(GameObjectVideo& obj, const char* field);
bool setVideoAttribute(GameObjectVideo& obj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&);
void onVideoObjFrame(GameObjectVideo& obj, float currentTime);

#endif