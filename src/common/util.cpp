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
void appendFile(std::string filepath, std::string content){
  std::cout << "appending: " << filepath << " w/ " << content << std::endl;
  std::fstream file(filepath.c_str(), std::ios::out | std::ios::app);
  file << content;
  file.close();
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

std::optional<std::string> getExtension(std::string file){
  auto parts = split(file, '.');
  if (parts.size() >= 2){
    return parts.at(parts.size() - 1);  
  }
  return std::nullopt;
}


std::vector<std::string> listFilesWithExtensions(std::string folder, std::vector<std::string> extensions){
  std::vector<std::string> models;
  for (auto file : listAllFiles(folder)){
    auto extensionData = getExtension(file);
    if (extensionData.has_value()){
      auto extension = extensionData.value();
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

std::vector<std::string> imageExtensions = { "png", "jpg", "jpeg", "tga" };
std::vector<std::string> audioExtensions = { "wav", "mp3" };
std::vector<std::string> videoExtensions = { "mkv", "avi", "webm" };
std::vector<std::string> modelExtensions = { "fbx", "dae", "obj" };

FILE_EXTENSION_TYPE getFileType(std::string filepath){
  auto extensionData = getExtension(filepath);
  if(extensionData.has_value()){
    auto extension = extensionData.value();
    if (std::find(imageExtensions.begin(), imageExtensions.end(), extension) != imageExtensions.end()){
      return IMAGE_EXTENSION;
    }
    if (std::find(audioExtensions.begin(), audioExtensions.end(), extension) != audioExtensions.end()){
      return AUDIO_EXTENSION;
    } 
    if (std::find(videoExtensions.begin(), videoExtensions.end(), extension) != videoExtensions.end()){
      return VIDEO_EXTENSION;
    } 
    if (std::find(modelExtensions.begin(), modelExtensions.end(), extension) != modelExtensions.end()){
      return MODEL_EXTENSION;
    } 
  }
  return UNKNOWN_EXTENSION;
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

AttributeValue interpolateAttribute(AttributeValue key1, AttributeValue key2, float percentage){  
  assert(percentage <= 1.f && percentage >= 0.f);
  auto attr1 = std::get_if<glm::vec3>(&key1);
  if (attr1 != NULL){
    auto attr2 = std::get_if<glm::vec3>(&key2);
    assert(attr2 != NULL);

    std::cout << "percentage: " << percentage << std::endl;
    std::cout << "key1: " << print(*attr1) << std::endl;
    std::cout << "key2: " << print(*attr2) << std::endl;
    return glm::vec3(
      (attr1 -> x * (1 - percentage)) + (attr2 -> x * percentage), 
      (attr1 -> y * (1 - percentage)) + (attr2 -> y * percentage), 
      (attr1 -> z * (1 - percentage)) + (attr2 -> z * percentage)
    );
  }
  auto attr2 = std::get_if<float>(&key1);
  if (attr2 != NULL){
    auto attr2 = std::get_if<float>(&key2);
    assert(attr2 != NULL);
    return *attr1 + *attr2;
  }
  assert(false);
  return key1;
}

std::string serializePropertySuffix(std::string key, AttributeValue value){
  auto prefix = key + ":";
  auto vecValue = std::get_if<glm::vec3>(&value);
  if (vecValue != NULL){
    return prefix + serializeVec(*vecValue);
  }
  auto floatValue = std::get_if<float>(&value);
  if (floatValue != NULL){
    return prefix + std::to_string(*floatValue);
  }

  auto stringValue = std::get_if<std::string>(&value);
  if (stringValue != NULL){
    return prefix + *stringValue;
  }
  assert(false);
  return prefix;
}