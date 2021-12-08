#ifndef MOD_OBJ_CAMERA
#define MOD_OBJ_CAMERA

#include "../../common/util.h"
#include "./obj_util.h"


struct GameObjectCamera {
  bool enableDof;
  float minBlurDistance;
  float maxBlurDistance;
};

GameObjectCamera createCamera(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeCamera(GameObjectCamera obj, ObjectSerializeUtil& util);
void cameraObjAttr(GameObjectCamera& meshObj, GameobjAttributes& _attributes);
void setCameraAttributes(GameObjectCamera& meshObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

#endif