#include "./scheme_util_test.h"

struct optsTestData {
	std::vector<OptionalValueType> types;
	std::vector<SCM> passedArgs;
	std::vector<std::optional<optionalValueData>> expectedValues;
};


//enum OptionalValueType { OPTIONAL_VALUE_UNSIGNED_INT, OPTIONAL_VALUE_INT, OPTIONAL_VALUE_STRING, OPTIONAL_VALUE_BOOL, OPTIONAL_VALUE_VEC4 };
//typedef std::variant<unsigned int, int, std::string, bool, glm::vec4> optionalValueData;

bool optValuesEqual(optionalValueData value1, optionalValueData value2){
  auto value1BoolPtr = std::get_if<bool>(&value1);
  auto value2BoolPtr = std::get_if<bool>(&value2);
  if (value1BoolPtr != NULL && value2BoolPtr != NULL){
    return *value1BoolPtr == *value2BoolPtr;
  }

  auto value1UIntPtr = std::get_if<unsigned int>(&value1);
  auto value2UIntPtr = std::get_if<unsigned int>(&value2);
  if (value1UIntPtr != NULL && value2UIntPtr != NULL){
    return *value1UIntPtr == *value2UIntPtr;
  }

  auto value1IntPtr = std::get_if<int>(&value1);
  auto value2IntPtr = std::get_if<int>(&value2);
  if (value1IntPtr != NULL && value2IntPtr != NULL){
    return *value1IntPtr == *value2IntPtr;
  }

  auto value1StrPtr = std::get_if<std::string>(&value1);
  auto value2StrPtr = std::get_if<std::string>(&value2);
  if (value1StrPtr != NULL && value2StrPtr != NULL){
    return *value1StrPtr == *value2StrPtr;
  }

  auto value1Vec4Ptr = std::get_if<glm::vec4>(&value1);
  auto value2Vec4Ptr = std::get_if<glm::vec4>(&value2);
  if (value1Vec4Ptr != NULL && value2Vec4Ptr != NULL){
    return *value1Vec4Ptr == *value2Vec4Ptr;
  }

	return false;
}

void optionalValueDataTest(){
  scm_init_guile();  // should be organized better to work w/ other unit tests...
	std::vector<optsTestData> optsTests = {
    optsTestData {    
     	.types =  {},
     	.passedArgs = {},
     	.expectedValues = {},
    },
    optsTestData {    
     	.types =  { OPTIONAL_VALUE_BOOL, OPTIONAL_VALUE_VEC4, OPTIONAL_VALUE_INT },
     	.passedArgs = {},
     	.expectedValues = { std::nullopt, std::nullopt, std::nullopt },
    },
    optsTestData {    
      .types =  { OPTIONAL_VALUE_BOOL, OPTIONAL_VALUE_VEC4, OPTIONAL_VALUE_INT },
      .passedArgs = { vec4ToScmList(glm::vec4(2.f, 3.f, 4.f, 1.f)), scm_from_int32(45) },
      .expectedValues = { std::nullopt, glm::vec4(2.f, 3.f, 4.f, 1.f), 45 },
    },
    optsTestData {    
     	.types =  { OPTIONAL_VALUE_BOOL  },
     	.passedArgs = {  },
     	.expectedValues = { std::nullopt },
    },
    optsTestData {    
     	.types =  { OPTIONAL_VALUE_BOOL  },
     	.passedArgs = { SCM_BOOL_T },
     	.expectedValues = { true },
    },
    optsTestData {    
      .types =  { OPTIONAL_VALUE_BOOL, OPTIONAL_VALUE_VEC4  },
      .passedArgs = { SCM_UNSPECIFIED, SCM_UNDEFINED },
      .expectedValues = { std::nullopt, std::nullopt },
    },
    optsTestData {    
      .types =  { OPTIONAL_VALUE_STRING },
      .passedArgs = { scm_from_locale_string("hello") },
      .expectedValues = { "hello" },
    },
    optsTestData {    
      .types =  {OPTIONAL_VALUE_STRING, OPTIONAL_VALUE_VEC4, OPTIONAL_VALUE_BOOL, OPTIONAL_VALUE_UNSIGNED_INT},
      .passedArgs = { scm_from_utf8_string("hello"), vec4ToScmList(glm::vec4(1.f, 1.f, 1.f, 1.f)), scm_from_bool(false), scm_from_int32(10) },
      .expectedValues = { "hello", glm::vec4(1.f, 1.f, 1.f, 1.f), false, (unsigned int)10 },
    },
  };

  for (int i = 0; i < optsTests.size(); i++){
    auto optTest = optsTests.at(i);
    auto args = optionalValues(optTest.types, optTest.passedArgs);
    if (args.size() != optTest.expectedValues.size()){
   	 	throw std::logic_error(std::string("actual size: ") + std::to_string(args.size()) + ", expected: " + std::to_string(optTest.expectedValues.size()));
  	}

    //std::cout << "args: i: " << i << " - ";
    //for (auto &arg : args){
    //  std::cout << "[" << (!arg.has_value() ? "nullopt" : optValueToStr(arg.value())) << "] ";
    //}
    //std::cout << std::endl;
    //std::cout << "expected: i: " << i << " - ";
    //for (auto &arg : optTest.expectedValues){
    //  std::cout << "[" << (!arg.has_value() ? "nullopt" : optValueToStr(arg.value())) << "] ";
    //}
    //std::cout << std::endl << std::endl;

  	for (int valueIndex = 0; valueIndex < optTest.expectedValues.size(); valueIndex++){
  		auto expectedValue = optTest.expectedValues.at(valueIndex);
  		auto actualValue = args.at(valueIndex);
  		auto type = optTest.types.at(valueIndex);

  		if (!expectedValue.has_value() && !actualValue.has_value()){
  			continue;
  		}
  		if ((expectedValue.has_value() && !actualValue.has_value()) || (!expectedValue.has_value() && actualValue.has_value())){
  			throw std::logic_error(std::to_string(i) + " has value: actual: " + (actualValue.has_value() ? "true" : "false") + ", expected: " + (expectedValue.has_value() ? "true" : "false"));
  		}

  		if (!optValuesEqual(expectedValue.value(), actualValue.value())){
  			throw std::logic_error(std::to_string(i) + " - values do not match - " + optValueToStr(expectedValue.value()) + " but actual: " + optValueToStr(actualValue.value()));
  		}

  	}

  }
}
