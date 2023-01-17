#include "./scheme_util.h"

unsigned int toUnsignedInt(SCM value){
  return scm_to_unsigned_integer(value, std::numeric_limits<unsigned int>::min(), std::numeric_limits<unsigned int>::max());
}

glm::quat scmListToQuat(SCM rotation){
  auto w = scm_to_double(scm_list_ref(rotation, scm_from_int64(0)));  
  auto x = scm_to_double(scm_list_ref(rotation, scm_from_int64(1)));
  auto y = scm_to_double(scm_list_ref(rotation, scm_from_int64(2)));
  auto z = scm_to_double(scm_list_ref(rotation, scm_from_int64(3)));  
  return  glm::quat(w, x, y, z);
}
SCM scmQuatToSCM(glm::quat rotation){
  SCM list = scm_make_list(scm_from_unsigned_integer(4), scm_from_unsigned_integer(0));
  scm_list_set_x (list, scm_from_unsigned_integer(0), scm_from_double(rotation.w));
  scm_list_set_x (list, scm_from_unsigned_integer(1), scm_from_double(rotation.x));
  scm_list_set_x (list, scm_from_unsigned_integer(2), scm_from_double(rotation.y));
  scm_list_set_x (list, scm_from_unsigned_integer(3), scm_from_double(rotation.z));
  return list;
}

glm::vec2 listToVec2(SCM vecList){
  auto x = scm_to_double(scm_list_ref(vecList, scm_from_int64(0)));   
  auto y = scm_to_double(scm_list_ref(vecList, scm_from_int64(1)));
  return glm::vec2(x, y);
}
glm::vec3 listToVec3(SCM vecList){
  auto x = scm_to_double(scm_list_ref(vecList, scm_from_int64(0)));   
  auto y = scm_to_double(scm_list_ref(vecList, scm_from_int64(1)));
  auto z = scm_to_double(scm_list_ref(vecList, scm_from_int64(2))); 
  return glm::vec3(x, y, z);
}
glm::vec4 listToVec4(SCM vecList){
  auto x = scm_to_double(scm_list_ref(vecList, scm_from_int64(0)));   
  auto y = scm_to_double(scm_list_ref(vecList, scm_from_int64(1)));
  auto z = scm_to_double(scm_list_ref(vecList, scm_from_int64(2))); 
  auto w = scm_to_double(scm_list_ref(vecList, scm_from_int64(3))); 
  return glm::vec4(x, y, z, w);
}

std::vector<glm::vec3> listToVecVec3(SCM vecList){
  std::vector<glm::vec3> vecPoints;
  auto numElements = toUnsignedInt(scm_length(vecList));
  for (int i = 0; i < numElements; i++){
    auto vecValue = scm_list_ref(vecList, scm_from_unsigned_integer(i));
    vecPoints.push_back(listToVec3(vecValue));
  }
  return vecPoints;
}

SCM listToSCM(std::vector<objid> idList){
  SCM list = scm_make_list(scm_from_unsigned_integer(idList.size()), scm_from_unsigned_integer(0));
  for (int i = 0; i < idList.size(); i++){
    scm_list_set_x (list, scm_from_unsigned_integer(i), scm_from_unsigned_integer(idList.at(i))); 
  }
  return list;
}
SCM listToSCM(std::vector<std::string> stringList){
  auto listSize = stringList.size();
  SCM list = scm_make_list(scm_from_unsigned_integer(listSize), scm_from_unsigned_integer(0));
  for (int i = 0; i < listSize; i++){
    scm_list_set_x (list, scm_from_unsigned_integer(i), scm_from_locale_string(stringList.at(i).c_str()));
  }
  return list;
}

SCM listToSCM(std::vector<std::vector<std::string>> stringList){
  auto listSize = stringList.size();
  SCM list = scm_make_list(scm_from_unsigned_integer(listSize), scm_from_unsigned_integer(0));
  for (int i = 0; i < listSize; i++){
    scm_list_set_x (list, scm_from_unsigned_integer(i), listToSCM(stringList.at(i)));
  }
  return list;
}

bool isList(SCM vecList){
  return scm_to_bool(scm_list_p(vecList));
}

SCM vec3ToScmList(glm::vec3 vec){
  SCM list = scm_make_list(scm_from_unsigned_integer(3), scm_from_unsigned_integer(0));
  scm_list_set_x (list, scm_from_unsigned_integer(0), scm_from_double(vec.x));
  scm_list_set_x (list, scm_from_unsigned_integer(1), scm_from_double(vec.y));
  scm_list_set_x (list, scm_from_unsigned_integer(2), scm_from_double(vec.z));
  return list;
}
SCM vec4ToScmList(glm::vec4 vec){
  SCM list = scm_make_list(scm_from_unsigned_integer(4), scm_from_unsigned_integer(0));
  scm_list_set_x (list, scm_from_unsigned_integer(0), scm_from_double(vec.x));
  scm_list_set_x (list, scm_from_unsigned_integer(1), scm_from_double(vec.y));
  scm_list_set_x (list, scm_from_unsigned_integer(2), scm_from_double(vec.z));
  scm_list_set_x (list, scm_from_unsigned_integer(3), scm_from_double(vec.w));
  return list;
}

bool symbolDefinedInModule(const char* symbol, SCM module){
  return scm_to_bool(scm_defined_p(scm_string_to_symbol(scm_from_locale_string(symbol)), module));
}
bool symbolDefined(const char* symbol){
  return symbolDefinedInModule(symbol, scm_current_module());
}
void maybeCallFunc(const char* function){
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_0(func_symbol);
  }   
}
void maybeCallFuncString(const char* function, const char* payload){
  if (symbolDefined(function)){
    SCM func_symbol = scm_variable_ref(scm_c_lookup(function));
    scm_call_1(func_symbol, scm_from_locale_string(payload));
  }   
}

void scmHandleAttrListItem(GameobjAttributes& _attr, SCM firstValue, SCM secondValue){
  auto attrName = scm_to_locale_string(firstValue);
  auto attrValue = secondValue;

  assert(
    _attr.stringAttributes.find(attrName) == _attr.stringAttributes.end() &&
    _attr.vecAttr.vec3.find(attrName) == _attr.vecAttr.vec3.end()
  );


  bool isNumber = scm_is_number(attrValue);
  bool isString = scm_is_string(attrValue);
  bool isList = scm_to_bool(scm_list_p(attrValue));
  assert(isNumber || isString || isList);

  if (isNumber){
    _attr.numAttributes[attrName] = scm_to_double(attrValue);
  }else if (isString){
    _attr.stringAttributes[attrName] = scm_to_locale_string(attrValue);
  }else{
    auto vecListLength = toUnsignedInt(scm_length(attrValue));
    assert(vecListLength == 3 || vecListLength == 4);

    if (vecListLength == 3){
      double values[] = {0, 0, 0};
      for (int j = 0; j < vecListLength; j++){
        auto vecValue = scm_list_ref(attrValue, scm_from_unsigned_integer(j));
        values[j] = scm_to_double(vecValue);
      }
      _attr.vecAttr.vec3[attrName] = glm::vec3(values[0], values[1], values[2]);
    }else{
      double values[] = {0, 0, 0, 0};
      for (int j = 0; j < vecListLength; j++){
        auto vecValue = scm_list_ref(attrValue, scm_from_unsigned_integer(j));
        values[j] = scm_to_double(vecValue);
      }
      _attr.vecAttr.vec4[attrName] = glm::vec4(values[0], values[1], values[2], values[3]);
    }
  }
}

ScmAttributeValue scmToAttributes(SCM scmAttributes){
  std::map<std::string, GameobjAttributes> submodelAttributes;
  ScmAttributeValue attrs {
    .attr = GameobjAttributes {
      .stringAttributes = {},
      .numAttributes = {},
      .vecAttr = vectorAttributes {
        .vec3 = {},
        .vec4 = {}
      },      
    },
    .submodelAttributes = submodelAttributes,
  };

  auto numElements = toUnsignedInt(scm_length(scmAttributes));
  for (int i = 0; i < numElements; i++){
    auto propertyPair = scm_list_ref(scmAttributes, scm_from_unsigned_integer(i));
    auto pairLength = toUnsignedInt(scm_length(propertyPair));
    assert(pairLength == 2 || pairLength == 3);
    bool isSubmodelAttr = pairLength == 3;

    auto firstValue = scm_list_ref(propertyPair, scm_from_unsigned_integer(0));
    auto secondValue = scm_list_ref(propertyPair, scm_from_unsigned_integer(1));
    auto thirdValue = isSubmodelAttr ? scm_list_ref(propertyPair, scm_from_unsigned_integer(2)) : SCM_UNSPECIFIED;

    if (isSubmodelAttr){
      auto submodelName = scm_to_locale_string(firstValue);
      if (attrs.submodelAttributes.find(submodelName) == attrs.submodelAttributes.end()){
        attrs.submodelAttributes[submodelName] = GameobjAttributes {
          .stringAttributes = {},
          .numAttributes = {},
          .vecAttr = {
            .vec3 = {},
            .vec4 = {},
          },
        };
      }
      scmHandleAttrListItem(attrs.submodelAttributes.at(submodelName), secondValue, thirdValue);
    }else{
      scmHandleAttrListItem(attrs.attr, firstValue, secondValue);
    }
  }

  return attrs;
}

AttributeValue toAttributeValue(SCM attrValue){
  bool isNumber = scm_is_number(attrValue);
  bool isString = scm_is_string(attrValue);
  bool isList = scm_to_bool(scm_list_p(attrValue));
  assert(isNumber || isString || isList);
  if (isNumber){
    return scm_to_double(attrValue);
  }
  if (isString){
    return scm_to_locale_string(attrValue);
  }

  auto numElements = toUnsignedInt(scm_length(attrValue));
  assert(numElements == 3 || numElements == 4);
  if (numElements == 4){
    return listToVec4(attrValue);    
  }
  return listToVec3(attrValue);
}
SCM fromAttributeValue(AttributeValue& value){
  auto vecValue = std::get_if<glm::vec3>(&value);
  if (vecValue != NULL){
    return vec3ToScmList(*vecValue);
  }
  auto vec4Value = std::get_if<glm::vec4>(&value);
  if (vec4Value != NULL){
    return vec4ToScmList(*vec4Value);
  }
  auto floatValue = std::get_if<float>(&value);
  if (floatValue != NULL){
    return scm_from_double(*floatValue);
  }
  auto strValue = std::get_if<std::string>(&value);
  if (strValue != NULL){
    return scm_from_locale_string(strValue -> c_str());
  }
  assert(false);
  return SCM_UNSPECIFIED;
}

ObjectValue scmListToObjectValue(SCM list){
  auto listLength = toUnsignedInt(scm_length(list));
  assert(listLength == 3);
  auto object = scm_to_locale_string(scm_list_ref(list, scm_from_unsigned_integer(0)));
  auto attribute = scm_to_locale_string(scm_list_ref(list, scm_from_unsigned_integer(1)));
  auto value = toAttributeValue(scm_list_ref(list, scm_from_unsigned_integer(2)));
  return ObjectValue {
    .object = object,
    .attribute = attribute,
    .value = value,
  };
}

std::vector<std::vector<std::string>> scmToStringList(SCM additionalValues){
  std::vector<std::vector<std::string>> tokens;
  auto numElements = toUnsignedInt(scm_length(additionalValues));
  for (int i = 0; i < numElements; i++){
      std::vector<std::string> token;
      auto objectTokenSCMList = scm_list_ref(additionalValues, scm_from_unsigned_integer(i));
      auto tokenLength = toUnsignedInt(scm_length(objectTokenSCMList));
      for (int j = 0; j < tokenLength; j++){
        auto tokenStr = scm_list_ref(objectTokenSCMList, scm_from_unsigned_integer(j));
        token.push_back(scm_to_locale_string(tokenStr));
      }
      assert(token.size() == 3);
      tokens.push_back(token);
  }
  return tokens;
}

std::vector<std::string> scmToList(SCM stringList){
  std::vector<std::string> values;
  auto numElements = toUnsignedInt(scm_length(stringList));
  for (int i = 0; i < numElements; i++){
    auto strValue = scm_list_ref(stringList, scm_from_unsigned_integer(i));
    values.push_back(scm_to_locale_string(strValue));
  }
  return values;
}

std::optional<optionalValueData> getScmValueIfType(OptionalValueType optType, SCM& scmValue){
  if (optType == OPTIONAL_VALUE_STRING){
    //std::cout << "opt type: string" << std::endl;
    auto isType = scm_is_string(scmValue);
    if (isType){
      auto value = scm_to_locale_string(scmValue);
      //std::cout << "str value is: " << optValueToStr(value) << std::endl;
      return std::string(value);
    }
  }
  else if (optType == OPTIONAL_VALUE_BOOL){
    //std::cout << "opt type: bool" << std::endl;
    auto isType = scm_is_bool(scmValue);
    if (isType){
      bool value = scm_to_bool(scmValue);
      if (value){
        return true;
      }
      return false;
    }
  }else if (optType == OPTIONAL_VALUE_UNSIGNED_INT){
    //std::cout << "opt type: unsigned int" << std::endl;
    auto isType = scm_is_number(scmValue);
    if (isType){
      return toUnsignedInt(scmValue);
    }
  }else if (optType == OPTIONAL_VALUE_INT){
    //std::cout << "opt type: int" << std::endl;
    auto isType = scm_is_number(scmValue);
    if (isType){
      return scm_to_int32(scmValue);
    }
  }else if (optType == OPTIONAL_VALUE_VEC4){
    //std::cout << "opt type: vec4" << std::endl;
    auto isType = isList(scmValue);
    if (isType){
      return listToVec4(scmValue);
    }
  }else if (optType == OPTIONAL_STRING_LIST){
    auto isType = isList(scmValue);
    if (isType){
      return scmToList(scmValue);
    }
  }else{
    modassert(false, "optional values - invalid type");
  }
  return std::nullopt;
}

// scm index
// 
std::vector<std::optional<optionalValueData>> optionalValues(std::vector<OptionalValueType> optValues, std::vector<SCM> scmValues){
  std::vector<std::optional<optionalValueData>> values;
  int scmValueIndex = 0;
  for (int optTypeIndex = 0; optTypeIndex < optValues.size(); optTypeIndex++){
    if (scmValueIndex >= scmValues.size()){
      values.push_back(std::nullopt);
      continue;
    }
    SCM& scmValue = scmValues.at(scmValueIndex);
    auto value = getScmValueIfType(optValues.at(optTypeIndex), scmValue);
    if (value.has_value()){
      values.push_back(value);
      //std::cout << "pushing back: " << optValueToStr(value.value()) << std::endl;
      scmValueIndex++;
    }else{
      //std::cout << "push_back nullopt " << std::endl;
      values.push_back(std::nullopt);
    }
    //std::cout << std::endl;
  }
  return values;
}

std::string optValueToStr(optionalValueData value1){
  auto value1StrPtr = std::get_if<std::string>(&value1);
  if (value1StrPtr != NULL){
    return *value1StrPtr + "(std::string)";
  }
  auto value1BoolPtr = std::get_if<bool>(&value1);
  if (value1BoolPtr != NULL){
    return std::string(*value1BoolPtr ? "true" : "false") + "(bool)";
  }
  auto value1UIntPtr = std::get_if<unsigned int>(&value1);
  if (value1UIntPtr != NULL){
    return std::to_string(*value1UIntPtr) + "(uint)";
  }
  auto value1IntPtr = std::get_if<int>(&value1);
  if (value1IntPtr != NULL){
    return std::to_string(*value1IntPtr) + "(int)";
  }
  auto value1Vec4Ptr = std::get_if<glm::vec4>(&value1);
  if (value1Vec4Ptr != NULL){
    return print(*value1Vec4Ptr) + "(vec4)";
  }  
  auto stringListPtr = std::get_if<std::vector<std::string>>(&value1);
  if (stringListPtr != NULL){
    return std::string("list of size = ") + std::to_string(stringListPtr -> size()) + "(strlist)";
  }
  modassert(false, "optValueToStr invalid value");
  return "";
}