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

std::vector<std::string> listToVecString(SCM stringList){
  std::vector<std::string> list;
  auto numElements = toUnsignedInt(scm_length(stringList));
  for (int i = 0; i < numElements; i++){
    auto stringValue = scm_to_locale_string(scm_list_ref(stringList, scm_from_unsigned_integer(i)));
    list.push_back(stringValue);
  }
  return list;
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

SCM vec3ToScmList(glm::vec3 vec){
  SCM list = scm_make_list(scm_from_unsigned_integer(3), scm_from_unsigned_integer(0));
  scm_list_set_x (list, scm_from_unsigned_integer(0), scm_from_double(vec.x));
  scm_list_set_x (list, scm_from_unsigned_integer(1), scm_from_double(vec.y));
  scm_list_set_x (list, scm_from_unsigned_integer(2), scm_from_double(vec.z));
  return list;
}

SCM nestedVecToSCM(std::vector<std::vector<std::string>>& list){
  SCM scmList = scm_make_list(scm_from_unsigned_integer(list.size()), scm_from_unsigned_integer(0));
  for (int i = 0; i < list.size(); i++){
    auto sublist = list.at(i);
    SCM scmSubList = scm_make_list(scm_from_unsigned_integer(sublist.size()), scm_from_unsigned_integer(0));
    for (int j = 0; j < sublist.size(); j++){
      scm_list_set_x (scmSubList, scm_from_unsigned_integer(j), scm_from_locale_string(sublist.at(j).c_str()));
    }
    scm_list_set_x(scmList, scm_from_unsigned_integer(i), scmSubList);
  }
  return scmList;
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
  std::map<std::string, glm::vec3> vecAttributes;

  auto numElements = toUnsignedInt(scm_length(scmAttributes));
  for (int i = 0; i < numElements; i++){
    auto propertyPair = scm_list_ref(scmAttributes, scm_from_unsigned_integer(i));
    auto pairLength = toUnsignedInt(scm_length(propertyPair));
    assert(pairLength == 2);
    auto attrName = scm_to_locale_string(scm_list_ref(propertyPair, scm_from_unsigned_integer(0)));
    assert(
      stringAttributes.find(attrName) == stringAttributes.end() &&
      vecAttributes.find(attrName) == vecAttributes.end()
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
      auto vec3ListLength = toUnsignedInt(scm_length(attrValue));
      assert(vec3ListLength == 3);

      double values[] = {0, 0, 0};
      for (int j = 0; j < vec3ListLength; j++){
        auto vecValue = scm_list_ref(attrValue, scm_from_unsigned_integer(j));
        values[j] = scm_to_double(vecValue);
      }
      vecAttributes[attrName] = glm::vec3(values[0], values[1], values[2]);
    }
  }
  GameobjAttributes attrs { 
    .stringAttributes = stringAttributes,
    .numAttributes = numAttributes,
    .vecAttributes = vecAttributes,
  };
  return attrs;
}