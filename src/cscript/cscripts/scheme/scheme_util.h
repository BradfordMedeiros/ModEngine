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
glm::vec2 listToVec2(SCM vecList);
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

struct ScmAttributeValue {
  GameobjAttributes attr;
  std::map<std::string, GameobjAttributes> submodelAttributes;
};
ScmAttributeValue scmToAttributes(SCM scmAttributes);
ObjectValue scmListToObjectValue(SCM list);
std::vector<std::vector<std::string>> scmToStringList(SCM additionalValues);
std::vector<std::string> scmToList(SCM stringList);

enum OptionalValueType { OPTIONAL_VALUE_UNSIGNED_INT, OPTIONAL_VALUE_INT, OPTIONAL_VALUE_STRING, OPTIONAL_VALUE_BOOL, OPTIONAL_VALUE_VEC4, OPTIONAL_STRING_LIST };
typedef std::variant<unsigned int, int, std::string, bool, glm::vec4, std::vector<std::string>> optionalValueData;
std::vector<std::optional<optionalValueData>> optionalValues(
  std::vector<OptionalValueType> optValues, 
  std::vector<SCM> scmValues
);
std::string optValueToStr(optionalValueData value1);

template <typename T>
T getOptValue(std::optional<optionalValueData>& value, T defaultValue){
  if (!value.has_value()){
    return defaultValue;
  }
  auto ptrValue = std::get_if<T>(&value.value());
  modassert(ptrValue != NULL, "ptr value is null");
  return *ptrValue;
}

template <typename T>
std::optional<T> optionalTypeFromVariant (std::optional<optionalValueData>& value){
  if (!value.has_value()){
    return std::nullopt;
  }
  //std::cout << "variant: " << optValueToStr(value.value()) << std::endl;
  auto unwrappedValue = value.value();
  auto valuePtr = std::get_if<T>(&unwrappedValue);
  modassert(valuePtr != NULL, "optionalTypeFromVariant optional value is null invalid type");
  return *valuePtr;
}


#endif