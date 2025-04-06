#ifndef MOD_SCENE_OFFLINE
#define MOD_SCENE_OFFLINE

#include <string>
#include <iostream>
#include <vector>
#include <queue>
#include "assert.h"
#include "../common/util.h"
#include "../common/files.h"
#include "./serialization.h"

void offlineNewScene(std::string scenepath);
void offlineDeleteScene(std::string scenepath);
bool offlineSceneExists(std::string scenepath);
bool offlineSceneExists(std::string scenepath);
void offlineCopyScene(std::string scenepath, std::string newScenepath, std::function<std::string(std::string)> readFile);
void offlineRemoveElement(std::string scenepath, std::string elementName, std::function<std::string(std::string)> readFile);
void offlineSetElementAttributes(std::string scenepath, std::string elementName, std::vector<std::pair<std::string, std::string>> attrs, std::function<std::string(std::string)> readFile);
void offlineUpdateElementAttributes(std::string scenepath, std::string elementName, std::vector<std::pair<std::string, std::string>> attrs, std::function<std::string(std::string)> readFile);
std::vector<Token> offlineGetElement(std::string scenepath, std::string elementName, std::function<std::string(std::string)> readFile);
std::string offlineGetElementAttr(std::string scenepath, std::string elementName, std::string attr, std::function<std::string(std::string)> readFile);
std::vector<std::string> offlineGetElements(std::string scenepath, std::function<std::string(std::string)> readFile);
std::vector<std::string> offlineGetElementsNoChildren(std::string scenepath, std::function<std::string(std::string)> readFile);
void offlineMoveElement(std::string fromScene, std::string toScene, std::string elementName, std::function<std::string(std::string)> readFile, bool renameOnCollision = false);
void offlineMoveElementAndChildren(std::string fromScene, std::string toScene, std::string elementName, bool renameOnCollision, std::function<std::string(std::string)> readFile);
size_t offlineHashSceneContent(std::string scenepath1, std::function<std::string(std::string)> readFile);

#endif