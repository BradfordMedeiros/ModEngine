#ifndef MOD_MODLAYER
#define MOD_MODLAYER

#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
#include <assert.h>
#include "./common/util.h"

void installMod(std::string layer);
void uninstallMod(std::string layer);
std::vector<std::string> listMods();
std::string modlayerReadFile(std::string file);

#endif