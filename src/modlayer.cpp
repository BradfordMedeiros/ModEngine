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


std::string pathForMod(std::string mod, std::string file){
	std::filesystem::path relativePath = std::filesystem::weakly_canonical(file); 
	return relativePath.string();
}
std::string modlayerReadFile(std::string file){
	for (int i = installedMods.size() - 1; i >= 0; i--){
		auto modpathRoot = installedMods.at(i);
		auto fullModpath = pathForMod(modpathRoot, file);
		std::cout << "modpath root: " << fullModpath << std::endl;
	}
	//(install-mod "./res/modlayers")
	// loop over each install modpath in least recent order (since newer ones overwrite older ones)
	// if file exists load from there, else load the prev and then base

	/*
	std::string loadFile(std::string filepath);
void saveFile(std::string filepath, std::string content);
bool fileExists(std::string path);
*/
	return loadFile(file);
}