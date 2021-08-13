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
  float deadzonemin;
  float deadzonemax;
  float trigger;
};
NamedValue parseNamedValue(std::string value){
  auto parts = filterWhitespace(split(value, ':'));
  auto command = parts.at(0);
  auto index = std::atoi(parts.at(1).c_str());

  float deadzonemin = 0.f;
  float deadzonemax = 0.f;
  float trigger = 1.f;

  if (command == "axis-deadzone"){
    deadzonemin = std::atof(parts.at(2).c_str());
    deadzonemax = std::atof(parts.at(3).c_str());
  }
  if (command == "axis-trigger"){
    trigger = std::atof(parts.at(2).c_str());
  }

  return NamedValue {
    .command = command, 
    .index = index,
    .deadzonemin = deadzonemin,
    .deadzonemax = deadzonemax,
    .trigger = trigger,
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

KeyRemapper readMapping(std::string filemapping, std::vector<InputDispatch> inputFns){
  if (filemapping == ""){
    return KeyRemapper{
      .inputFns = inputFns,
    };
  }

  std::vector<KeyMapping> mapping;
  std::map<int, KeyAxisConfiguration> axisConfigurations; 
  std::string keyMapperContent = loadFile(filemapping);

  auto mappedLines = filterWhitespace(filterComments(split(keyMapperContent, '\n')));
  for (auto line : mappedLines){
    auto expression = split(line, '>');

    if (expression.size() == 1){
      auto data = trim(expression.at(0));
      auto keyType = getKeyValueType(data);
      if (keyType == KEY_VALUE_NAMED){
        auto value = parseNamedValue(data);
        if (value.command == "axis-deadzone"){
          if (axisConfigurations.find(value.index) == axisConfigurations.end()){
            axisConfigurations[value.index] = KeyAxisConfiguration{};
          }
          axisConfigurations[value.index].deadzonemin = value.deadzonemin;
          axisConfigurations[value.index].deadzonemax = value.deadzonemax;
        }else{
          std::cout << "invalid command type" << std::endl;
          assert(false);
        }
      }else{
        std::cout << "invalid value -- " <<  data << std::endl;
        assert(false);
      }
    }else if (expression.size() == 2){
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
          if (axisConfigurations.find(namedValue.index) == axisConfigurations.end()){
            axisConfigurations[namedValue.index] = KeyAxisConfiguration{};
          }
          axisConfigurations[namedValue.index].hasKeyMapping = true;
          axisConfigurations[namedValue.index].mapping = KeyMapping {
            .sourceKey = namedValue.index,
            .destinationKey = secondPart,
          };
        }else if (namedValue.command == "axis-trigger"){
          if (axisConfigurations.find(namedValue.index) == axisConfigurations.end()){
            axisConfigurations[namedValue.index] = KeyAxisConfiguration{};
          }
          axisConfigurations[namedValue.index].shouldMapKey = true;
          axisConfigurations[namedValue.index].amount = namedValue.trigger;
          axisConfigurations[namedValue.index].destinationKey = secondPart;
        }
      }else{
        std::cout << "Keymapping: combination not supported" << std::endl;
      }

      }else{
        std::cout << "INFO: key mapping " << filemapping << " is invalid\n" << line << std::endl;
        exit(1);
      }
  }

  KeyRemapper remapper {
    .mapping = mapping,
    .axisConfigurations = axisConfigurations,
    .inputFns = inputFns,
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
