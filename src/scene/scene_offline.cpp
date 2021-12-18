#include "./scene_offline.h"

void offlineNewScene(std::string scenepath){
  std::cout << "offline: create new scene: " << scenepath << std::endl;
  saveFile(scenepath, "");
}
void offlineDeleteScene(std::string scenepath){
  std::cout << "offline: delete new scene: " << scenepath << std::endl;
  std::remove(scenepath.c_str());
}
void offlineCopyScene(std::string scenepath, std::string newScenepath){
  std::cout << "offline: copy scene: " << scenepath << " from: " << newScenepath << std::endl;
  saveFile(newScenepath, loadFile(scenepath));
}

void offlineRemoveElement(std::string scenepath, std::string elementName){
  std::cout << "offline: remove element: " << elementName << " from: " << scenepath << std::endl;
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