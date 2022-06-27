#include "./modlayer.h"

std::vector<std::string> installedMods = {};
void installMod(std::string layer){
	assert(std::count(installedMods.begin(), installedMods.end(), layer) == 0);
	installedMods.push_back(layer);
	std::cout << "install mod: " << layer << std::endl;
}

void uninstallMod(std::string layer){
	assert(std::count(installedMods.begin(), installedMods.end(), layer) == 1);
	std::vector<std::string> newInstalledMods = {};
	for (auto mod : installedMods){
		if (mod != layer){
			newInstalledMods.push_back(mod);
		}
	}
	installedMods = newInstalledMods;
	std::cout << "uninstall mod: " << layer << std::endl;
}

std::vector<std::string> listMods(){
	return installedMods;
}