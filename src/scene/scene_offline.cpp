#include "./scene_offline.h"

void offlineNewScene(std::string scenepath){
  std::cout << "offline: create new scene: " << scenepath << std::endl;
  saveFile(scenepath, "");
}
void offlineDeleteScene(std::string scenepath){
  std::cout << "offline: delete new scene: " << scenepath << std::endl;
  std::remove(scenepath.c_str());
}
bool offlineSceneExists(std::string scenepath){
  return fileExists(scenepath);
}
void offlineCopyScene(std::string scenepath, std::string newScenepath){
  std::cout << "offline: copy scene: " << scenepath << " in file: " << newScenepath << std::endl;
  saveFile(newScenepath, loadFile(scenepath));
}

void offlineRemoveElement(std::string scenepath, std::string elementName){
  std::cout << "offline: remove element: " << elementName << " in file: " << scenepath << std::endl;
  auto tokens = parseFormat(loadFile(scenepath));
  std::vector<Token> newTokens;
  for (auto token : tokens){
    if (token.target == elementName){
      continue;
    }
    newTokens.push_back(token);
  }
  saveFile(scenepath, serializeSceneTokens(newTokens));
}

void offlineSetElementAttributes(std::string scenepath, std::string elementName, std::vector<std::pair<std::string, std::string>> attrs){
  std::cout << "offline: set element attributes: " << elementName << " in file: " << scenepath << " - ";
  auto tokens = parseFormat(loadFile(scenepath));
  std::vector<Token> newTokens;
  for (auto token : tokens){
    if (token.target == elementName){
      continue;
    }
    newTokens.push_back(token);
  }

  for (auto &[key, val] : attrs){
    newTokens.push_back(Token{
      .target = elementName,
      .attribute = key,
      .payload = val,
    });
  }
  saveFile(scenepath, serializeSceneTokens(newTokens));
}

// like set element but preserves the old attributes
void offlineUpdateElementAttributes(std::string scenepath, std::string elementName, std::vector<std::pair<std::string, std::string>> attrs){
  std::cout << "offline: update element attributes: " << elementName << " in file: " << scenepath << " - ";
  auto tokens = parseFormat(loadFile(scenepath));
  std::vector<Token> newTokens;

  std::map<std::string, std::string> elementTokens;
  for (auto token : tokens){
    if (token.target == elementName){
      elementTokens[token.attribute] = token.payload;
      continue;
    }
    newTokens.push_back(token);
  }
  for (auto attr : attrs){
    elementTokens[attr.first] = attr.second;
  }

  for (auto &[key, val] : elementTokens){
    newTokens.push_back(Token{
      .target = elementName,
      .attribute = key,
      .payload = val,
    });
  }
  saveFile(scenepath, serializeSceneTokens(newTokens));
}

std::vector<Token> offlineGetElement(std::string scenepath, std::string elementName){
  auto tokens = parseFormat(loadFile(scenepath));
  std::vector<Token> element;
  for (auto token : tokens){
    if (token.target == elementName){
      element.push_back(token);
    }
  }
  return element;
}

std::vector<std::string> offlineGetElements(std::string scenepath){
  std::vector<std::string> elements;
  auto tokens = parseFormat(loadFile(scenepath));
  for (auto &token : tokens){
    if (std::find(elements.begin(), elements.end(), token.target) == elements.end()){
      elements.push_back(token.target);
    }
  }
  return elements;
}

std::vector<std::string> offlineGetElementsNoChildren(std::string scenepath){
  auto tokens = parseFormat(loadFile(scenepath));
  auto elementsToAttrs = deserializeSceneTokens(tokens);
  std::set<std::string> allChildren;
  for (auto &[_, elementToAttr] : elementsToAttrs){
    for (auto child : elementToAttr.children){
      allChildren.insert(child);
    }
  }
  std::vector<std::string> elements;
  for (auto &[elementName, _] : elementsToAttrs){
    if (allChildren.find(elementName) == allChildren.end()){
      elements.push_back(elementName);
    }
  }
  return elements;
}

void offlineMoveElement(std::string fromScene, std::string toScene, std::string elementName){
  auto elements = offlineGetElement(fromScene, elementName);
  offlineRemoveElement(fromScene, elementName);
  std::vector<std::pair<std::string, std::string>> attrs;
  for (auto element : elements){
    attrs.push_back({element.attribute, element.payload});
  }
  if (attrs.size() == 0){
    std::cout << "element " << elementName << " not found in " << fromScene << std::endl;
   // assert(false);
  }
  offlineSetElementAttributes(toScene, elementName, attrs);
}

std::vector<std::string> offlineNodeAndChildren(std::string fromScene, std::string elementName){
  auto tokens = parseFormat(loadFile(fromScene));
  auto elementsToAttrs = deserializeSceneTokens(tokens);

  std::vector<std::string> allChildren;
  std::queue<std::string> elements;
  elements.push(elementName);
  if (elementsToAttrs.find(elementName) == elementsToAttrs.end()){
    return {};
  }

  while (!elements.empty()){
    auto nextElement = elements.front();
    allChildren.push_back(nextElement);
    elements.pop();

    auto nextElementChildren = elementsToAttrs.at(nextElement).children;
    for (auto element : nextElementChildren){
      elements.push(element);
    }
  }
  return allChildren;
}
void offlineMoveElementAndChildren(std::string fromScene, std::string toScene, std::string elementName){
  auto children = offlineNodeAndChildren(fromScene, elementName);
  for (auto child : children){
    offlineMoveElement(fromScene, toScene, child);
  }
}
