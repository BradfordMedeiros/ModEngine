#ifndef MOD_OBJ_CAMERA
#define MOD_OBJ_CAMERA

#include "../../common/util.h"
#include "./obj_util.h"


struct GameObjectCamera {
  bool enableDof;
  float minBlurDistance;
  float maxBlurDistance;
  std::string target;
};

GameObjectCamera createCamera(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeCamera(GameObjectCamera obj, ObjectSerializeUtil& util);
void cameraObjAttr(GameObjectCamera& cameraObj, GameobjAttributes& _attributes);
void setCameraAttributes(GameObjectCamera& cameraObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

#endif