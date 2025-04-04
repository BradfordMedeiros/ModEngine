#include "./modlayer.h"

std::string readFileOrPackage(std::string filepath);

std::vector<std::string> installedMods = {};  // static-state 
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
	auto resourcePath = std::filesystem::absolute(std::filesystem::path("res"));
	auto filepath = std::filesystem::absolute(std::filesystem::path(file)).lexically_normal();
	auto relativePath = filepath.lexically_relative(resourcePath).string(); // file relative to the res folder
  std::filesystem::path modpath = std::filesystem::canonical(mod);    
  return std::filesystem::weakly_canonical(modpath / relativePath).string(); 						
}

std::string modlayerPath(std::string file){
	for (int i = installedMods.size() - 1; i >= 0; i--){
		auto modpathRoot = installedMods.at(i);
		auto fullModpath = joinModPath(modpathRoot, file);
		std::cout << "modlayer : modpath = " << modpathRoot << " file = " << file <<  " fullmodpath = " << fullModpath << std::endl;
		if (fileExists(fullModpath)){
			return fullModpath;
		}
	}	
	return file;
}

bool modlayerFileExists(std::string file){
	auto filepath = modlayerPath(file);
	return fileExists(filepath);
}

std::string modlayerReadFile(std::string file){
	return readFileOrPackage(modlayerPath(file));
}