#ifndef MOD_MODLAYER
#define MOD_MODLAYER

#include <vector>
#include <string>
#include <iostream>

void installMod(std::string layer);
void uninstallMod(std::string layer);
std::vector<std::string> listMods();

#endif