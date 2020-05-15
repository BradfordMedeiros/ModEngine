#include "./util.h"

std::string loadFile(std::string filepath){
   std::ifstream file(filepath.c_str());
   if (!file.good()){
     throw std::runtime_error("file not found" + filepath);
   }   
   std::stringstream buffer;
   buffer << file.rdbuf();
   return buffer.str();
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
std::vector<std::string> listFilesWithExtensions(std::string folder, std::vector<std::string> extensions){
  std::vector<std::string> models;
  for (auto file : listAllFiles(folder)){
    auto parts = split(file, '.');
    if (parts.size() >= 2){
      auto extension = parts.at(parts.size() - 1);

      bool isValidExtension = false;
      for (auto knownExtension : extensions){
        if (extension == knownExtension){
          isValidExtension = true;
          break;
        }
      }
      if (isValidExtension){
        models.push_back(file);
      }
    }
  }
  return models;
}


std::string trim(const std::string& str){
  size_t first = str.find_first_not_of(' ');
  if (std::string::npos == first){
    return str;
  }
  size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}

std::vector<std::string> split(std::string strToSplit, char delimeter){
  std::stringstream ss(strToSplit);
  std::string item;
  std::vector<std::string> splittedStrings;
  while (std::getline(ss, item, delimeter)){
    splittedStrings.push_back(item);
  }
  return splittedStrings;
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

std::string print(glm::vec3 vec){
  std::stringstream stream;
  stream << vec.x << " " << vec.y << " " << vec.z;
  return stream.str();
}
std::string print(glm::vec2 vec){
  std::stringstream stream;
  stream << vec.x << " " << vec.y;
  return stream.str();
}
std::string print(glm::quat quat){
  std::stringstream stream;
  stream << quat.x << " " << quat.y << " " << quat.z;
  return stream.str();
}
glm::vec3 parseVec(std::string positionRaw){;
  float x, y, z;
  std::istringstream in(positionRaw);
  in >> x >> y >> z;
  return glm::vec3(x, y, z);
}
glm::quat parseQuat(std::string payload){
  glm::vec3 eulerAngles = parseVec(payload);
  glm::quat rotation = glm::quat(glm::vec3(eulerAngles.x + 0, eulerAngles.y + 0, (eulerAngles.z + M_PI)));
  return rotation;
}
glm::vec3 quatToVec(glm::quat quat){
  return quat * glm::vec3(0.f, 0.f, -1.f);    // rotate the forward direction by the quat. 
}

float maxvalue(float x, float y, float z){
  if (x >= y && x >= z){
    return x;
  }
  if (y >= x && y >= z){
    return y;
  }
  return z;
}

int maxvalue(int x, int y, int z){
  if (x >= y && x >= z){
    return x;
  }
  if (y >= x && y >= z){
    return y;
  }
  return z;
}

