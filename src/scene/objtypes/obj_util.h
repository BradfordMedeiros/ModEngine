#ifndef MOD_OBJ_UTIL
#define MOD_OBJ_UTIL

#include <map>
#include <set>
#include "../common/mesh.h"
#include "../types/emitter.h"

struct GameObjectUICommon {
  Mesh mesh;
  bool isFocused;
  std::string onFocus;
  std::string onBlur;
};

struct TextureLoadingData {
  int textureId;
  std::string textureString;
  bool isLoaded;
};
struct TextureInformation {
  glm::vec2 textureoffset;
  glm::vec2 texturetiling;
  glm::vec2 texturesize;
  TextureLoadingData loadingInfo;
};

struct ObjectTypeUtil {
  objid id;
  std::function<Mesh(std::string)> createMeshCopy;
  std::map<std::string, MeshRef>& meshes;
  std::function<Texture(std::string)> ensureTextureLoaded;
  std::function<void(int)> releaseTexture;
  std::function<Mesh(MeshData&)> loadMesh;
  std::function<void(std::string, float, float, int, GameobjAttributes&, std::map<std::string, GameobjAttributes>&, std::vector<EmitterDelta>, bool, EmitterDeleteBehavior)> addEmitter;
  std::function<std::vector<std::string>(std::string, bool*)> ensureMeshLoaded;
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

struct AutoSerializeForceString {
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

struct AutoSerializeRotation {
  size_t structOffset;
  const char* field;
  glm::quat defaultValue;
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
  std::function<void(void* offset, void* value)> deserialize;
  std::function<void(void* offset, void* fieldValue)> setAttributes;
  std::function<AttributeValue(void* offset)> getAttribute;
};

struct AutoserializeReservedField {
  const char* field;
  AttributeValueType fieldType;
};

typedef std::variant<AutoSerializeBool, AutoSerializeString, AutoSerializeForceString, AutoSerializeRequiredString, AutoSerializeFloat, AutoSerializeTextureLoaderManual, AutoSerializeInt, AutoSerializeUInt, AutoSerializeVec2, AutoSerializeVec3, AutoSerializeVec4, AutoSerializeRotation, AutoSerializeEnums, AutoSerializeCustom, AutoserializeReservedField> AutoSerialize;
void createAutoSerialize(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attr);
void createAutoSerializeWithTextureLoading(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attr, ObjectTypeUtil& util);
void autoserializerSerialize(char* structAddress, std::vector<AutoSerialize>& values, std::vector<std::pair<std::string, std::string>>& _pairs);
void autoserializerGetAttr(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& _attributes);
void autoserializerSetAttr(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attributes);
void autoserializerSetAttrWithTextureLoading(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attributes, ObjectSetAttribUtil& util);
std::vector<std::string> serializerNames(std::vector<AutoSerialize>& serializers);
std::optional<AutoSerialize> serializerByName(std::vector<AutoSerialize>& serializer, std::string& name);
std::set<std::string> serializerFieldNames(std::vector<AutoSerialize>& serializers);
AttributeValueType typeForSerializer(AutoSerialize& serializer);

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

  autoserializer.push_back(
    AutoSerializeVec2 {
      .structOffset = offsetof(T, texture.texturesize),
      .structOffsetFiller = std::nullopt,
      .field = "texturesize",
      .defaultValue = glm::vec2(1.f, 1.f),
    }
  );

  autoserializer.push_back(
    AutoSerializeTextureLoaderManual {
      .structOffset = offsetof(T, texture.loadingInfo),
      .field = "texture",
      .defaultValue = "",
    }
  );
  
  return 0;
}

std::string printTextureDebug(TextureInformation& info);

#endif