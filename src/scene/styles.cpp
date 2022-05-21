#include "./styles.h"

StyleSelectorType getStyleSelectorType(std::string& target){
  auto firstElement = target.at(0);
  if (firstElement == ';'){
    return STYLE_SELECTOR_NAME;
  }
  if (firstElement == '.'){
    return STYLE_SELECTOR_ATTRIBUTE;
  }
  if (firstElement == ','){
    return STYLE_SELECTOR_PAYLOAD;
  }
  std::cout << "get style selector - invalid selector: " << firstElement << std::endl;
  assert(false);
  return STYLE_SELECTOR_NAME; 
}

std::string selectorTypeToString(StyleSelectorType& type){
  if (type == STYLE_SELECTOR_NAME){
    return "STYLE_SELECTOR_NAME";
  }
  if (type == STYLE_SELECTOR_ATTRIBUTE){
    return "STYLE_SELECTOR_ATTRIBUTE";
  }
  if (type == STYLE_SELECTOR_PAYLOAD){
    return "STYLE_SELECTOR_PAYLOAD";
  }
  return "UNKNOWN TYPE - bug";
}

std::vector<StyleSelectorQuery> parseStyleSelectorQuery(std::string tokentarget){
  std::vector<StyleSelectorQuery> selectors;
  auto selectorQueries = splitNoWhitespace(tokentarget, ' ');
  for (auto selectorQuery : selectorQueries){
    auto type = getStyleSelectorType(selectorQuery);
    auto target = selectorQuery.substr(1, selectorQuery.size());
    std::cout << "selectorQuery is: " << selectorQuery << " ( " << selectorTypeToString(type) << " )" << " - " << target << std::endl;
    selectors.push_back(StyleSelectorQuery{
      .type = type,
      .target = target,
    });
  }
  return selectors;
}

std::vector<Style> loadStyles(std::string filepath){
  auto tokens = parseFormat(loadFile(filepath));
  std::vector<Style> styles;
  for (auto &token : tokens){
    styles.push_back(Style{
      .queries = parseStyleSelectorQuery(token.target), 
      .attribute = token.attribute,
      .payload = token.payload,
    });
  }
  return styles;
}


bool matchesSelectorQuery(Token& token, StyleSelectorQuery& query){
  if (query.type == STYLE_SELECTOR_NAME){
    if (token.target == query.target){
      return true;
    }
  }else if (query.type == STYLE_SELECTOR_ATTRIBUTE){
    if (token.attribute == query.target){
      return true;
    }
  }else if (query.type == STYLE_SELECTOR_PAYLOAD){
    if (token.payload == query.target){
      return true;
    }
  }else{
    std::cout << "invalid query type" << std::endl;
    assert(false);
  }
  return false;
}
bool matchesAllQueries(Token& token, std::vector<StyleSelectorQuery>& queries){
  for (auto &query : queries){
    bool matches = matchesSelectorQuery(token, query);
    if (!matches){
      return false;
    }
  }
  return true;
}

// any style selector for a subelement has be in the token list already, or by definition, a name selector (or false)
// a name selector with multiple values is false unless they're all teh subelement
std::string additionalMatchingSubelements(std::vector<Token>& tokens,  Style& style){
  if (style.queries.size() == 0){
    return "";
  }
  for (auto &query : style.queries){
    if (query.type != STYLE_SELECTOR_NAME){
      return "";
    }
  }
  for (int i = 0; i < style.queries.size(); i++){
    for (int j = 1; j < style.queries.size(); j++){
      if (i == j){ continue; }
      if (style.queries.at(i).target != style.queries.at(j).target){
        return "";
      }
    }
  }

  auto query = style.queries.at(0);

  bool foundParentTarget = false;
  for (auto &token : tokens){
    if (token.target == mainTargetElement(query.target)){
      foundParentTarget = true;
    }
  }

  // make sure all queries have same target
  if (!foundParentTarget){
    return "";
  }
  return style.queries.at(0).target;
}


std::set<std::string> matchingElementsNames(std::vector<Token>& tokens, Style& style){
  std::set<std::string> matchingElements;
  for (auto &token : tokens){
    if (matchesAllQueries(token, style.queries)){
      matchingElements.insert(token.target);
    }
  }
  return matchingElements;
}
int matchingTokenForAttribute(std::vector<Token>& tokens, std::string target, std::string attribute){
  for (int i = 0; i < tokens.size(); i++){
    Token& token = tokens.at(i);
    if (token.target == target && token.attribute == attribute){
      return i;
    }
  }
  return -1;
}

struct StyleNewPayloadForIndex {
  int index;
  std::string newPayload;
};  
void applyStyles(std::vector<Token>& tokens, std::vector<Style>& styles){
  std::vector<Token> additionalTokens;
  std::vector<StyleNewPayloadForIndex> newPayloads;
  for (auto &style : styles){
    auto matchingElements = matchingElementsNames(tokens, style);
    auto additionalSubelement = additionalMatchingSubelements(tokens, style);
    if (additionalSubelement != ""){
      matchingElements.insert(additionalSubelement);
    }

    for (auto &element : matchingElements){
      auto tokenIndex = matchingTokenForAttribute(tokens, element, style.attribute);
      if (tokenIndex != -1){
        newPayloads.push_back(StyleNewPayloadForIndex{
          .index = tokenIndex,
          .newPayload = style.payload,
        });
        continue;
      }
      additionalTokens.push_back(Token{
        .target = element, 
        .attribute = style.attribute,
        .payload = style.payload,
      });
    }
  }
  for (auto &newPayload : newPayloads){
    tokens.at(newPayload.index).payload = newPayload.newPayload;
  }
  for (auto &additionalToken : additionalTokens){
    tokens.push_back(additionalToken);
  }
}