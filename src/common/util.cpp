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
    std::to_string(mat[0][0]) + ", " + std::to_string(mat[0][1]) + ", " + std::to_string(mat[0][2]) + ", " + std::to_string(mat[0][3]) + "; " +
    std::to_string(mat[1][0]) + ", " + std::to_string(mat[1][1]) + ", " + std::to_string(mat[1][2]) + ", " + std::to_string(mat[1][3]) + "; " +
    std::to_string(mat[2][0]) + ", " + std::to_string(mat[2][1]) + ", " + std::to_string(mat[2][2]) + ", " + std::to_string(mat[2][3]) + "; " +
    std::to_string(mat[3][0]) + ", " + std::to_string(mat[3][1]) + ", " + std::to_string(mat[3][2]) + ", " + std::to_string(mat[3][3]) + 
  "]";
}
std::string print(glm::mat3 mat){
  return "[" + 
    std::to_string(mat[0][0]) + ", " + std::to_string(mat[0][1]) + ", " + std::to_string(mat[0][2]) + "; " +
    std::to_string(mat[1][0]) + ", " + std::to_string(mat[1][1]) + ", " + std::to_string(mat[1][2]) + "; " +
    std::to_string(mat[2][0]) + ", " + std::to_string(mat[2][1]) + ", " + std::to_string(mat[2][2]) + "; " +
  "]";
}
std::string print(glm::mat2 mat){
  return "[" + 
    std::to_string(mat[0][0]) + ", " + std::to_string(mat[0][1]) + "; " +
    std::to_string(mat[1][0]) + ", " + std::to_string(mat[1][1]) + "; " +
  "]";
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
  for (auto value : values){
    floats.push_back(std::atof(value.c_str()));
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

glm::vec4 serializeQuatToVec4(glm::quat rotation){
  auto axis = rotation * glm::vec3(0.f, 0.f, -1.f);
  auto axisOrientation  = orientationFromPos(glm::vec3(0.f, 0.f, 0.f), axis);
  float w = angleFromQuat(glm::inverse(axisOrientation) * rotation);
  //std::cout << "(radians, degree) : " << w << " , " << glm::degrees(w) << std::endl;
  float degreesAngle = glm::degrees(w);
  return glm::vec4(axis.x, axis.y, axis.z, degreesAngle);  
}
// Fails serialization by .04 degrees... which is probably ok...but why, remnant of just float ops?  
// At least would be nice to round the values to nearest degree maybe?  
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

std::string serializeVec(glm::vec4 vec){
  return std::to_string(vec.x) + " " + std::to_string(vec.y) + " " + std::to_string(vec.z) + " " + std::to_string(vec.w);
}
std::string serializeVec(glm::vec3 vec){
  return std::to_string(vec.x) + " " + std::to_string(vec.y) + " " + std::to_string(vec.z);
}
std::string serializeVec(glm::vec2 vec){
  return std::to_string(vec.x) + " " + std::to_string(vec.y);
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

AttributeValue parseAttributeValue(std::string payload){
  glm::vec3 vec(0.f, 0.f, 0.f);
  bool isVec = maybeParseVec(payload, vec);
  if (isVec){
    return vec;
  }
  float number = 0.f;
  bool isFloat = maybeParseFloat(payload, number);
  if (isFloat){
    return number;
  } 
  return payload;
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
  modassert(false, "string values not supported");
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

int numUniqueDepthLayers(std::vector<LayerInfo> layers){
  std::set<int> depths;
  for (auto layer : layers){
    depths.insert(layer.depthBufferLayer);
  }
  return depths.size();
}

std::string print(GameobjAttributes& attr){
  std::string content = "";
  content = content + "string-attr[" + std::to_string(attr.stringAttributes.size()) + "]\n--------\n";
  for (auto &[key, value] : attr.stringAttributes){
    content = content + key + ":" + value + "\n\n"; 
  }
  content = content + "vec3-attr[" + std::to_string(attr.vecAttr.vec3.size()) + "]\n--------\n";
  for (auto &[key, value] : attr.vecAttr.vec3){
    content = content + key + ":" + print(value) + "\n\n"; 
  }

  content = content + "vec4-attr[" + std::to_string(attr.vecAttr.vec4.size()) + "]\n--------\n";
  for (auto &[key, value] : attr.vecAttr.vec4){
    content = content + key + ":" + print(value) + "\n\n"; 
  }

  content = content + "num-attr[" + std::to_string(attr.numAttributes.size()) + "]\n--------\n";
  for (auto &[key, value] : attr.numAttributes){
    content = content + key + ":" + std::to_string(value) + "\n\n"; 
  }  
  return content;
}

bool aboutEqual(float one, float two){
  float delta = 0.0001f;
  return one > (two - delta) && one < (two + delta);
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

bool isIdentityVec(glm::vec3 scale){
  return scale.x = 1 && scale.y == 1 && scale.z == 1;
}
bool isIdentityVec(glm::vec4 vec){
  return vec.x = 1 && vec.y == 1 && vec.z == 1 && vec.w == 1;
}

const int maxCallstack = 128;
bool warnOnly = false;
void assertWithBacktrace(bool isTrue, std::string message){
  if (!isTrue){
    std::cout << message << std::endl;
    if (warnOnly){
      return;
    }
    void* callstack[maxCallstack];
    int frames = backtrace(callstack, maxCallstack);
    char** strs = backtrace_symbols(callstack, frames);
    for (int i = 0; i < frames; ++i) {
      printf("%s\n", strs[i]);
    }
    free(strs);
    exit(1);
  }
}

void assertTodo(std::string message){
  assertWithBacktrace(false, "TODO hit: " + message);
}

bool maybeSetVec3FromAttr(glm::vec3* _valueToUpdate, const char* field, GameobjAttributes& attributes){
  if (attributes.vecAttr.vec3.find(field) != attributes.vecAttr.vec3.end()){
    *_valueToUpdate = attributes.vecAttr.vec3.at(field);
    return true;
  }
  return false;
}
bool maybeSetVec4FromAttr(glm::vec4* _valueToUpdate, const char* field, GameobjAttributes& attributes){
  if (attributes.vecAttr.vec4.find(field) != attributes.vecAttr.vec4.end()){
    *_valueToUpdate = attributes.vecAttr.vec4.at(field);
    return true;
  }
  return false;
}
bool maybeSetBoolFromStrAttr(bool* _valueToUpdate, const char* field, const char* trueValue, const char* falseValue, GameobjAttributes& attributes){
  if (attributes.stringAttributes.find(field) != attributes.stringAttributes.end()){
    *_valueToUpdate = attributes.stringAttributes.at(field) == trueValue ? true : false; 
    return true;
  }
  return false;
}

void mergeAttributes(GameobjAttributes& toAttributes, GameobjAttributes& fromAttributes){
  for (auto &[name, value] : fromAttributes.stringAttributes){
    toAttributes.stringAttributes[name] = value;
  }
  for (auto &[name, value] : fromAttributes.numAttributes){
    toAttributes.numAttributes[name] = value;
  }
  for (auto &[name, value] : fromAttributes.vecAttr.vec3){
    toAttributes.vecAttr.vec3[name] = value;
  }
  for (auto &[name, value] : fromAttributes.vecAttr.vec4){
    toAttributes.vecAttr.vec4[name] = value;
  }
}