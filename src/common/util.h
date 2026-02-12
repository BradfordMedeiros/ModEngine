#ifndef MOD_UTIL
#define MOD_UTIL

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
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
#include <any>
#include <queue>
#include <execinfo.h>

void printBacktrace();
void assertWithBacktrace(bool isTrue, std::string message);
void assertWithBacktraceWarn(bool isTrue, std::string message);
void assertTodo(std::string message);

#ifdef MODDEBUG
  #define modassert(m,x) do { if(!(m)) { assertWithBacktrace(m, x); } } while(0) ;  // the while loop thing is just for sanitization purposes of syntax.  
  #define modassertwarn(m,x) do { if(!(m)) { assertWithBacktraceWarn(m, x); } } while(0) ;  // the while loop thing is just for sanitization purposes of syntax.  

  #ifdef ASSERT_TODOS
    #define MODTODO(m) assertTodo(m);
  #else
    #define MODTODO(m) ;
  #endif
#else
  #define modassert(m,x) ;
  #define modassertwarn(m, x) ;
  #define MODTODO(m) ;
#endif

#define VAL(x) #x
#define STR(macro) VAL(macro)   // stringified the value, eg for include 

enum MODLOG_LEVEL { MODLOG_INFO = 0, MODLOG_WARNING = 1, MODLOG_ERROR = 2 };
void modlog(const char* identifier, const char* value, MODLOG_LEVEL level = MODLOG_INFO);
void modlog(const char* identifier, std::string value, MODLOG_LEVEL level = MODLOG_INFO);
void modlogSetEnabled(bool filterLogs, MODLOG_LEVEL level, std::vector<std::string> levels);
void modlogSetLogEndpoint(std::optional<std::function<void(std::string&)>> fn);



bool stringContains(std::string& str, char character);
bool stringContains(std::string& str, const char* value);
bool stringEndsWith(std::string& str, const char* value);
std::string trim(const std::string& str);
std::vector<std::string> split(std::string strToSplit, char delimeter);
std::string join(std::vector<std::string> values, char delimeter);
std::pair<std::string, std::string> carAndRest(std::string& strToSplit, char delimeter);
std::vector<std::string> subvector(std::vector<std::string>& values, int indexLow, int indexHigh);
std::vector<std::string> filterWhitespace(std::vector<std::string> values);
std::vector<std::string> splitNoWhitespace(std::string string, char character);
std::vector<std::string> filterComments(std::vector<std::string> values);

enum ManipulatorMode { NONE, ROTATE, TRANSLATE, SCALE };
enum Axis { NOAXIS, XAXIS, YAXIS, ZAXIS };
enum SNAPPING_MODE { SNAP_CONTINUOUS, SNAP_ABSOLUTE, SNAP_RELATIVE };

typedef int32_t objid;
objid getUniqueObjId();
objid getUniqueObjIdReserved(int claimAmount = 1);
bool isReservedObjId(objid);
void resetReservedId();

std::string getUniqueObjectName(std::string&& prefix);

std::string print(bool value);
std::string print(glm::vec3 vec);
std::string print(glm::vec4 vec);
std::string print(glm::ivec3 vec);
std::string print(glm::vec2 vec);
std::string print(std::optional<glm::vec2> vec);
std::string print(glm::quat quat);
std::string rawprint(glm::quat quat);
std::string print(glm::mat4 mat);
std::string print(glm::mat3 mat);
std::string print(glm::mat2 mat);
std::string print(std::vector<std::string>& values);
std::string print(std::vector<objid>& values);
std::string print(std::optional<std::string> value);
std::string print(std::set<objid> id);
std::string print(std::vector<bool>& values);
std::string print(std::set<std::string>& values);
std::string print(std::set<unsigned int>& values);
std::string print(std::vector<float>& values);
std::string print(std::optional<objid> id);
std::string print(std::optional<bool> value);
std::string print(void*);

glm::vec3 parseVec(std::string positionRaw);
glm::vec4 parseVec4(std::string positionRaw);
glm::vec3 parseVec3(std::string positionRaw);
bool maybeParseVec(std::string positionRaw, glm::vec3& _vec);
bool maybeParseVec4(std::string positionRaw, glm::vec4& _vec);
glm::vec2 parseVec2(std::string positionRaw);
bool maybeParseVec2(std::string positionRaw, glm::vec2& _vec);

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

struct DeleteAttribute{};
enum AttributeValueType { ATTRIBUTE_VEC2, ATTRIBUTE_VEC3, ATTRIBUTE_VEC4, ATTRIBUTE_STRING, ATTRIBUTE_ARR_STRING, ATTRIBUTE_FLOAT };
typedef std::variant<glm::vec2, glm::vec3, glm::vec4, std::string, std::vector<std::string>, float, bool, DeleteAttribute> AttributeValue;
typedef std::variant<glm::vec2*, glm::vec3*, glm::vec4*, std::string*, float*, bool*, int*, uint*, glm::quat*> AttributeValuePtr;
std::string attributeTypeStr(AttributeValueType type);
AttributeValue timeAdjustedAttribute(AttributeValue delta, float timestep);


template <typename T>
std::optional<T*> getTypeFromAttr(std::optional<AttributeValuePtr> ptrValue){
  if (!ptrValue.has_value()){
    return std::nullopt;
  }
  T** valuePtrPtr = std::get_if<T*>(&ptrValue.value());
  assert(valuePtrPtr);
  T* valuePtr = *valuePtrPtr;
  return valuePtr;
}


template<typename T>
T unwrapAttr(AttributeValue value) {   
  T* unwrappedValue = std::get_if<T>(&value);
  if (unwrappedValue == NULL){
    modassert(false, "unwrapAttr invalid type");
  }
  return *unwrappedValue;
}

template<typename T>
std::optional<T> unwrapAttrOpt(std::optional<AttributeValue> value) {   
  if (!value.has_value()){
    return std::nullopt;
  }
  T* unwrappedValue = std::get_if<T>(&(value.value()));
  if (unwrappedValue == NULL){
    assert(false);
  }
  return *unwrappedValue;
}

template<typename T>
std::optional<T> maybeUnwrapAttrOpt(std::optional<AttributeValue> value) {   
  if (!value.has_value()){
    return std::nullopt;
  }
  T* unwrappedValue = std::get_if<T>(&(value.value()));
  if (unwrappedValue == NULL){
    return std::nullopt;
  }
  return *unwrappedValue;
}

AttributeValue parseAttributeValue(std::string payload);
AttributeValue addAttributes(AttributeValue one, AttributeValue two);
std::string print(AttributeValue& value);

struct ScenegraphDebug {
  std::string parent;
  objid parentId;
  std::string child;
  objid childId;
  objid parentScene;
  objid childScene;
};

struct StringAttribute {
  std::string strTopic;
  std::any strValue;
};

struct HitObject {
  objid id;
  int mask;
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
  int symbol;
  int zIndex;
  bool lighting;
  bool orthographic;
  bool scale;
  bool disableViewTransform;
  bool visible;
  int depthBufferLayer;
  float fovRaw;
  float nearplane;
  float farplane;
  int selectIndex;
  RenderUniforms uniforms;
};
int numUniqueDepthLayers(std::vector<LayerInfo> layers);

struct GameobjAttribute {
  std::string field;
  AttributeValue attributeValue;
};


struct GameobjAttributes {
  std::unordered_map<std::string, AttributeValue> attr;
};

GameobjAttributes gameobjAttrFromValue(std::string field, AttributeValue value);

std::optional<std::string> getStrAttr(GameobjAttributes& objAttr, std::string key);
std::optional<std::vector<std::string>> getArrStrAttr(GameobjAttributes& objAttr, std::string key);
std::optional<float> getFloatAttr(GameobjAttributes& objAttr, std::string key);
std::optional<int> getIntFromAttr(GameobjAttributes& objAttr, std::string key);
std::optional<glm::vec3> getVec3Attr(GameobjAttributes& objAttr, std::string key);
std::optional<glm::vec4> getVec4Attr(GameobjAttributes& objAttr, std::string key);

// Below two are redundant, should eliminate one
std::optional<AttributeValue> getAttr(GameobjAttributes& objAttr, std::string key);
bool hasAttribute(GameobjAttributes& attrs, std::string& type);

void mergeAttributes(GameobjAttributes& toAttributes, GameobjAttributes& fromAttributes);

std::string print(GameobjAttributes& attr);

typedef void (*func_t)();

bool aboutEqual(float one, float two);
bool aboutEqual(glm::vec2 one, glm::vec2 two);
bool aboutEqual(glm::vec3 one, glm::vec3 two);
bool aboutEqualNormalized(glm::vec3 one, glm::vec3 two);
bool aboutEqual(glm::vec4 one, glm::vec4 two);
bool aboutEqual(AttributeValue one, AttributeValue two);

bool isIdentityVec(glm::vec4 vec);

template<typename T, typename N>
std::vector<T> mapKeys(std::unordered_map<T, N>& values){   
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
typedef std::function<void(int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal, float force)> colposfun;
typedef std::function<void(int32_t id, void* data, int32_t obj1, int32_t obj2, glm::vec3 pos, glm::vec3 normal, glm::vec3 oppositeNormal, float force)> id_colposfun;
typedef void(*colfun)(int32_t obj1, int32_t obj2);
typedef void(*id_colfun)(int32_t id, void* data, int32_t obj1, int32_t obj2);
typedef void(*mousecallback)(int button, int action, int mods);
typedef std::function<void(int32_t id, void* data, int button, int action, int mods)> id_mousecallback;
typedef void(*mousemovecallback)(double xPos, double yPos, float xNdc, float yNdc);
typedef std::function<void(int32_t id, void* data, double xPos, double yPos, float xNdc, float yNdc)> id_mousemovecallback;
typedef void(*scrollcallback)(double amount);
typedef std::function<void(int32_t id, void* data, double amount)> id_scrollcallback;
typedef void(*onobjectSelectedFunc)(int32_t index, glm::vec3 color, int selectIndex);
typedef std::function<void(int32_t id, void* data, int32_t index, glm::vec3 color, int selectIndex)> id_onobjectSelectedFunc;
typedef void(*onobjectHoverFunc)(int32_t index, bool hoverOn);
typedef std::function<void(int32_t id, void* data, int32_t index, bool hoverOn)> id_onobjectHoverFunc;
typedef void(*funcMappingFunc)(int32_t index);
typedef std::function<void(int32_t id, void* data, int32_t index)> id_funcMappingFunc;
typedef void(*keycallback)(int key, int scancode, int action, int mods);
typedef std::function<void(int32_t id, void* data, int key, int scancode, int action, int mods)> id_keycallback;
typedef void(*keycharcallback)(unsigned int codepoint);
typedef std::function<void(int32_t id, void* data, unsigned int codepoint)> id_keycharcallback;
typedef void(*stringboolFunc)(std::string, bool value);
typedef std::function<void(int32_t id, std::string, bool value)> id_stringboolFunc;
typedef void(*string2func)(std::string&, std::any&);
typedef std::function<void(int32_t, void*, std::string&, std::any&)> id_string2func;
typedef void(*stringfunc)(std::string&);
typedef std::function<void(int32_t, std::string&)> id_stringfunc;

typedef void(*messagefunc)(std::queue<StringAttribute>&);


std::string mainTargetElement(std::string target);
std::string suffixTargetElement(std::string target);
std::string rewriteTargetName(std::string target, std::string newname);
bool isSubelementName(std::string& name);
std::optional<std::string> subelementTargetName(std::string& name);

std::vector<GameobjAttribute> allKeysAndAttributes(GameobjAttributes& attributes);
std::optional<AttributeValue> getAttributeValue(GameobjAttributes& attributes, const char* field);

struct Token {
  std::string target;
  std::string attribute;
  std::string payload;
};

enum RecordingPlaybackType { RECORDING_PLAY_ONCE, RECORDING_PLAY_ONCE_REVERSE, RECORDING_PLAY_LOOP, RECORDING_PLAY_LOOP_REVERSE, RECORDING_SETONLY };
struct RecordingOptionResume{};
struct RecordingOptionResumeAtTime{
  float elapsedTime;
};
typedef std::variant<RecordingOptionResume, RecordingOptionResumeAtTime> PlayRecordingOption;

struct RecordingState {
  float timeElapsed;
  float length;
};

struct ModAABB {
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 min;
  glm::vec3 max;
};

struct ModAABB2 {
  glm::vec3 position;
  glm::vec3 size;
};

struct Bounds {
  glm::vec3 topLeftFront;
  glm::vec3 topRightFront;
  glm::vec3 bottomLeftFront;
  glm::vec3 bottomRightFront;
  glm::vec3 topLeftBack;
  glm::vec3 topRightBack;
  glm::vec3 bottomLeftBack;
  glm::vec3 bottomRightBack;
};

Bounds toBounds(ModAABB2& aabb);

template <typename T>
T* anycast(std::any& anyValue){
  try {
    T* value = std::any_cast<T>(&anyValue);
    //modassert(value, std::string("anycast value was NULL: ") + anyValue.type().name());
    return value;
  }catch(...){
    return NULL;
  }
}

struct RotationDirection {
  glm::vec3 position;
  glm::vec3 direction;
  glm::vec3 viewDir;
  glm::vec3 projectedPosition;
};

struct Transformation {
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
};

enum AnimationType { ONESHOT, LOOP, FORWARDS };
std::string print(AnimationType type);

struct PositionAndScale {
  glm::vec3 position;
  glm::vec3 size;
};
struct PositionAndScaleVerts {
  std::vector<glm::vec3> verts;
  glm::vec3 centeringOffset;
  std::vector<Transformation> specialBlocks;
};

struct ShapeOptions {
  std::optional<unsigned int> shaderId = std::nullopt;
  std::optional<int> zIndex = std::nullopt;
};

struct SubstMatch {
  int index;
  int endIndex;
  std::string key;
};
std::vector<SubstMatch> envSubstMatches(std::string& content);

struct EnvSubstResult {
  std::string result;
  std::string error;
  bool valid;
};
EnvSubstResult envSubst(std::string content, std::unordered_map<std::string, std::string> values);

struct Sampler2D { 
  int textureUnitId; 
};
struct SamplerCube {
  int textureUnitId;
};
typedef std::variant<bool, float, glm::vec2, glm::vec3, glm::vec4, Sampler2D, SamplerCube, glm::mat4, int> UniformValue;
struct UniformData {
  std::string name;
  UniformValue value;
};

struct RenderStagesDofInfo {
  int blurAmount;
  float minBlurDistance;
  float maxBlurDistance;
  float nearplane;
  float farplane;
};


enum FILE_EXTENSION_TYPE { IMAGE_EXTENSION, AUDIO_EXTENSION, MODEL_EXTENSION, UNKNOWN_EXTENSION, EFFEKSEEKER_EXTENSION };
FILE_EXTENSION_TYPE getFileType(std::string filepath);
std::optional<std::string> getExtension(std::string file);
std::optional<std::string> getPreExtension(std::string file);

std::string relativePath(std::string folder, std::string path, std::string workingDir);
bool isInFolder(std::string folder, std::string path, std::string workingDir);
bool isExtensionType(std::string& file, std::vector<std::string>& extensions);

struct FileDecomposition {
  std::string dirPath;
  std::string filename;
  std::string extension;
};

FileDecomposition decomposePath(const std::string& filepath);
std::string print(FileDecomposition& file);

struct Hint {
  const char* hint = NULL;
};


enum CONSOLE_COLOR { CONSOLE_COLOR_GREEN, CONSOLE_COLOR_RED, CONSOLE_COLOR_BLUE, CONSOLE_COLOR_YELLOW };
void printColor(std::string str, std::optional<CONSOLE_COLOR> color);
std::string inColor(std::string str, std::optional<CONSOLE_COLOR> color);

struct TagInfo {
  int key;
  //int value;
  std::string value; // this should be a union type
};
enum OctreeMaterial { OCTREE_MATERIAL_DEFAULT, OCTREE_MATERIAL_WATER };


typedef std::variant<std::string, std::vector<std::string>, bool, int, float> JsonType;
std::string saveToJson(std::unordered_map<std::string, std::unordered_map<std::string, JsonType>>& allValues);
std::unordered_map<std::string, std::unordered_map<std::string, JsonType>> loadFromJson(std::string& fileContent, bool* success);

struct LayerOverride {
  //int zIndex;
  //bool lighting;
  //bool orthographic;
  //bool scale;
  //bool disableViewTransform;
  //int depthBufferLayer;
  std::string name;  // should use symbol instead
  std::optional<float> fovRaw;
  std::optional<bool> visible;
  //float nearplane;
  //float farplane;
};

struct DefaultBindingOption {};
struct BloomBindingOption {};
struct PortalBindingOption {};
struct TextureBindingOption { 
  bool flipCoords = false;
  std::string texture; 
};
struct DepthBindingOption {};
struct UserTextureBindingOption {};
struct Unknown1BindingOption {};
struct Unknown2BindingOption {};

typedef std::variant<
  DefaultBindingOption, 
  BloomBindingOption, 
  PortalBindingOption,
  TextureBindingOption,
  DepthBindingOption,
  UserTextureBindingOption,
  Unknown1BindingOption,
  Unknown2BindingOption
> ViewportOption;

struct ViewportMeshEnablement {
  bool enable;
  int viewport;
};

struct AxisInfo {
  float leftTrigger;
  float rightTrigger;
  float leftStickX;
  float leftStickY;
  float rightStickX;
  float rightStickY;
};

enum BUTTON_TYPE { BUTTON_A, BUTTON_B, BUTTON_X, BUTTON_Y, BUTTON_LEFT_STICK, BUTTON_RIGHT_STICK, BUTTON_START, BUTTON_LB, BUTTON_RB, BUTTON_HOME, BUTTON_UP, BUTTON_DOWN, BUTTON_LEFT, BUTTON_RIGHT };
std::string print(BUTTON_TYPE button);

struct ButtonInfo {
  bool a = false;
  bool b = false;
  bool x = false;
  bool y = false;
  bool leftStick = false;
  bool rightStick = false;
  bool start = false;
  bool leftBumper = false;
  bool rightBumper = false;
  bool home = false;
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
};

struct ControlInfo {
  AxisInfo axisInfo;
  ButtonInfo buttonInfo;
};
struct ControlInfo2{
  ControlInfo thisFrame;
  ControlInfo lastFrame;
};


#endif
