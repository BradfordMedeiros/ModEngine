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
#include <execinfo.h>

std::string loadFile(std::string filepath);
void saveFile(std::string filepath, std::string content);
void rmFile(std::string filepath);
bool fileExists(std::string path);
std::optional<std::string> getExtension(std::string file);
std::optional<std::string> getPreExtension(std::string file);
std::vector<std::string> listFilesWithExtensions(std::string folder, std::vector<std::string> extensions);

enum FILE_EXTENSION_TYPE { IMAGE_EXTENSION, AUDIO_EXTENSION, MODEL_EXTENSION, UNKNOWN_EXTENSION };
FILE_EXTENSION_TYPE getFileType(std::string filepath);

std::string trim(const std::string& str);
std::vector<std::string> split(std::string strToSplit, char delimeter);
std::string join(std::vector<std::string> values, char delimeter);
std::vector<std::string> subvector(std::vector<std::string>& values, int indexLow, int indexHigh);
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
std::string rawprint(glm::quat quat);
std::string print(glm::mat4 mat);
std::string print(glm::mat3 mat);
std::string print(glm::mat2 mat);
std::string print(std::vector<std::string>& values);
glm::vec3 parseVec(std::string positionRaw);
glm::vec4 parseVec4(std::string positionRaw);
bool maybeParseVec(std::string positionRaw, glm::vec3& _vec);
bool maybeParseVec4(std::string positionRaw, glm::vec4& _vec);
glm::vec2 parseVec2(std::string positionRaw);
std::vector<float> parseFloatVec(std::string value);
bool maybeParseBool(std::string value, bool* _value);
bool maybeParseFloat(std::string value, float& _number);
float parseFloat(std::string value);
glm::quat eulerToQuat(glm::vec3 eulerAngles);
glm::quat parseQuat(glm::vec4 vecXYZRotation);

glm::vec3 quatToVec(glm::quat quat);
glm::quat orientationFromPos(glm::vec3 fromPos, glm::vec3 targetPosition);

std::string serializeFloat(float value);
std::string serializeVec(glm::vec4 vec);
std::string serializeVec(glm::vec3 vec);
std::string serializeVec(glm::vec2 vec);
glm::vec4 serializeQuatToVec4(glm::quat rotation);
std::string serializeQuat(glm::quat rotation);


float maxvalue(float x, float y, float z);
int maxvalue(int x, int y, int z);

typedef int32_t objid;
objid getUniqueObjId();
std::string getUniqueObjectName(std::string& prefix);

enum AttributeValueType { ATTRIBUTE_VEC3, ATTRIBUTE_VEC4, ATTRIBUTE_STRING, ATTRIBUTE_FLOAT };
typedef std::variant<glm::vec3, glm::vec4, std::string, float> AttributeValue;
std::string attributeTypeStr(AttributeValueType type);

template<typename T>
T unwrapAttr(AttributeValue value) {   
  T* unwrappedValue = std::get_if<T>(&value);
  if (unwrappedValue == NULL){
    assert(false);
  }
  return *unwrappedValue;
}

AttributeValue parseAttributeValue(std::string payload);
AttributeValue addAttributes(AttributeValue one, AttributeValue two);
std::string print(AttributeValue& value);

struct StringPairVec2 {
  std::string key;
  std::string value;
  glm::ivec2 vec;
};

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

struct StrValues {
  std::string target;
  std::string attribute;
  std::string payload;
};

struct RenderDataInt {
  std::string uniformName;
  int value;
};
struct RenderDataFloat {
  std::string uniformName;
  float value;
};
struct RenderDataFloatArr {
  std::string uniformName;
  std::vector<float> value;
};
struct RenderDataVec3 {
  std::string uniformName;
  glm::vec3 value;
};
struct RenderDataBuiltIn {
  std::string uniformName;
  std::string builtin;
};
struct RenderUniforms {
  std::vector<RenderDataInt> intUniforms;
  std::vector<RenderDataFloat> floatUniforms;
  std::vector<RenderDataFloatArr> floatArrUniforms;
  std::vector<RenderDataVec3> vec3Uniforms;
  std::vector<RenderDataBuiltIn> builtInUniforms;
};

struct LayerInfo {
  std::string name;
  int zIndex;
  bool orthographic;
  bool scale;
  bool disableViewTransform;
  bool visible;
  int depthBufferLayer;
  float fov;
  float nearplane;
  float farplane;
  int selectIndex;
  std::string cursor;
  RenderUniforms uniforms;
};
int numUniqueDepthLayers(std::vector<LayerInfo> layers);

struct vectorAttributes {
  std::map<std::string, glm::vec3> vec3;
  std::map<std::string, glm::vec4> vec4;
};
struct GameobjAttributes {
  std::map<std::string, std::string> stringAttributes;
  std::map<std::string, double> numAttributes;
  vectorAttributes vecAttr;
};

bool maybeSetVec3FromAttr(glm::vec3* _valueToUpdate, const char* field, GameobjAttributes& attributes);
bool maybeSetVec4FromAttr(glm::vec4* _valueToUpdate, const char* field, GameobjAttributes& attributes);

void mergeAttributes(GameobjAttributes& toAttributes, GameobjAttributes& fromAttributes);

std::string print(GameobjAttributes& attr);

typedef void (*func_t)();

bool aboutEqual(float one, float two);
bool aboutEqual(glm::vec2 one, glm::vec2 two);
bool aboutEqual(glm::vec3 one, glm::vec3 two);
bool aboutEqualNormalized(glm::vec3 one, glm::vec3 two);
bool aboutEqual(glm::vec4 one, glm::vec4 two);

bool isIdentityVec(glm::vec3 scale);
bool isIdentityVec(glm::vec4 vec);

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
typedef std::function<void(int32_t id)> id_func;
typedef std::function<void(int32_t id, void* data)> id_func_data;
typedef std::function<void(int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal)> colposfun;
typedef std::function<void(int32_t id, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal)> id_colposfun;
typedef void(*colfun)(int32_t obj1, int32_t obj2);
typedef void(*id_colfun)(int32_t id, void* data, int32_t obj1, int32_t obj2);
typedef void(*mousecallback)(int button, int action, int mods);
typedef std::function<void(int32_t id, void* data, int button, int action, int mods)> id_mousecallback;
typedef void(*mousemovecallback)(double xPos, double yPos, float xNdc, float yNdc);
typedef std::function<void(int32_t id, void* data, double xPos, double yPos, float xNdc, float yNdc)> id_mousemovecallback;
typedef void(*scrollcallback)(double amount);
typedef std::function<void(int32_t id, double amount)> id_scrollcallback;
typedef void(*onobjectSelectedFunc)(int32_t index, glm::vec3 color);
typedef std::function<void(int32_t id, int32_t index, glm::vec3 color)> id_onobjectSelectedFunc;
typedef void(*onobjectHoverFunc)(int32_t index, bool hoverOn);
typedef std::function<void(int32_t id, int32_t index, bool hoverOn)> id_onobjectHoverFunc;
typedef void(*funcMappingFunc)(int32_t index);
typedef std::function<void(int32_t id, int32_t index)> id_funcMappingFunc;
typedef void(*keycallback)(int key, int scancode, int action, int mods);
typedef std::function<void(int32_t id, void* data, int key, int scancode, int action, int mods)> id_keycallback;
typedef void(*keycharcallback)(unsigned int codepoint);
typedef std::function<void(int32_t id, unsigned int codepoint)> id_keycharcallback;
typedef void(*stringboolFunc)(std::string, bool value);
typedef std::function<void(int32_t id, std::string, bool value)> id_stringboolFunc;
typedef void(*string2func)(std::string&, AttributeValue&);
typedef std::function<void(int32_t, void*, std::string&, AttributeValue&)> id_string2func;
typedef void(*stringfunc)(std::string&);
typedef std::function<void(int32_t, std::string&)> id_stringfunc;

typedef void(*messagefunc)(std::queue<StringString>&);

void assertWithBacktrace(bool isTrue, std::string message);
void assertTodo(std::string message);

#ifdef MODDEBUG
  #define modassert(m,x) assertWithBacktrace(m, x);
  #ifdef ASSERT_TODOS
    #define MODTODO(m) assertTodo(m);
  #else
    #define MODTODO(m) ;
  #endif
#else
  #define modassert(m,x) ;
  #define MODTODO(m) ;
#endif

enum MODLOG_LEVEL { MODLOG_INFO = 0, MODLOG_WARNING = 1, MODLOG_ERROR = 2 };
void modlog(const char* identifier, const char* value, MODLOG_LEVEL level = MODLOG_INFO);
void modlog(const char* identifier, std::string value, MODLOG_LEVEL level = MODLOG_INFO);
void modlogSetEnabled(bool filterLogs, MODLOG_LEVEL level, std::vector<std::string> levels);

std::string mainTargetElement(std::string target);
std::string suffixTargetElement(std::string target);
std::string rewriteTargetName(std::string target, std::string newname);
bool isSubelementName(std::string& name);
std::optional<std::string> subelementTargetName(std::string& name);

struct AttributeKeyAndValue {
  std::string attribute;
  AttributeValue payload;
};

std::vector<AttributeKeyAndValue> allKeysAndAttributes(GameobjAttributes& attributes);

struct Token {
  std::string target;
  std::string attribute;
  std::string payload;
};

enum RecordingPlaybackType { RECORDING_PLAY_ONCE, RECORDING_PLAY_LOOP };

#endif
