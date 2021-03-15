#ifndef MOD_UTIL
#define MOD_UTIL

#include <string>
#include <algorithm>
#include <fstream>
#include <optional>
#include <filesystem>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <glm/glm.hpp>
#include <sstream>
#include <glm/gtc/quaternion.hpp>
#include <stdlib.h>    
#include <variant>
#include <iostream>

std::string loadFile(std::string filepath);
void saveFile(std::string filepath, std::string content);
void rmFile(std::string filepath);
void appendFile(std::string filepath, std::string content);
std::vector<std::string> listFilesWithExtensions(std::string folder, std::vector<std::string> extensions);

enum FILE_EXTENSION_TYPE { IMAGE_EXTENSION, AUDIO_EXTENSION, VIDEO_EXTENSION, MODEL_EXTENSION, UNKNOWN_EXTENSION };
FILE_EXTENSION_TYPE getFileType(std::string filepath);

std::string trim(const std::string& str);
std::vector<std::string> split(std::string strToSplit, char delimeter);
std::string join(std::vector<std::string> values, char delimeter);
std::vector<std::string> filterWhitespace(std::vector<std::string> values);
std::vector<std::string> filterComments(std::vector<std::string> values);

enum ManipulatorMode { NONE, ROTATE, TRANSLATE, SCALE };
enum Axis { NOAXIS, XAXIS, YAXIS, ZAXIS };

std::string print(glm::vec3 vec);
std::string print(glm::vec2 vec);
std::string print(glm::quat quat);
std::string print(glm::mat4 mat);
glm::vec3 parseVec(std::string positionRaw);
glm::vec2 parseVec2(std::string positionRaw);
std::vector<float> parseFloatVec(std::string value);
glm::quat eulerToQuat(glm::vec3 eulerAngles);
glm::quat parseQuat(std::string payload);
glm::vec3 quatToVec(glm::quat quat);
std::string serializeVec(glm::vec3 vec);
std::string serializeVec(glm::vec2 vec);
std::string serializeRotation(glm::quat rotation);


float maxvalue(float x, float y, float z);
int maxvalue(int x, int y, int z);

typedef int32_t objid;
objid getUniqueObjId();
std::string getUniqueObjectName();

struct StringFloat {
  std::string strValue;
  float floatValue;
};

struct HitObject {
  objid id;
  glm::vec3 point;
};

typedef std::variant<glm::vec3, std::string, float> AttributeValue;
AttributeValue interpolateAttribute(AttributeValue key1, AttributeValue key2, float percentage);
std::string serializePropertySuffix(std::string key, AttributeValue value);

struct Property {
  std::string propertyName;
  AttributeValue value;
};

struct LayerInfo {
  std::string name;
  int zIndex;
  bool orthographic;
  bool ignoreDepthBuffer;
};

#endif