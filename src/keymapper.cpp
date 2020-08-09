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

  KeyRemapper remapper {
    .mapping = mapping,
  };
  return remapper;
}