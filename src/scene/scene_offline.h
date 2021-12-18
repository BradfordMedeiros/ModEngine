#ifndef MOD_SCENE_OFFLINE
#define MOD_SCENE_OFFLINE

#include <string>
#include <iostream>
#include "assert.h"

void offlineNewScene(std::string scenepath);
void offlineDeleteScene(std::string scenepath);
void offlineCopyScene(std::string scenepath, std::string newScenepath);
void offlineRemoveElement(std::string scenepath, std::string elementName);

#endif