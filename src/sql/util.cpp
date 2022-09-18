#include "./util.h"

namespace sql { 
/// warning copy/paste part. Maybe should include common code for such little code...?  But might be not worth it...
std::vector<std::string> listAllCsvFilesStems(std::filesystem::path path) {
  std::vector<std::string> files;
  for(auto &file: std::filesystem::recursive_directory_iterator(path)) {
    if (!std::filesystem::is_directory(file)) {
      if (file.path().extension() == ".csv"){
        files.push_back(file.path().stem());
      }
    }
  }
  return files;
}

std::string join(std::vector<std::string> values, char delimeter){
  std::string value = "";
  for (int i = 0; i < values.size(); i++){
    value = value + values[i];
    if (i < (values.size() - 1)){
      value = value + delimeter;
    }
  }
  return value;
}
void saveFile(std::string filepath, std::string content){
  std::ofstream file;
  file.open(filepath);
  file << content;
  file.close();
}
std::string loadFile(std::string filepath){
   std::ifstream file(filepath.c_str());
   if (!file.good()){
     throw std::runtime_error("file not found" + filepath);
   }   
   std::stringstream buffer;
   buffer << file.rdbuf();
   return buffer.str();
}
std::vector<std::string> split(std::string strToSplit, char delimeter){
  std::vector<std::string> splittedStrings;
  int lowIndex = 0;
  for (int i = 0; i < strToSplit.size(); i++){
    if (strToSplit.at(i) == delimeter){
      auto stringlength = i - lowIndex;
      auto token = strToSplit.substr(lowIndex, stringlength);
      lowIndex = i + 1;
      splittedStrings.push_back(token);
    }
  }
  if (lowIndex != strToSplit.size()){
    auto token = strToSplit.substr(lowIndex, strToSplit.size() - lowIndex);
    splittedStrings.push_back(token);
  }
  if (strToSplit.size() > 0 && strToSplit.at(strToSplit.size() - 1) == delimeter){
    splittedStrings.push_back("");
  }
  return splittedStrings;
}
std::string trim(const std::string& str){
  size_t first = str.find_first_not_of(' ');
  if (std::string::npos == first){
    return str;
  }
  size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}

std::vector<std::string> filterWhitespace(std::vector<std::string> values){
  std::vector<std::string> newStrings;
  for (auto value : values){
    auto newValue = trim(value);
    if (newValue != ""){
      newStrings.push_back(newValue);
    }
  }
  return newStrings;
}
void appendFile(std::string filepath, std::string content){
  std::cout << "appending: " << filepath << " w/ " << content << std::endl;
  std::fstream file(filepath.c_str(), std::ios::out | std::ios::app);
  file << content;
  file.close();
}
/////////////////////

}