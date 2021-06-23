#ifndef SCHEME_UTIL
#define SCHEME_UTIL

#include <libguile.h>
#include <limits>       
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../common/util.h"

unsigned int toUnsignedInt(SCM value);
glm::quat scmListToQuat(SCM rotation);
SCM scmQuatToSCM(glm::quat rotation);
glm::vec3 listToVec3(SCM vecList);
std::vector<std::string> listToVecString(SCM stringList);
std::vector<glm::vec3> listToVecVec3(SCM vecList);
SCM listToSCM(std::vector<std::string> stringList);
SCM listToSCM(std::vector<std::vector<std::string>> stringList);
SCM vec3ToScmList(glm::vec3 vec);
bool symbolDefinedInModule(const char* symbol, SCM module);
bool symbolDefined(const char* symbol);
void maybeCallFunc(const char* function);
void maybeCallFuncString(const char* function, const char* payload);
GameobjAttributes scmToAttributes(SCM scmAttributes);

#endif