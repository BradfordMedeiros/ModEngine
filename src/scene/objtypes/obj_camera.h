#ifndef MOD_OBJ_CAMERA
#define MOD_OBJ_CAMERA

#include "../../common/util.h"
#include "./obj_util.h"


struct GameObjectCamera {
  bool enableDof;
  float minBlurDistance;
  float maxBlurDistance;
  unsigned int blurAmount;
  std::string target;
};

GameObjectCamera createCamera(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeCamera(GameObjectCamera obj, ObjectSerializeUtil& util);
std::optional<AttributeValuePtr> getCameraAttribute(GameObjectCamera& obj, const char* field);
bool setCameraAttribute(GameObjectCamera& cameraObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&);

#endif