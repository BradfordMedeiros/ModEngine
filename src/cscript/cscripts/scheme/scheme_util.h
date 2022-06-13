#ifndef SCHEME_UTIL
#define SCHEME_UTIL

#include <libguile.h>
#include <limits>       
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "../../../common/util.h"

unsigned int toUnsignedInt(SCM value);
glm::quat scmListToQuat(SCM rotation);
SCM scmQuatToSCM(glm::quat rotation);
glm::vec3 listToVec3(SCM vecList);
glm::vec4 listToVec4(SCM vecList);
std::vector<glm::vec3> listToVecVec3(SCM vecList);
SCM listToSCM(std::vector<objid> idList);
SCM listToSCM(std::vector<std::string> stringList);
SCM listToSCM(std::vector<std::vector<std::string>> stringList);
bool isList(SCM vecList);
SCM vec3ToScmList(glm::vec3 vec);
SCM vec4ToScmList(glm::vec4 vec);
bool symbolDefinedInModule(const char* symbol, SCM module);
bool symbolDefined(const char* symbol);
void maybeCallFunc(const char* function);
void maybeCallFuncString(const char* function, const char* payload);
SCM fromAttributeValue(AttributeValue& value);
AttributeValue toAttributeValue(SCM attrValue);
GameobjAttributes scmToAttributes(SCM scmAttributes);
ObjectValue scmListToObjectValue(SCM list);
std::vector<std::vector<std::string>> scmToStringList(SCM additionalValues);

struct OptionalValues{
  std::optional<glm::vec4> tint;
  std::optional<unsigned int> textureId;
  bool perma;
};
OptionalValues optionalOpts(SCM opt1, SCM opt2, SCM opt3);

#endif