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

std::vector<Style> loadStyles(std::string filepath){
  auto tokens = parseFormat(loadFile(filepath));
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

std::set<std::string> matchingElementsNames(std::vector<Token>& tokens, Style& style){
  std::set<std::string> matchingElements;
  if (style.type != STYLE_SELECTOR_NAME){
    std::cout << "matching element name must be name" << std::endl;
    assert(false);
  }
  for (auto &token : tokens){
    if (token.target == style.target){
      matchingElements.insert(token.target);
    }
  }
  return matchingElements;
}
int matchingTokenForAttribute(std::vector<Token>& tokens, std::string attribute){
  for (int i = 0; i < tokens.size(); i++){
    Token& token = tokens.at(i);
    if (token.attribute == attribute){
      return i;
    }
  }
  return -1;
}

void applyStyles(std::vector<Token>& tokens, std::vector<Style>& styles){
  std::vector<Token> additionalTokens;
  for (auto &style : styles){
    auto matchingElements = matchingElementsNames(tokens, style);
    for (auto &element : matchingElements){
      auto tokenIndex = matchingTokenForAttribute(tokens, style.attribute);
      if (tokenIndex != -1){
        tokens.at(tokenIndex).payload = style.payload;
        continue;
      }
      additionalTokens.push_back(Token{
        .target = element, 
        .attribute = style.attribute,
        .payload = style.payload,
      });
    }
  }
  for (auto &additionalToken : additionalTokens){
    tokens.push_back(additionalToken);
  }
  // if STYLE_SELECTOR_NAME
  // determine all elements that match
  // if element
  // then get token index that has the attribute if it exists and modify that token attribute payload if exists
  // else append a new token for the element with that attribute payload pair


}