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
  for (auto &[key, val] : attrs){
    std::cout << "(" << key << ", " << val << ") ";
  }
  std::cout << std::endl;

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
  if (attrs.size() == 0){
    newTokens.push_back(Token{
      .target = elementName,
      .attribute = "position", 
      .payload = "0 0 0",
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

void offlineMoveElement(std::string fromScene, std::string toScene, std::string elementName){
  auto elements = offlineGetElement(fromScene, elementName);
  offlineRemoveElement(fromScene, elementName);
  std::vector<std::pair<std::string, std::string>> attrs;
  for (auto element : elements){
    attrs.push_back({element.attribute, element.payload});
  }
  offlineSetElementAttributes(toScene, elementName, attrs);
}