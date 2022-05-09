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
#include <queue>

std::string loadFile(std::string filepath);
void saveFile(std::string filepath, std::string content);
bool fileExists(std::string path);
std::optional<std::string> getExtension(std::string file);
std::optional<std::string> getPreExtension(std::string file);
std::vector<std::string> listFilesWithExtensions(std::string folder, std::vector<std::string> extensions);

enum FILE_EXTENSION_TYPE { IMAGE_EXTENSION, AUDIO_EXTENSION, MODEL_EXTENSION, UNKNOWN_EXTENSION };
FILE_EXTENSION_TYPE getFileType(std::string filepath);

std::string trim(const std::string& str);
std::vector<std::string> split(std::string strToSplit, char delimeter);
std::string join(std::vector<std::string> values, char delimeter);
std::vector<std::string> filterWhitespace(std::vector<std::string> values);
std::vector<std::string> splitNoWhitespace(std::string string, char character);
std::vector<std::string> filterComments(std::vector<std::string> values);

enum ManipulatorMode { NONE, ROTATE, TRANSLATE, SCALE };
enum Axis { NOAXIS, XAXIS, YAXIS, ZAXIS };
enum SNAPPING_MODE { SNAP_CONTINUOUS, SNAP_ABSOLUTE, SNAP_RELATIVE };

std::string print(glm::vec3 vec);
std::string print(glm::vec4 vec);
std::string print(glm::ivec3 vec);
std::string print(glm::vec2 vec);
std::string print(glm::quat quat);
std::string print(glm::mat4 mat);
std::string print(glm::mat3 mat);
std::string print(glm::mat2 mat);
glm::vec3 parseVec(std::string positionRaw);
glm::vec4 parseVec4(std::string positionRaw);
bool maybeParseVec(std::string positionRaw, glm::vec3& _vec);
glm::vec2 parseVec2(std::string positionRaw);
std::vector<float> parseFloatVec(std::string value);
bool maybeParseBool(std::string value, bool* _value);
bool maybeParseFloat(std::string value, float& _number);
glm::quat eulerToQuat(glm::vec3 eulerAngles);
glm::quat parseQuat(std::string payload);

glm::vec3 quatToVec(glm::quat quat);
glm::quat orientationFromPos(glm::vec3 fromPos, glm::vec3 targetPosition);

std::string serializeVec(glm::vec4 vec);
std::string serializeVec(glm::vec3 vec);
std::string serializeVec(glm::vec2 vec);
std::string serializeQuat(glm::quat rotation);


float maxvalue(float x, float y, float z);
int maxvalue(int x, int y, int z);

typedef int32_t objid;
objid getUniqueObjId();
std::string getUniqueObjectName();

typedef std::variant<glm::vec3, std::string, float> AttributeValue;
AttributeValue parseAttributeValue(std::string payload);

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

struct ObjectValue {
  std::string object;
  std::string attribute;
  AttributeValue value;
};

struct LayerInfo {
  std::string name;
  int zIndex;
  bool orthographic;
  bool scale;
  bool disableViewTransform;
  int depthBufferLayer;
  float fov;
  float nearplane;
  float farplane;
  int selectIndex;
};
int numUniqueDepthLayers(std::vector<LayerInfo> layers);

struct GameobjAttributes {
  std::map<std::string, std::string> stringAttributes;
  std::map<std::string, double> numAttributes;
  std::map<std::string, glm::vec3> vecAttributes;
  
  // todo get rid of these fields
  std::vector<std::string> children;
};
std::string print(GameobjAttributes& attr);

typedef void (*func_t)();

bool aboutEqual(float one, float two);
bool aboutEqual(glm::vec3 one, glm::vec3 two);
bool aboutEqual(glm::vec4 one, glm::vec4 two);

template<typename T, typename N>
std::vector<T> mapKeys(std::map<T, N>& values){   
  std::vector<T> transformedValues;
  for (auto &[id, _]: values){
    transformedValues.push_back(id);
  }
  return transformedValues;
}


/* Move back into customobj, just here to share between this an scheme for now */
typedef void(*func)();
typedef void(*id_func)(int32_t id);
typedef void(*colposfun)(int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal);
typedef void(*id_colposfun)(int32_t id, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal);
typedef void(*colfun)(int32_t obj1, int32_t obj2);
typedef void(*id_colfun)(int32_t id, int32_t obj1, int32_t obj2);
typedef void(*mousecallback)(int button, int action, int mods);
typedef void(*id_mousecallback)(int32_t id, int button, int action, int mods);
typedef void(*mousemovecallback)(double xPos, double yPos, float xNdc, float yNdc);
typedef void(*id_mousemovecallback)(int32_t id, double xPos, double yPos, float xNdc, float yNdc);
typedef void(*scrollcallback)(double amount);
typedef void(*id_scrollcallback)(int32_t id, double amount);
typedef void(*onobjectSelectedFunc)(int32_t index, glm::vec3 color);
typedef void(*id_onobjectSelectedFunc)(int32_t id, int32_t index, glm::vec3 color);
typedef void(*onobjectHoverFunc)(int32_t index, bool hoverOn);
typedef void(*id_onobjectHoverFunc)(int32_t id, int32_t index, bool hoverOn);
typedef void(*keycallback)(int key, int scancode, int action, int mods);
typedef void(*id_keycallback)(int32_t id, int key, int scancode, int action, int mods);
typedef void(*keycharcallback)(unsigned int codepoint);
typedef void(*id_keycharcallback)(int32_t id, unsigned int codepoint);
typedef void(*stringboolFunc)(std::string, bool value);
typedef void(*id_stringboolFunc)(int32_t id, std::string, bool value);
typedef void(*string2func)(std::string&, AttributeValue&);
typedef void(*id_string2func)(int32_t, std::string&, AttributeValue&);
typedef void(*stringfunc)(std::string&);
typedef void(*id_stringfunc)(int32_t, std::string&);

typedef void(*messagefunc)(std::queue<StringString>&);

#endif


