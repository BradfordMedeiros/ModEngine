#include "./keymapper.h"


bool isAsciiValue(std::string value){
  return value.size() > 2 && value.at(0) == '\'' && value.at(value.size() - 1) == '\'';
}
int getValue(std::string trimmedValue){
  if (isAsciiValue(trimmedValue)){
    auto payload = trimmedValue.substr(1, trimmedValue.size() - 2);
    assert(payload.size() == 1);
    int character = payload.at(0);
    return character;
  }
  return char(std::atoi(trimmedValue.c_str()));
}

KeyRemapper readMapping(std::string filemapping){
  if (filemapping == ""){
    return KeyRemapper{};
  }

  std::vector<KeyMapping> mapping;
  std::string keyMapperContent = loadFile(filemapping);

  auto mappedLines = filterWhitespace(filterComments(split(keyMapperContent, '\n')));
  for (auto line : mappedLines){
    auto expression = split(line, '>');
    if (expression.size() != 2){
      std::cout << "INFO: key mapping " << filemapping << " is invalid\n" << line << std::endl;
      exit(1);
    }
    int fromChar = getValue(trim(expression.at(0)));
    int secondPart = getValue(trim(expression.at(1)));
    assert(fromChar >= 0);
    assert(secondPart >= 0);
    mapping.push_back(KeyMapping{
      .sourceKey = fromChar,
      .destinationKey = secondPart,
    });
  }

  std::vector<KeyMapping> buttonMappings;                
  buttonMappings.push_back(KeyMapping {
    .sourceKey = 0,
    .destinationKey = 65,
  });
  buttonMappings.push_back(KeyMapping {
    .sourceKey = 1,
    .destinationKey = 66,
  });

  std::vector<KeyAxisConfiguration> axisConfigurations; 
  axisConfigurations.push_back(KeyAxisConfiguration{
    .index = 1,
    .deadzonemin = -0.1f,
    .deadzonemax = 0.1f,
  });
  axisConfigurations.push_back(KeyAxisConfiguration{
    .index = 0,
    .deadzonemin = -0.1f,
    .deadzonemax = 0.1f,
  });

  KeyRemapper remapper {
    .mapping = mapping,
    .buttonMappings = buttonMappings,
    .axisConfigurations = axisConfigurations,
  };
  return remapper;
}

int getKeyRemapping(KeyRemapper& keymapper, int key){
  int remappedKey = key;
  for (auto remapping : keymapper.mapping){
    if (remapping.sourceKey == key){
      remappedKey = remapping.destinationKey;
      break;
    }
  }
  return remappedKey;  
}

KeyAxisConfiguration getAxisConfig(KeyRemapper& keymapper, int index){
  for (auto axisConfig : keymapper.axisConfigurations){
    if (axisConfig.index == index){
      return axisConfig;
    }
  }
  assert(false);
  return KeyAxisConfiguration{};
}