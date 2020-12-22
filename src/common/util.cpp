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
void saveFile(std::string filepath, std::string content){
  std::ofstream file;
  file.open(filepath);
  file << content;
  file.close();
}
void rmFile(std::string filepath){    // maybe should guard to not be above the executable path?
  std::remove(filepath.c_str());
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

std::vector<std::string> filterComments(std::vector<std::string> values){
  std::vector<std::string> newStrings;
  for (auto value : values){
    auto parts = split(value, '#');
    if (parts.size() > 0){
      newStrings.push_back(parts.at(0));
    }
  }
  return newStrings;
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
glm::vec2 parseVec2(std::string positionRaw){;
  float x, y;
  std::istringstream in(positionRaw);
  in >> x >> y;
  return glm::vec2(x, y);
}
std::vector<float> parseFloatVec(std::string value){
  std::vector<float> floats;
  auto values = filterWhitespace(split(value, ' '));
  for (auto value : values){
    floats.push_back(std::atof(value.c_str()));
  }
  return floats;
}

glm::quat eulerToQuat(glm::vec3 eulerAngles){
  return glm::quat(glm::vec3(eulerAngles.x, eulerAngles.y, (eulerAngles.z + M_PI)));
}
glm::quat parseQuat(std::string payload){
  return eulerToQuat(parseVec(payload));
}
glm::vec3 quatToVec(glm::quat quat){
  return quat * glm::vec3(0.f, 0.f, -1.f);    // rotate the forward direction by the quat. 
}
std::string serializeVec(glm::vec3 vec){
  return std::to_string(vec.x) + " " + std::to_string(vec.y) + " " + std::to_string(vec.z);
}
std::string serializeVec(glm::vec2 vec){
  return std::to_string(vec.x) + " " + std::to_string(vec.y);
}
std::string serializeRotation(glm::quat rotation){
  glm::vec3 angles = eulerAngles(rotation);
  return std::to_string(angles.x) + " " + std::to_string(angles.y) + " " + std::to_string(angles.z - M_PI); 
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

// Chance of collision for id is 
// (n^2) / (2^k) 
// n = number of objects
// k = number of bits

// Generates a uuid of size bits(objid) - 1 (doesn't use the highest bit). 
// Probably not very random

// Currently 
// (n ^ 2) / (2 ^16)
// n^2 / 65536

// WARNING: this will probably collide, but let's actually hit it and accept the int32_t for now.
// 10000 objects => 15% chance of collision
static bool seeded = false;
objid getUniqueObjId(){   
  if (!seeded){
    srand(time(NULL));
    seeded = true;
  }

  int numBits = sizeof(objid) * 8;
  objid randId = 0;

  for (int i = 0; i < (numBits - 1); i++){   // -1 so always produces positive value.  Wastes 1 bit of space for unsigned values
    auto randomValue = rand() % 2; 
    bool bitHigh = randomValue > 0;
    randId =  randId | (bitHigh << i);
  }

  if (randId == -1){              // TODO - hack, -1 used as sentinel, need to eliminate those and then can get rid of this (scenegraph mostly)
    return getUniqueObjId();
  }
  return randId;
}

std::string getUniqueObjectName(){
  return std::string("name(") + std::to_string(getUniqueObjId()) + ")";
}