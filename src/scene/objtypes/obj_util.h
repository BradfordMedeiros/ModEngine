#ifndef MOD_OBJ_UTIL
#define MOD_OBJ_UTIL

#include <map>
#include "../common/mesh.h"
#include "../types/emitter.h"

struct GameObjectUICommon {
  Mesh mesh;
  bool isFocused;
  std::string onFocus;
  std::string onBlur;
};

struct TextureInformation {
  glm::vec2 textureoffset;
  glm::vec2 texturetiling;
  glm::vec2 texturesize;
  std::string textureOverloadName;
  int textureOverloadId;
};

struct ObjectTypeUtil {
  objid id;
  std::function<Mesh(std::string)> createMeshCopy;
  std::map<std::string, MeshRef>& meshes;
  std::function<Texture(std::string)> ensureTextureLoaded;
  std::function<Mesh(MeshData&)> loadMesh;
  std::function<void(float, float, int, GameobjAttributes&, std::vector<EmitterDelta>, bool, EmitterDeleteBehavior)> addEmitter;
  std::function<std::vector<std::string>(std::string)> ensureMeshLoaded;
  std::function<void()> onCollisionChange;
  std::function<std::string(std::string)> pathForModLayer;
};

struct ObjectSerializeUtil {
  std::function<std::string(int)> textureName;
};

struct ObjectRemoveUtil {
  objid id;
  std::function<void()> rmEmitter;
};

struct ObjectSetAttribUtil {
  std::function<void(bool)> setEmitterEnabled;
  std::function<Texture(std::string)> ensureTextureLoaded;
  std::function<void(int)> releaseTexture;
};

void attrSetCommon(GameobjAttributes& attr, GameObjectUICommon& common, std::map<std::string, MeshRef>& meshes);
void addSerializeCommon(std::vector<std::pair<std::string, std::string>>& pairs, GameObjectUICommon& common);
void setTextureInfo(GameobjAttributes& attr, std::function<Texture(std::string)> ensureTextureLoaded, TextureInformation& info);
void addSerializedTextureInformation(std::vector<std::pair<std::string, std::string>>& pairs, TextureInformation& texture);
void setTextureAttributes(TextureInformation& info, GameobjAttributes& attr, ObjectSetAttribUtil& util);

void attrSet(GameobjAttributes& attr, bool* value, const char* field);
void attrSet(GameobjAttributes& attr, bool* value, bool defaultValue, const char* field);
void attrSet(GameobjAttributes& attr, std::string* value, const char* field);
void attrSet(GameobjAttributes& attr, std::string* value, std::string defaultValue, const char* field);
void attrSetRequired(GameobjAttributes& attr, std::string* _value, const char* field);
void attrSet(GameobjAttributes& attr, float* value, const char* field);
void attrSet(GameobjAttributes& attr, float* _value, float defaultValue, const char* field);
void attrSet(GameobjAttributes& attr, float* _value, bool* _hasValue, float defaultValue, const char* field);
void attrSet(GameobjAttributes& attr, unsigned int* value, const char* field);
void attrSet(GameobjAttributes& attr, unsigned int* value, unsigned int defaultValue, const char* field);
void attrSet(GameobjAttributes& attr, int* _value, const char* field);
void attrSet(GameobjAttributes& attr, int* _value, int defaultValue, const char* field);
void attrSet(GameobjAttributes& attr, glm::vec3* _value, glm::vec3 defaultValue, const char* field);
void attrSet(GameobjAttributes& attr, glm::vec4* _value, const char* field);
void attrSet(GameobjAttributes& attr, glm::vec4* _value, glm::vec4 defaultValue, const char* field);
void attrSet(GameobjAttributes& attr, glm::vec4* _value, bool* _hasValue, glm::vec4 defaultValue, const char* field);
void attrSet(GameobjAttributes& attr, glm::vec2* _value, glm::vec2 defaultValue, const char* field);
void attrSet(GameobjAttributes& attr, bool* _value, const char* onString, const char* offString, bool defaultValue, const char* field, bool strict);
void attrSetLoadTexture(GameobjAttributes& attr, std::function<Texture(std::string)> ensureTextureLoaded, int* _textureId, std::string defaultTexture, const char* field);
void attrSetLoadTexture(GameobjAttributes& attr, std::function<Texture(std::string)> ensureTextureLoaded, int* _textureId, std::string* _textureName, std::string defaultTexture, const char* field);
void attrSet(GameobjAttributes& attr, int* _value, std::vector<int> enums, std::vector<std::string> enumStrings, int defaultValue, const char* field, bool strict);

struct AutoSerializeBool {
  size_t structOffset;
  const char* field;
  const char* onString;
  const char* offString;
  bool defaultValue;
};

struct AutoSerializeString {
  size_t structOffset;
  const char* field;
  std::string defaultValue;
};

struct AutoSerializeFloat {
  size_t structOffset;
  const char* field;
  float defaultValue;
};

struct AutoSerializeTextureLoader {
  size_t structOffset;
  std::optional<size_t> structOffsetName;
  const char* field;
  std::string defaultValue;
}; 

struct AutoSerializeUInt {
  size_t structOffset;
  const char* field;
  uint defaultValue;
};

struct AutoSerializeVec4 {
  size_t structOffset;
  std::optional<size_t> structOffsetFiller;
  const char* field;
  glm::vec4 defaultValue;
};


typedef std::variant<AutoSerializeBool, AutoSerializeString, AutoSerializeFloat, AutoSerializeTextureLoader, AutoSerializeUInt, AutoSerializeVec4> AutoSerialize;
void createAutoSerialize(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attr, ObjectTypeUtil& util);

#endif