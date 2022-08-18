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
  std::function<void(int)> releaseTexture;
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
  std::function<std::string(std::string)> pathForModLayer;
};

void attrSet(GameobjAttributes& attr, bool* value, const char* field);
void attrSet(GameobjAttributes& attr, bool* value, bool defaultValue, const char* field);
void attrSet(GameobjAttributes& attr, bool* _value, const char* onString, const char* offString, const char* field, bool strict);
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

struct AutoSerializeRequiredString {
  size_t structOffset;
  const char* field;
};

struct AutoSerializeFloat {
  size_t structOffset;
  std::optional<size_t> structOffsetFiller;
  const char* field;
  float defaultValue;
};

struct AutoSerializeTextureLoader {
  size_t structOffset;
  std::optional<size_t> structOffsetName;
  const char* field;
  std::string defaultValue;
}; 

struct TextureLoadingData {
  int textureId;
  std::string textureString;
  bool isLoaded;
};
struct AutoSerializeTextureLoaderManual {
  size_t structOffset;
  const char* field;
  std::string defaultValue;

};

struct AutoSerializeInt {
  size_t structOffset;
  const char* field;
  int defaultValue;
};

struct AutoSerializeUInt {
  size_t structOffset;
  const char* field;
  uint defaultValue;
};

struct AutoSerializeVec2 {
  size_t structOffset;
  std::optional<size_t> structOffsetFiller;
  const char* field;
  glm::vec2 defaultValue;
};

struct AutoSerializeVec3 {
  size_t structOffset;
  std::optional<size_t> structOffsetFiller;
  const char* field;
  glm::vec3 defaultValue;
};

struct AutoSerializeVec4 {
  size_t structOffset;
  std::optional<size_t> structOffsetFiller;
  const char* field;
  glm::vec4 defaultValue;
};

struct AutoSerializeEnums {
  size_t structOffset;
  std::vector<int> enums;
  std::vector<std::string> enumStrings;
  const char* field;
  int defaultValue;
};

struct AutoSerializeCustom {
  size_t structOffset;
  const char* field;
  AttributeValueType fieldType;
  std::function<void(ObjectTypeUtil& util, void* offset, void* value)> deserialize;
  std::function<void(ObjectSetAttribUtil& util, void* offset, void* fieldValue)> setAttributes;
  std::function<AttributeValue(void* offset)> getAttribute;
};

typedef std::variant<AutoSerializeBool, AutoSerializeString, AutoSerializeRequiredString, AutoSerializeFloat, AutoSerializeTextureLoader, AutoSerializeTextureLoaderManual, AutoSerializeInt, AutoSerializeUInt, AutoSerializeVec2, AutoSerializeVec3, AutoSerializeVec4, AutoSerializeEnums, AutoSerializeCustom> AutoSerialize;
void createAutoSerialize(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attr, ObjectTypeUtil& util);
void createAutoSerializeWithTextureLoading(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attr, ObjectTypeUtil& util);
void autoserializerSerialize(char* structAddress, std::vector<AutoSerialize>& values, std::vector<std::pair<std::string, std::string>>& _pairs);
void autoserializerGetAttr(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& _attributes);
void autoserializerSetAttr(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attributes, ObjectSetAttribUtil& util);
void autoserializerSetAttrWithTextureLoading(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

template <typename T>
int addCommonAutoserializer(std::vector<AutoSerialize>& autoserializer){
  autoserializer.push_back(
    AutoSerializeString {
      .structOffset = offsetof(T, common.onFocus),
      .field = "focus",
      .defaultValue = "",
    }
  );
  autoserializer.push_back(
    AutoSerializeString {
      .structOffset = offsetof(T, common.onBlur),
      .field = "blur",
      .defaultValue = "",
    }
  );
  return 0;
}

template <typename T>
int addTextureAutoserializer(std::vector<AutoSerialize>& autoserializer){
  // attrSet(attr, &info.textureoffset, glm::vec2(0.f, 0.f), "textureoffset");
  autoserializer.push_back(
    AutoSerializeVec2 {
      .structOffset = offsetof(T, texture.textureoffset),
      .structOffsetFiller = std::nullopt,
      .field = "textureoffset",
      .defaultValue = glm::vec2(0.f, 0.f),
    }
  );

  // attrSet(attr, &info.texturetiling, glm::vec2(1.f, 1.f), "texturetiling");
  autoserializer.push_back(
    AutoSerializeVec2 {
      .structOffset = offsetof(T, texture.texturetiling),
      .structOffsetFiller = std::nullopt,
      .field = "texturetiling",
      .defaultValue = glm::vec2(1.f, 1.f),
    }
  );

  //attrSet(attr, &info.texturesize, glm::vec2(1.f, 1.f), "texturesize");
  autoserializer.push_back(
    AutoSerializeVec2 {
      .structOffset = offsetof(T, texture.texturesize),
      .structOffsetFiller = std::nullopt,
      .field = "texturesize",
      .defaultValue = glm::vec2(1.f, 1.f),
    }
  );

  autoserializer.push_back(
    AutoSerializeTextureLoader {
      .structOffset = offsetof(T, texture.textureOverloadId),
      .structOffsetName = offsetof(T, texture.textureOverloadName),
      .field = "texture",
      .defaultValue = "",
    }
  );
  

  return 0;
}

#endif