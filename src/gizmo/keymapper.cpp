#include "./keymapper.h"

bool isAsciiValue(std::string value){
  return value.size() > 2 && value.at(0) == '\'' && value.at(value.size() - 1) == '\'';
}
int parseAsciiValue(std::string trimmedValue){
  auto payload = trimmedValue.substr(1, trimmedValue.size() - 2);
  assert(payload.size() == 1);
  int character = payload.at(0);
  return character; 
}

bool isNamedValue(std::string value){
  return value.at(0) == ':';
}

struct NamedValue {
  std::string command;
  int index;
};
NamedValue parseNamedValue(std::string value){
  return NamedValue {
    .command = "button", 
    .index = 0,
  };
}

int parseCharValue(std::string trimmedValue){
  return char(std::atoi(trimmedValue.c_str()));
}

enum KeyValueType { KEY_VALUE_ASCII, KEY_VALUE_CHAR, KEY_VALUE_NAMED };
KeyValueType getKeyValueType(std::string trimmedValue){
  if (isAsciiValue(trimmedValue)){
    return KEY_VALUE_ASCII;
  }else if (isNamedValue(trimmedValue)){
    return KEY_VALUE_NAMED;
  }
  return KEY_VALUE_CHAR;
}




KeyRemapper readMapping(std::string filemapping){
  if (filemapping == ""){
    return KeyRemapper{};
  }

  std::vector<KeyMapping> mapping;
  std::vector<KeyMapping> buttonMappings;                
  std::string keyMapperContent = loadFile(filemapping);

  auto mappedLines = filterWhitespace(filterComments(split(keyMapperContent, '\n')));
  for (auto line : mappedLines){
    auto expression = split(line, '>');
    if (expression.size() != 2){
      std::cout << "INFO: key mapping " << filemapping << " is invalid\n" << line << std::endl;
      exit(1);
    }

    auto expr1 = trim(expression.at(0));
    auto expr2 = trim(expression.at(1));
    auto keyType = getKeyValueType(expr1);
    auto keyType2 = getKeyValueType(expr2);

    if ((keyType == KEY_VALUE_ASCII  || keyType == KEY_VALUE_CHAR) && (keyType2 == KEY_VALUE_ASCII || keyType2 == KEY_VALUE_CHAR)){
      int fromChar = keyType == KEY_VALUE_ASCII ?  parseAsciiValue(expr1) : parseCharValue(expr1);
      int secondPart = keyType2 == KEY_VALUE_ASCII ?  parseAsciiValue(expr2) : parseCharValue(expr2);
      assert(fromChar >= 0);
      assert(secondPart >= 0);
      mapping.push_back(KeyMapping{
        .sourceKey = fromChar,
        .destinationKey = secondPart,
      });
    }else if (keyType == KEY_VALUE_NAMED && (keyType2 == KEY_VALUE_ASCII || keyType2 == KEY_VALUE_CHAR)){
      std::cout << "Keymapping: not yet implemented" << std::endl;
      auto namedValue = parseNamedValue(expr1);
      int secondPart = keyType2 == KEY_VALUE_ASCII ?  parseAsciiValue(expr2) : parseCharValue(expr2);
      if (namedValue.command == "button"){
        buttonMappings.push_back(KeyMapping {
          .sourceKey = namedValue.index,
          .destinationKey = secondPart,
        });
      }
    }else{
      std::cout << "Keymapping: combination not supported" << std::endl;
    }

  }

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

  axisConfigurations.push_back(KeyAxisConfiguration{
    .index = 5,
    .shouldMapKey = true,
    .amount = 0.7,
    .destinationKey = 68,
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