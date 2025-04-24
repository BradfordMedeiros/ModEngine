#include "./scene_offline.h"

void offlineNewScene(std::string scenepath){
  std::cout << "offline: create new scene: " << scenepath << std::endl;
  realfiles::saveFile(scenepath, "");
}
void offlineDeleteScene(std::string scenepath){
  std::cout << "offline: delete new scene: " << scenepath << std::endl;
  std::remove(scenepath.c_str());
}
bool offlineSceneExists(std::string scenepath){
  return realfiles::fileExists(scenepath);
}
void offlineCopyScene(std::string scenepath, std::string newScenepath, std::function<std::string(std::string)> readFile){
  std::cout << "offline: copy scene: " << scenepath << " in file: " << newScenepath << std::endl;
  realfiles::saveFile(newScenepath, readFile(scenepath));
}

void offlineRemoveElement(std::string scenepath, std::string elementName, std::function<std::string(std::string)> readFile){
  std::cout << "offline: remove element: " << elementName << " in file: " << scenepath << std::endl;
  auto tokens = parseFormat(readFile(scenepath));
  std::vector<Token> newTokens;
  for (auto token : tokens){
    if (token.target == elementName){
      continue;
    }
    newTokens.push_back(token);
  }
  realfiles::saveFile(scenepath, serializeSceneTokens(newTokens));
}

void offlineSetElementAttributes(std::string scenepath, std::string elementName, std::vector<std::pair<std::string, std::string>> attrs, std::function<std::string(std::string)> readFile){
  std::cout << "offline: set element attributes: " << elementName << " in file: " << scenepath << " - ";
  auto tokens = parseFormat(readFile(scenepath));
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
  realfiles::saveFile(scenepath, serializeSceneTokens(newTokens));
}

// like set element but preserves the old attributes
void offlineUpdateElementAttributes(std::string scenepath, std::string elementName, std::vector<std::pair<std::string, std::string>> attrs, std::function<std::string(std::string)> readFile){
  std::cout << "offline: update element attributes: " << elementName << " in file: " << scenepath << " - ";
  auto tokens = parseFormat(readFile(scenepath));
  std::vector<Token> newTokens;

  std::unordered_map<std::string, std::string> elementTokens;
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
  realfiles::saveFile(scenepath, serializeSceneTokens(newTokens));
}

std::vector<Token> offlineGetElement(std::string scenepath, std::string elementName, std::function<std::string(std::string)> readFile){
  auto tokens = parseFormat(readFile(scenepath));
  std::vector<Token> element;
  for (auto token : tokens){
    if (token.target == elementName){
      element.push_back(token);
    }
  }
  return element;
}

std::string offlineGetElementAttr(std::string scenepath, std::string elementName, std::string attr, std::function<std::string(std::string)> readFile){
  auto tokens = offlineGetElement(scenepath, elementName, readFile);
  for (auto token : tokens){
    if (token.attribute == attr){
      return token.payload;
    }
  }
  assert(false);
  return "";
}

std::vector<std::string> offlineGetElements(std::string scenepath, std::function<std::string(std::string)> readFile){
  std::vector<std::string> elements;
  auto tokens = parseFormat(readFile(scenepath));
  for (auto &token : tokens){
    if (std::find(elements.begin(), elements.end(), token.target) == elements.end()){
      elements.push_back(token.target);
    }
  }
  return elements;
}

bool offlineElementExists(std::string scenepath, std::string elementName, std::function<std::string(std::string)> readFile){
  auto elements = offlineGetElement(scenepath, elementName, readFile);
  return elements.size() > 0;
}

std::vector<std::string> offlineGetElementsNoChildren(std::string scenepath, std::function<std::string(std::string)> readFile){
  auto tokens = parseFormat(readFile(scenepath));
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

void offlineMoveElement(std::string fromScene, std::string toScene, std::string elementName, std::function<std::string(std::string)> readFile, bool renameOnCollision){
  std::string targetElementName = elementName;
  if (offlineElementExists(toScene, elementName, readFile)){
    std::cout << "scene offline: " << elementName << " already exists in " << toScene << std::endl;
    if (!renameOnCollision){
      assert(false);
    }
    bool renamed = false;

    for(int indexNumber = 0; indexNumber < 100; indexNumber++){  // 100 arbitrary limit
      indexNumber++;
      std::string newElementName = elementName + "_" + std::to_string(indexNumber);
      if (!offlineElementExists(toScene, newElementName, readFile)){
        targetElementName = newElementName;
        renamed = true;
        std::cout << "renamed to elementName: " << targetElementName << std::endl;
        break;
      }
    }
    assert(renamed);
  }
  auto elements = offlineGetElement(fromScene, elementName, readFile);
  offlineRemoveElement(fromScene, elementName, readFile);
  std::vector<std::pair<std::string, std::string>> attrs;
  for (auto element : elements){
    attrs.push_back({element.attribute, element.payload});
  }
  if (attrs.size() == 0){
    std::cout << "element " << elementName << " not found in " << fromScene << std::endl;
   // assert(false);
  }
  offlineSetElementAttributes(toScene, targetElementName, attrs, readFile);
}

std::vector<std::string> offlineNodeAndChildren(std::string fromScene, std::string elementName, std::function<std::string(std::string)> readFile){
  auto tokens = parseFormat(readFile(fromScene));
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
void offlineMoveElementAndChildren(std::string fromScene, std::string toScene, std::string elementName, bool renameOnCollision, std::function<std::string(std::string)> readFile){
  auto children = offlineNodeAndChildren(fromScene, elementName, readFile);
  for (auto child : children){
    offlineMoveElement(fromScene, toScene, child, readFile, renameOnCollision);
  }
}

size_t offlineHashSceneContent(std::string scenepath1, std::function<std::string(std::string)> readFile){
  auto sceneOne = readFile(scenepath1);
  std::hash<std::string> hashStr;
  return hashStr(sceneOne);
}