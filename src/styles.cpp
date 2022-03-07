#include "./styles.h"

StyleSelectorType getStyleSelectorType(std::string& target){
  auto firstElement = target.at(0);
  if (firstElement == ';'){
    return STYLE_SELECTOR_NAME;
  }
  if (firstElement == '.'){
    return STYLE_SELECTOR_ATTRIBUTE;
  }
  if (firstElement == '.'){
    return STYLE_SELECTOR_PAYLOAD;
  }
  std::cout << "get style selector - invalid selector: " << firstElement << std::endl;
  assert(false);
  return STYLE_SELECTOR_NAME; 
}
std::string getStyleSelectorTarget(StyleSelectorType& type, std::string target){
  if (type == STYLE_SELECTOR_NAME){
    return target;
  }
  if (type == STYLE_SELECTOR_ATTRIBUTE){
    return target;
  }
  return target;
}

std::vector<Style> loadStyles(std::string& filepath){
  auto tokens = parseFormat(filepath);

  std::vector<Style> styles;
  for (auto &token : tokens){
    auto type = getStyleSelectorType(token.target);
    auto target = getStyleSelectorTarget(type, token.target.substr(1, token.target.size()));
    styles.push_back(Style{
      .type = type,
      .target = target,
      .attribute = token.attribute,
      .payload = token.payload,
    });
  }

  return styles;
}