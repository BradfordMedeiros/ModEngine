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

// eg file = ./res/scenes/example.p.rawscene, mod = ./res/modlayers
std::string joinModPath(std::string mod, std::string file){
	auto relativePath = std::filesystem::path(file).lexically_relative("./res").string(); // file relative to the res folder
  std::filesystem::path modpath = std::filesystem::canonical(mod);                      

  return std::filesystem::weakly_canonical(modpath / relativePath).string(); 						// then join to modpath
}

std::string modlayerPath(std::string file){
	for (int i = installedMods.size() - 1; i >= 0; i--){
		auto modpathRoot = installedMods.at(i);
		auto fullModpath = joinModPath(modpathRoot, file);
		if (fileExists(fullModpath)){
			return fullModpath;
		}
	}	
	return file;
}

std::string modlayerReadFile(std::string file){
	return loadFile(modlayerPath(file));
}