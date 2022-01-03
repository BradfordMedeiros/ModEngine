#ifndef MOD_SCENE_OFFLINE
#define MOD_SCENE_OFFLINE

#include <string>
#include <iostream>
#include <vector>
#include "assert.h"
#include "../common/util.h"
#include "./serialization.h"

void offlineNewScene(std::string scenepath);
void offlineDeleteScene(std::string scenepath);
bool offlineSceneExists(std::string scenepath);
bool offlineSceneExists(std::string scenepath);
void offlineCopyScene(std::string scenepath, std::string newScenepath);
void offlineRemoveElement(std::string scenepath, std::string elementName);
void offlineSetElementAttributes(std::string scenepath, std::string elementName, std::vector<std::pair<std::string, std::string>> attrs);
std::vector<Token> offlineGetElement(std::string scenepath, std::string elementName);
std::vector<std::string> offlineGetElements(std::string scenepath);

#endif