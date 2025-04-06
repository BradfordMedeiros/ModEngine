#ifndef MOD_FILES
#define MOD_FILES

#include <string>
#include <filesystem>
#include "./util.h"

namespace realfiles {
	std::string doLoadFile(std::string filepath);
	void saveFile(std::string filepath, std::string content);
	void rmFile(std::string filepath);
	bool fileExists(std::string path);
	std::optional<std::string> getPreExtension(std::string file);
	std::vector<std::string> listFilesWithExtensions(std::string folder, std::vector<std::string> extensions);
	std::vector<std::string> listFilesAndDir(std::filesystem::path path);
	std::vector<std::string> listAllFiles(std::filesystem::path path);
}



#endif 
