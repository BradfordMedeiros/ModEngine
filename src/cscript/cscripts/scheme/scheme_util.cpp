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

GameobjAttributes scmToAttributes(SCM scmAttributes){
  std::map<std::string, double> numAttributes;
  std::map<std::string, std::string> stringAttributes;
  std::map<std::string, glm::vec3> vec3Attributes;
  std::map<std::string, glm::vec4> vec4Attributes;

  auto numElements = toUnsignedInt(scm_length(scmAttributes));
  for (int i = 0; i < numElements; i++){
    auto propertyPair = scm_list_ref(scmAttributes, scm_from_unsigned_integer(i));
    auto pairLength = toUnsignedInt(scm_length(propertyPair));
    assert(pairLength == 2);
    auto attrName = scm_to_locale_string(scm_list_ref(propertyPair, scm_from_unsigned_integer(0)));
    assert(
      stringAttributes.find(attrName) == stringAttributes.end() &&
      vec3Attributes.find(attrName) == vec3Attributes.end()
    );

    auto attrValue = scm_list_ref(propertyPair, scm_from_unsigned_integer(1));

    bool isNumber = scm_is_number(attrValue);
    bool isString = scm_is_string(attrValue);
    bool isList = scm_to_bool(scm_list_p(attrValue));
    assert(isNumber || isString || isList);

    if (isNumber){
      numAttributes[attrName] = scm_to_double(attrValue);
    }else if (isString){
      stringAttributes[attrName] = scm_to_locale_string(attrValue);
    }else{
      auto vecListLength = toUnsignedInt(scm_length(attrValue));
      assert(vecListLength == 3 || vecListLength == 4);

      if (vecListLength == 3){
        double values[] = {0, 0, 0};
        for (int j = 0; j < vecListLength; j++){
          auto vecValue = scm_list_ref(attrValue, scm_from_unsigned_integer(j));
          values[j] = scm_to_double(vecValue);
        }
        vec3Attributes[attrName] = glm::vec3(values[0], values[1], values[2]);
      }else{
        double values[] = {0, 0, 0, 0};
        for (int j = 0; j < vecListLength; j++){
          auto vecValue = scm_list_ref(attrValue, scm_from_unsigned_integer(j));
          values[j] = scm_to_double(vecValue);
        }
        vec4Attributes[attrName] = glm::vec4(values[0], values[1], values[2], values[3]);
      }

    }
  }
  GameobjAttributes attrs { 
    .stringAttributes = stringAttributes,
    .numAttributes = numAttributes,
    .vecAttr = vectorAttributes {
      .vec3 = vec3Attributes,
      .vec4 = vec4Attributes
    },
  };
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

OptionalValues optionalOpts(SCM opt1, SCM opt2, SCM opt3){
  bool value1Defined = !scm_is_eq(opt1, SCM_UNDEFINED);
  bool value2Defined = !scm_is_eq(opt2, SCM_UNDEFINED);
  bool value3Defined = !scm_is_eq(opt3, SCM_UNDEFINED);
  std::optional<glm::vec4> color = std::nullopt;
  bool isPermaText = false;
  std::optional<unsigned int> textureId = std::nullopt;

  bool colorDefined = false;
  bool permaDefined = false;
  bool textureIdDefined = false;

  // Should really create a util for this pattern
  // Basic idea is that you can omit the type for any slot, and pass arg to the next parameter and omit types on the right (types must be different to disttinguish)
  if (value1Defined){
     if (isList(opt1)){
        color = listToVec4(opt1);
        colorDefined = true;
     }else if (scm_is_bool(opt1)){
        isPermaText = scm_to_bool(opt1);
        permaDefined = true;
     }else{
       textureId = toUnsignedInt(opt1);
       textureIdDefined = true;
     }
  }
  if (value2Defined){
     if (scm_is_bool(opt2)){
       assert(!permaDefined);
       isPermaText = scm_to_bool(opt2);
       permaDefined = true;
     }else{
       assert(!textureIdDefined);
       textureId = toUnsignedInt(opt2);
       textureIdDefined = true;
     }
  }
  if (value3Defined){
   assert(!textureIdDefined);
   textureId = toUnsignedInt(opt3);
   textureIdDefined = true;
  }

  OptionalValues opts {
    .tint = color,
    .textureId = textureId,
    .perma = isPermaText,
  };
  return opts;
}

//enum OptionalValueType { OPTIONAL_VALUE_UNSIGNED_INT, OPTIONAL_VALUE_INT, OPTIONAL_VALUE_STRING, OPTIONAL_VALUE_BOOL, OPTIONAL_VALUE_VEC4 };
std::vector<std::optional<optionalValueData>> optionalValues(std::vector<OptionalValueType> optValues, std::vector<SCM> scmValues){
  std::vector<std::optional<optionalValueData>> values;
  for (int scmValueIndex = 0; scmValueIndex < optValues.size(); scmValueIndex++){
    
    bool foundType = false;
    for (int optValueIndex = scmValueIndex; optValueIndex < optValues.size(); optValueIndex++){
      if (scmValueIndex >= scmValues.size()){
        break;
      }
      SCM& scmValue = scmValues.at(scmValueIndex);
      auto optType = optValues.at(optValueIndex);
      //enum OptionalValueType { OPTIONAL_VALUE_UNSIGNED_INT, OPTIONAL_VALUE_INT, OPTIONAL_VALUE_STRING, OPTIONAL_VALUE_BOOL, OPTIONAL_VALUE_VEC4 };
      if (optType == OPTIONAL_VALUE_UNSIGNED_INT){
        auto isType = scm_is_number(scmValue);
        if (isType){
          values.push_back(toUnsignedInt(scmValue));
          foundType = true;
        }
      }else if (optType == OPTIONAL_VALUE_INT){
        auto isType = scm_is_number(scmValue);
        if (isType){
          values.push_back(scm_to_int32(scmValue));
          foundType = true;
        }
      }else if (optType == OPTIONAL_VALUE_STRING){
        auto isType = scm_is_string(scmValue);
        if (isType){
          values.push_back(scm_to_locale_string(scmValue));
          foundType = true;
        }
      }else if (optType == OPTIONAL_VALUE_BOOL){
        auto isType = scm_is_bool(scmValue);
        if (isType){
          values.push_back(scm_to_bool(scmValue));
          foundType = true;
        }
      }else if (optType == OPTIONAL_VALUE_VEC4){
        auto isType = isList(scmValue);
        if (isType){
          values.push_back(listToVec4(scmValue));
          foundType = true;
        }
      }else{
        modassert(false, "optional values - invalid type");
      }
    }
    if (!foundType){
      values.push_back(std::nullopt); 
    }
  }
  return values;
}