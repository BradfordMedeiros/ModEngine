#include "./util.h"

// Base IO fns ////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////


std::optional<std::string> getExtension(std::string file){
  auto parts = split(file, '.');
  if (parts.size() >= 2){
    return parts.at(parts.size() - 1);  
  }
  return std::nullopt;
}

std::optional<std::string> getPreExtension(std::string file){
  auto parts = split(file, '.');
  if (parts.size() >= 3){
    return parts.at(parts.size() - 2);  
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
std::vector<std::string> modelExtensions = { "fbx", "dae", "obj", "gltf" };

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
    if (std::find(modelExtensions.begin(), modelExtensions.end(), extension) != modelExtensions.end()){
      return MODEL_EXTENSION;
    } 
  }
  return UNKNOWN_EXTENSION;
}


bool stringContains(std::string& str, char character){
  size_t first = str.find_first_not_of(' ');
  return std::string::npos != first;
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
std::pair<std::string, std::string> carAndRest(std::string& strToSplit, char delimeter){
  auto splitValues = split(strToSplit, delimeter);
  auto rest = subvector(splitValues, 1, splitValues.size());
  return { splitValues.at(0), join(rest, delimeter) };
}

std::vector<std::string> subvector(std::vector<std::string>& values, int indexLow, int indexHigh){
  std::vector<std::string> subvec;
  for (int i = indexLow; i < indexHigh; i++){
    subvec.push_back(values.at(i));
  }
  return subvec;
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

std::vector<std::string> splitNoWhitespace(std::string string, char character){
  return filterWhitespace(split(string, character));
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

std::string print(bool value){
  return value ? "true" : "false";
}
std::string print(glm::vec3 vec){
  std::stringstream stream;
  stream << vec.x << " " << vec.y << " " << vec.z;
  return stream.str();
}
std::string print(glm::vec4 vec){
  std::stringstream stream;
  stream << vec.x << " " << vec.y << " " << vec.z << " " << vec.w;
  return stream.str();
}
std::string print(glm::ivec3 vec){
  return print(glm::vec3(vec.x, vec.y, vec.z));
}
std::string print(glm::vec2 vec){
  std::stringstream stream;
  stream << vec.x << " " << vec.y;
  return stream.str();
}

std::string print(std::vector<std::string>& values){
  std::stringstream stream;
  stream << "[ ";
  for (auto &value : values){
    stream << value << " ";
  }
  stream << "]";
  return stream.str();
}

std::string print(std::vector<objid>& values){
  std::stringstream stream;
  stream << "[ ";
  for (auto &value : values){
    stream << value << " ";
  }
  stream << "]";
  return stream.str();
}
std::string print(std::vector<bool>& values){
  std::string strValue = "[";
  for (auto value : values){
    strValue += " " + print(value);
  }
  strValue += " ]";
  return strValue;
}

std::string print(std::set<objid>& values){
  std::string strValue = "[";
  for (auto value : values){
    strValue += " " + std::to_string(value);
  }
  strValue += " ]";
  return strValue;
}

std::string rawprint(glm::quat quat){
  std::stringstream stream;
  stream << quat.x << " " << quat.y << " " << quat.z << " " << quat.w;
  return stream.str();
}
std::string print(glm::quat quat){
  glm::vec3 radianAngles = glm::eulerAngles(quat);
  auto degreeX = glm::degrees(radianAngles.x);
  auto degreeY = glm::degrees(radianAngles.y);
  auto degreeZ = glm::degrees(radianAngles.z);
  auto x = degreeX >= 0 ? degreeX : degreeX + 360;
  auto y = degreeY >= 0 ? degreeY : degreeY + 360;
  auto z = degreeZ >= 0 ? degreeZ : degreeZ + 360;
  std::stringstream stream;
  stream << x << " " << y << " " << z;
  return stream.str();
}
std::string print(glm::mat4 mat){
  return "[" + 
    std::to_string(mat[0][0]) + ", " + std::to_string(mat[1][0]) + ", " + std::to_string(mat[2][0]) + ", " + std::to_string(mat[3][0]) + "; " +
    std::to_string(mat[0][1]) + ", " + std::to_string(mat[1][1]) + ", " + std::to_string(mat[2][1]) + ", " + std::to_string(mat[3][1]) + "; " +
    std::to_string(mat[0][2]) + ", " + std::to_string(mat[1][2]) + ", " + std::to_string(mat[2][2]) + ", " + std::to_string(mat[3][2]) + "; " +
    std::to_string(mat[0][3]) + ", " + std::to_string(mat[1][3]) + ", " + std::to_string(mat[2][3]) + ", " + std::to_string(mat[3][3]) + 
  "]";
}
std::string print(glm::mat3 mat){
  return "[" + 
    std::to_string(mat[0][0]) + ", " + std::to_string(mat[1][0]) + ", " + std::to_string(mat[2][0]) + "; " +
    std::to_string(mat[0][1]) + ", " + std::to_string(mat[1][1]) + ", " + std::to_string(mat[2][1]) + "; " +
    std::to_string(mat[0][2]) + ", " + std::to_string(mat[1][2]) + ", " + std::to_string(mat[2][2]) + "; " +
  "]";
}
std::string print(glm::mat2 mat){
  return "[" + 
    std::to_string(mat[0][0]) + ", " + std::to_string(mat[1][0]) + "; " +
    std::to_string(mat[0][1]) + ", " + std::to_string(mat[1][1]) + "; " +
  "]";
}

std::string print(std::set<objid> values){
  std::stringstream stream;
  stream << "[ ";
  for (auto &value : values){
    stream << value << " ";
  }
  stream << "]";
  return stream.str();
}

std::string print(std::optional<std::string> value){
  if (!value.has_value()){
    return "[no value]";
  }
  return value.value();
}

bool maybeParseFloat(std::string value, float& _number){
  try {
    std::string::size_type parsedSize;
    float number = std::stof(value, &parsedSize);
    if (parsedSize != value.size()){
      return false;
    }
    _number = number;
    return true;
  }catch(...){}
  return false;
}

float parseFloat(std::string value){
  float number = 0.f;
  bool valid = maybeParseFloat(value, number);
  modassert(valid, "invalid float number: " + value);
  return number;
}

glm::vec3 parseVec(std::string positionRaw){;
  float x, y, z;
  std::istringstream in(positionRaw);
  in >> x >> y >> z;
  return glm::vec3(x, y, z);
}
glm::vec4 parseVec4(std::string positionRaw){
  float x, y, z, w;
  std::istringstream in(positionRaw);
  in >> x >> y >> z >> w;
  return glm::vec4(x, y, z, w);  
}
bool maybeParseVec2(std::string positionRaw, glm::vec2& _vec){
  auto parts = filterWhitespace(split(positionRaw, ' '));
  if (parts.size() != 2){
    return false;
  }
  float vecParts[2] = { 0, 0 };
  for (int i = 0; i < 2; i++){
    float number;
    bool isFloat = maybeParseFloat(parts.at(i), number);
    if (!isFloat){
      return false;
    }
    vecParts[i] = number;
  }
  _vec.x = vecParts[0];
  _vec.y = vecParts[1];
  return true;
}

bool maybeParseVec(std::string positionRaw, glm::vec3& _vec){
  auto parts = filterWhitespace(split(positionRaw, ' '));
  if (parts.size() != 3){
    return false;
  }
  float vecParts[3] = { 0, 0, 0 };
  for (int i = 0; i < 3; i++){
    float number;
    bool isFloat = maybeParseFloat(parts.at(i), number);
    if (!isFloat){
      return false;
    }
    vecParts[i] = number;
  }
  _vec.x = vecParts[0];
  _vec.y = vecParts[1];
  _vec.z = vecParts[2];
  return true;
}
bool maybeParseVec4(std::string positionRaw, glm::vec4& _vec){
  auto parts = filterWhitespace(split(positionRaw, ' '));
  if (parts.size() != 4){
    return false;
  }
  float vecParts[4] = { 0, 0, 0, 0 };
  for (int i = 0; i < 4; i++){
    float number;
    bool isFloat = maybeParseFloat(parts.at(i), number);
    if (!isFloat){
      return false;
    }
    vecParts[i] = number;
  }
  _vec.x = vecParts[0];
  _vec.y = vecParts[1];
  _vec.z = vecParts[2];
  _vec.w = vecParts[3];
  return true; 
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
  for (auto strValue : values){
    floats.push_back(std::atof(strValue.c_str()));
  }
  return floats;
}

bool maybeParseBool(std::string value, bool* _value){
  if (value == "true"){
    *_value = true;
    return true;
  }
  if (value == "false"){
    *_value = false;
    return true;
  }
  return false;
}

glm::quat eulerToQuat(glm::vec3 eulerAngles){
  return glm::quat(glm::vec3(eulerAngles.x, eulerAngles.y, eulerAngles.z));
}

// im misunderstanding what the quaternion represnts, thais is only for the single rotation,
// so what i could do, is simply rotate 
glm::quat parseQuat(glm::vec4 vecXYZRotation){
  auto parsedVec = glm::normalize(glm::vec3(vecXYZRotation.x, vecXYZRotation.y, vecXYZRotation.z));
  auto direction = orientationFromPos(glm::vec3(0.f, 0.f, 0.f), parsedVec);
  auto rotateAngle = glm::angleAxis(glm::radians(vecXYZRotation.w), glm::vec3(0, 0, 1));
  return  direction * rotateAngle;
}

float angleFromQuat(glm::quat rotation){
  return glm::acos(rotation.w) * 2.f;
}

// Fails serialization by .04 degrees... which is probably ok...but why, remnant of just float ops?  
// At least would be nice to round the values to nearest degree maybe? 
glm::vec4 serializeQuatToVec4(glm::quat rotation){
  auto axis = rotation * glm::vec3(0.f, 0.f, -1.f);
  auto axisOrientation  = orientationFromPos(glm::vec3(0.f, 0.f, 0.f), axis);
  float w = angleFromQuat(glm::inverse(axisOrientation) * rotation);
  //std::cout << "(radians, degree) : " << w << " , " << glm::degrees(w) << std::endl;
  float degreesAngle = glm::degrees(w);
  return glm::vec4(axis.x, axis.y, axis.z, degreesAngle);  
}
 
std::string serializeQuat(glm::quat rotation){
  return serializeVec(serializeQuatToVec4(rotation)); 
}

glm::vec3 quatToVec(glm::quat quat){
  return quat * glm::vec3(0.f, 0.f, -1.f);    // rotate the forward direction by the quat. 
}
glm::quat orientationFromPos(glm::vec3 fromPos, glm::vec3 targetPosition){
  // @TODO consider extracting a better up direction from current orientation
  // https://stackoverflow.com/questions/18151845/converting-glmlookat-matrix-to-quaternion-and-back/29992778
  // This feels like a really bad hack, but if an object is just straight up, this returns NaN. 
  // Should look more into the math!  How to pick up vector properly? 
  if (fromPos.x == targetPosition.x && fromPos.z == targetPosition.z && !(fromPos.y == targetPosition.y)){   
    if (fromPos.y < targetPosition.y){
      return  glm::quat(glm::vec3(glm::radians(90.f), 0, 0));
    }
    return glm::quat(glm::vec3(glm::radians(-90.f), 0, 0));
  }
  return glm::normalize(glm::conjugate(glm::quat_cast(glm::lookAt(fromPos, targetPosition, glm::vec3(0, 1, 0)))));
}

std::string serializeFloat(float value){
  char buff[100];
  snprintf(buff, sizeof(buff), "%g", value);
  return buff;
}
std::string serializeVec(glm::vec4 vec){
  return serializeFloat(vec.x) + " " + serializeFloat(vec.y) + " " + serializeFloat(vec.z) + " " + serializeFloat(vec.w);
}
std::string serializeVec(glm::vec3 vec){
  return serializeFloat(vec.x) + " " + serializeFloat(vec.y) + " " + serializeFloat(vec.z);
}
std::string serializeVec(glm::vec2 vec){
  return serializeFloat(vec.x) + " " + serializeFloat(vec.y);
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


const int NUM_BITS_RESERVED = 13;  // 8192
const int32_t MAX_ID_RESERVED = 1 << NUM_BITS_RESERVED;

bool isReservedObjId(objid id){
  return id < MAX_ID_RESERVED && id >= 0;
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

  if (randId == -1 || isReservedObjId(randId)){              // TODO - hack, -1 used as sentinel, need to eliminate those and then can get rid of this (scenegraph mostly)
    return getUniqueObjId();
  }
  return randId;
}


int32_t currentReservedId = 0;
objid getUniqueObjIdReserved(int claimAmount){
  currentReservedId += claimAmount;
  modassert(currentReservedId <= MAX_ID_RESERVED, "reserved id too big");
  return currentReservedId;
}
void resetReservedId(){
  currentReservedId = 0;
}


std::string getUniqueObjectName(std::string&& prefix){
  return prefix + std::to_string(getUniqueObjId()) + ")";
}

AttributeValue parseAttributeValue(std::string payload){
  glm::vec3 vec(0.f, 0.f, 0.f);
  bool isVec = maybeParseVec(payload, vec);
  if (isVec){
    return vec;
  }

  glm::vec4 vec4(0.f, 0.f, 0.f, 0.f);
  bool isVec4 = maybeParseVec4(payload, vec4);
  if (isVec4){
    return vec4;
  }

  float number = 0.f;
  bool isFloat = maybeParseFloat(payload, number);
  if (isFloat){
    return number;
  } 
  return payload;
}

std::string attributeTypeStr(AttributeValueType type){
  if (type == ATTRIBUTE_VEC2){
    return "vec2";
  }else if (type == ATTRIBUTE_VEC3){
    return "vec3";
  }else if (type == ATTRIBUTE_VEC4){
    return "vec4";
  }else if (type == ATTRIBUTE_FLOAT){
    return "float";
  }else if (type == ATTRIBUTE_STRING){
    return "string";
  }
  modassert(false, "attribute type str invalid type");
  return "";
}

AttributeValue addAttributes(AttributeValue one, AttributeValue two){
  auto valueOne = std::get_if<glm::vec3>(&one);
  auto valueTwo = std::get_if<glm::vec3>(&two);
  if (valueOne != NULL){
    modassert(valueTwo != NULL, "Incompatible types between vec3 value one and two");
    return *valueOne + *valueTwo;
  }


  auto value4One = std::get_if<glm::vec4>(&one);
  auto value4Two = std::get_if<glm::vec4>(&two);
  if (value4One != NULL){
    modassert(value4Two != NULL, "Incompatible types between vec4 value one and two");
    return *value4One + *value4Two;
  }

  auto fValueOne = std::get_if<float>(&one);
  auto fValueTwo = std::get_if<float>(&two);
  if (fValueOne != NULL){
    modassert(fValueTwo != NULL, "Incompatible types between float value one and two");
    return *fValueOne + *valueTwo;
  }
  modassert(false, "addAttributes value types not supported");
  return 0;
}

std::string print(AttributeValue& value){
  auto attrStrValue = std::get_if<std::string>(&value);
  if (attrStrValue != NULL){
    return *attrStrValue + "[string]";
  }
  auto attrFloatValue = std::get_if<float>(&value);
  if (attrFloatValue != NULL){
    return std::to_string(*attrFloatValue) + "[float]";
  }
  auto attrVec3Value = std::get_if<glm::vec3>(&value);
  if (attrVec3Value != NULL){
    return print(*attrVec3Value) + "[vec3]";
  }
  auto attrVec4Value = std::get_if<glm::vec4>(&value);
  if (attrVec4Value != NULL){
    return print(*attrVec4Value) + "[vec4]";
  }
  modassert(false, "invalid attribute type");
  return "";
}

AttributeValue interpolateAttribute(AttributeValue key1, AttributeValue key2, float percentage){  
  assert(percentage <= 1.f && percentage >= 0.f);
  auto attr1 = std::get_if<glm::vec3>(&key1);
  if (attr1 != NULL){
    auto attr2 = std::get_if<glm::vec3>(&key2);
    assert(attr2 != NULL);

    //std::cout << "percentage: " << percentage << std::endl;
    //std::cout << "key1: " << print(*attr1) << std::endl;
    //std::cout << "key2: " << print(*attr2) << std::endl;
    return glm::vec3(
      (attr1 -> x * (1 - percentage)) + (attr2 -> x * percentage), 
      (attr1 -> y * (1 - percentage)) + (attr2 -> y * percentage), 
      (attr1 -> z * (1 - percentage)) + (attr2 -> z * percentage)
    );
  }
  auto attr2 = std::get_if<float>(&key1);
  if (attr2 != NULL){
    auto attr2Float = std::get_if<float>(&key2);
    assert(attr2Float != NULL);
    return *attr1 + *attr2Float;
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

int numUniqueDepthLayers(std::vector<LayerInfo> layers){
  std::set<int> depths;
  for (auto layer : layers){
    depths.insert(layer.depthBufferLayer);
  }
  return depths.size();
}

std::string print(GameobjAttributes& attr){
  std::string content = "";
  content = content + "attr[" + std::to_string(attr.attr.size()) + "]\n--------\n";
  for (auto &[key, value] : attr.attr){
    content = content + key + ":" + print(value) + "\n\n"; 
  }
  return content;
}

bool aboutEqual(float one, float two){
  float delta = 0.0001f;
  return one > (two - delta) && one < (two + delta);
}
bool aboutEqual(glm::vec2 one, glm::vec2 two){
  return aboutEqual(one.x, two.x) && aboutEqual(one.y, two.y);
}
bool aboutEqual(glm::vec3 one, glm::vec3 two){
  return aboutEqual(one.x, two.x) && aboutEqual(one.y, two.y) && aboutEqual(one.z, two.z);
}
bool aboutEqualNormalized(glm::vec3 one, glm::vec3 two){
  return aboutEqual(glm::normalize(one), glm::normalize(two));
}
bool aboutEqual(glm::vec4 one, glm::vec4 two){
  return aboutEqual(one.x, two.x) && aboutEqual(one.y, two.y) && aboutEqual(one.z, two.z) && aboutEqual(one.w, two.w);
}
bool aboutEqual(AttributeValue one, AttributeValue two){
  auto strValueOne = std::get_if<std::string>(&one);
  auto strValueTwo = std::get_if<std::string>(&two);
  if (strValueOne && strValueTwo){
    return *strValueOne == *strValueTwo;
  }
  auto floatValueOne = std::get_if<float>(&one);
  auto floatValueTwo = std::get_if<float>(&two);
  if (floatValueOne && floatValueTwo){
    return aboutEqual(*floatValueOne, *floatValueTwo);
  }
  auto vec3ValueOne = std::get_if<glm::vec3>(&one);
  auto vec3ValueTwo = std::get_if<glm::vec3>(&two);
  if (vec3ValueOne && vec3ValueTwo){
    return aboutEqual(*vec3ValueOne, *vec3ValueTwo);
  }
  auto vec4ValueOne = std::get_if<glm::vec4>(&one);
  auto vec4ValueTwo = std::get_if<glm::vec4>(&two);
  if (vec4ValueOne && vec4ValueTwo){
    return aboutEqual(*vec4ValueOne, *vec4ValueTwo);
  }
  modassert(false, "cannot compare types: " + print(one) + " " + print(two));
  return false;
}


bool isIdentityVec(glm::vec4 vec){
  return vec.x = 1 && vec.y == 1 && vec.z == 1 && vec.w == 1;
}

const int maxCallstack = 128;
bool warnOnly = false;

void printBacktrace(){ 
  void* callstack[maxCallstack];
  int frames = backtrace(callstack, maxCallstack);
  char** strs = backtrace_symbols(callstack, frames);
  for (int i = 0; i < frames; ++i) {
    printf("%s\n", strs[i]);
  }
  free(strs);
}
void assertWithBacktrace(bool isTrue, std::string message){
  if (!isTrue){
    std::cout << message << std::endl;
    if (warnOnly){
      return;
    }
    printBacktrace();    
    exit(1);
  }
}

void assertTodo(std::string message){
  assertWithBacktrace(false, "TODO hit: " + message);
}

GameobjAttributes gameobjAttributes2To1(std::vector<GameobjAttribute>& attributes){
  GameobjAttributes attr {
    .attr = {},
  };
  for (auto &attrValue : attributes){
    auto stringValue = std::get_if<std::string>(&attrValue.attributeValue);
    auto floatValue = std::get_if<float>(&attrValue.attributeValue);
    auto vec2Value = std::get_if<glm::vec2>(&attrValue.attributeValue);
    auto vec3Value = std::get_if<glm::vec3>(&attrValue.attributeValue);
    auto vec4Value = std::get_if<glm::vec4>(&attrValue.attributeValue);
    if (stringValue || floatValue || vec2Value || vec3Value || vec4Value){
      attr.attr[attrValue.field] = attrValue.attributeValue;
    }else{
      modassert(false, "invalid attribute value type");
    }    
  }
  return attr; 
}

GameobjAttributes gameobjAttrFromValue(std::string field, AttributeValue value){
  std::vector<GameobjAttribute> attrs = { 
    GameobjAttribute {
      .field = field,
      .attributeValue = value,
    },
  };
  return gameobjAttributes2To1(attrs);
}

std::optional<std::string> getStrAttr(GameobjAttributes& objAttr, std::string key){
  if (objAttr.attr.find(key) != objAttr.attr.end()){
    auto strValue = std::get_if<std::string>(&objAttr.attr.at(key));
    if (!strValue){
      return std::nullopt;
    }
    return *strValue;
  }
  return std::nullopt;
}

std::optional<float> getFloatAttr(GameobjAttributes& objAttr, std::string key){
  if (objAttr.attr.find(key) != objAttr.attr.end()){
    auto floatValue = std::get_if<float>(&objAttr.attr.at(key));
    if (!floatValue){
      return std::nullopt;
    }
    return *floatValue;
  }
  return std::nullopt;
}

std::optional<int> getIntFromAttr(GameobjAttributes& objAttr, std::string key){
  auto floatAttr = getFloatAttr(objAttr, key);
  if (!floatAttr.has_value()){
    return std::nullopt;
  }
  return static_cast<int>(floatAttr.value());
}

std::optional<glm::vec3> getVec3Attr(GameobjAttributes& objAttr, std::string key){
  if (objAttr.attr.find(key) != objAttr.attr.end()){
    auto vec3Value = std::get_if<glm::vec3>(&objAttr.attr.at(key));
    if (!vec3Value){
      return std::nullopt;
    }
    return *vec3Value;
  }
  return std::nullopt;
}

std::optional<glm::vec4> getVec4Attr(GameobjAttributes& objAttr, std::string key){
  if (objAttr.attr.find(key) != objAttr.attr.end()){
    auto vec4Value = std::get_if<glm::vec4>(&objAttr.attr.at(key));
    if (!vec4Value){
      return std::nullopt;
    }
    return *vec4Value;
  }
  return std::nullopt;
}

std::optional<AttributeValue> getAttr(GameobjAttributes& objAttr, std::string key){
  auto strAttr = getStrAttr(objAttr, key);
  if (strAttr.has_value()){
    return strAttr;
  }
  auto floatAttr = getFloatAttr(objAttr, key);
  if (floatAttr.has_value()){
    return floatAttr;
  }
  auto vec3Attr = getVec3Attr(objAttr, key);
  if (vec3Attr.has_value()){
    return vec3Attr;
  }
  auto vec4Attr = getVec4Attr(objAttr, key);
  if (vec4Attr.has_value()){
    return vec4Attr;
  }

  return std::nullopt;
}


bool hasAttribute(GameobjAttributes& attrs, std::string& type){
  if (attrs.attr.find(type) != attrs.attr.end()){
    return true;
  }
  return false;
}

void mergeAttributes(GameobjAttributes& toAttributes, GameobjAttributes& fromAttributes){
  for (auto &[name, value] : fromAttributes.attr){
    toAttributes.attr[name] = value;
  }
}

const char* levelToString(MODLOG_LEVEL level){
  if (level == MODLOG_INFO){
    return "INFO";
  }else if (level == MODLOG_WARNING){
    return "WARNING";
  }else if (level == MODLOG_ERROR){
    return "ERROR";
  }
  modassert(false, "invalid modlog level");
  return "";
}

static std::vector<std::string> modlogLevels;
static bool shouldFilterLogs = false;
static MODLOG_LEVEL filterLevel;

std::function<void(std::string&)> defaultLoggingFn = [](std::string& message) -> void {
  std::cout << message << std::endl;
};
std::function<void(std::string&)> loggingFn = defaultLoggingFn;

void modlog(const char* identifier, const char* value, MODLOG_LEVEL level){
  if (level < filterLevel){
    return;
  }
  if (!shouldFilterLogs){
    std::string message("modlog: ");
    message += levelToString(level);
    message += " : ";
    message += identifier;
    message += " : ";
    message += value;
    loggingFn(message);
  }else{
    for (auto &modlogLevel : modlogLevels){
      if (identifier == modlogLevel){
        std::string message("modlog: ");
        message += levelToString(level);
        message += " : ";
        message += identifier;
        message += " : ";
        message += value;
        loggingFn(message);
        return;
      }
    }
  }
}
void modlog(const char* identifier, std::string value, MODLOG_LEVEL level){
 modlog(identifier, value.c_str(), level);
}
void modlogSetEnabled(bool filterLogs, MODLOG_LEVEL level, std::vector<std::string> levels){
  modlogLevels = levels;
  shouldFilterLogs = filterLogs;
  filterLevel = level;
}

void modlogSetLogEndpoint(std::optional<std::function<void(std::string&)>> fn){
  if (!fn.has_value()){
    loggingFn = defaultLoggingFn;
    return;
  }
  loggingFn = fn.value();
}

std::string mainTargetElement(std::string target){
  return split(target, '/').at(0);
}
std::string suffixTargetElement(std::string target){
  auto values = split(target, '/');
  std::vector<std::string> rest;
  for (int i = 1; i < values.size(); i++){
    rest.push_back(values.at(i));
  }
  return join(rest, '/');
}

std::string rewriteTargetName(std::string target, std::string newname){
  auto suffixTarget = suffixTargetElement(target);
  if (suffixTarget != ""){
    return newname + "/" + suffixTarget;
  }
  return newname;
}

bool isSubelementName(std::string& name){
  auto numTokens = split(name, '/');
  return numTokens.size() > 1;
}

std::optional<std::string> subelementTargetName(std::string& name){
  if (!isSubelementName(name)){
    return std::nullopt;
  }
  return suffixTargetElement(name);
}

std::vector<GameobjAttribute> allKeysAndAttributes(GameobjAttributes& attributes){
  std::vector<GameobjAttribute> values;
  for (auto &[key, value] : attributes.attr){
    values.push_back(GameobjAttribute{
      .field = key,
      .attributeValue = value,
    });
  }
  return values;
}


std::optional<AttributeValue> getAttributeValue(GameobjAttributes& attributes, const char* field){
  auto attrs = allKeysAndAttributes(attributes);
  for (auto &attr : attrs){
    if (attr.field == field){
      return attr.attributeValue;
    }
  }
  return std::nullopt;
}