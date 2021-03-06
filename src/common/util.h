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
#include <map>
#include <set>

std::string loadFile(std::string filepath);
void saveFile(std::string filepath, std::string content);
void rmFile(std::string filepath);
void appendFile(std::string filepath, std::string content);
std::optional<std::string> getExtension(std::string file);
std::optional<std::string> getPreExtension(std::string file);
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
enum SNAPPING_MODE { SNAP_CONTINUOUS, SNAP_ABSOLUTE, SNAP_RELATIVE };

std::string print(glm::vec3 vec);
std::string print(glm::vec2 vec);
std::string print(glm::quat quat);
std::string print(glm::mat4 mat);
glm::vec3 parseVec(std::string positionRaw);
bool maybeParseVec(std::string positionRaw, glm::vec3& _vec);
glm::vec2 parseVec2(std::string positionRaw);
std::vector<float> parseFloatVec(std::string value);
bool maybeParseFloat(std::string value, float& _number);
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

typedef std::variant<glm::vec3, std::string, float> AttributeValue;

struct StringString {
  std::string strTopic;
  AttributeValue strValue;
};

struct HitObject {
  objid id;
  glm::vec3 point;
  glm::quat normal;
};

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
  int depthBufferLayer;
};
int numUniqueDepthLayers(std::vector<LayerInfo> layers);

struct GameobjAttributes {
  std::map<std::string, std::string> stringAttributes;
  std::map<std::string, double> numAttributes;
  std::map<std::string, glm::vec3> vecAttributes;
  
  // todo get rid of these fields
  std::vector<std::string> children;
};

#endif