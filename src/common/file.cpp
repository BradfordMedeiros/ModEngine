#include "./files.h"

namespace realfiles {

std::string doLoadFile(std::string filepath){
  modlog("load file", filepath);
   std::ifstream file(filepath.c_str());
   if (!file.good()){
     throw std::runtime_error("file not found" + filepath);
   }   
   std::stringstream buffer;
   buffer << file.rdbuf();
   return buffer.str();
}

void saveFile(std::string filepath, std::string content){
  std::ofstream file;
  file.open(filepath);
  file << content;
  file.close();
}

void rmFile(std::string filepath){
  std::remove(filepath.c_str());
}

bool fileExists(std::string path){
  std::ifstream infile(path);
  return infile.good();
}

std::vector<std::string> listAllFiles(std::filesystem::path path) {
  std::vector<std::string> files;
  for(auto &file: std::filesystem::recursive_directory_iterator(path)) {
    if (!std::filesystem::is_directory(file)) {
      files.push_back(file.path());
    }
  }
  return files;
}

std::vector<std::string> listFilesAndDir(std::filesystem::path path){
  std::vector<std::string> files;
  for (const auto& entry : std::filesystem::directory_iterator(path)) {
    files.push_back(entry.path().filename());
  }
  return files;
}

std::vector<std::string> listFilesWithExtensions(std::string folder, std::vector<std::string> extensions){
  std::vector<std::string> models;
  for (auto file : listAllFiles(folder)){
    bool isValidExtension = isExtensionType(file, extensions);
    if (isValidExtension){
      models.push_back(file);
    }
  }
  return models;
}

}