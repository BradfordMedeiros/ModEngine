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
  std::map<std::string, MeshRef>& meshes;
  std::function<Texture(std::string)>& ensureTextureLoaded;
  std::function<Mesh(MeshData&)> loadMesh;
  std::function<void(float, float, int, GameobjAttributes&, std::vector<EmitterDelta>, bool, EmitterDeleteBehavior)> addEmitter;
  std::function<std::vector<std::string>(std::string, std::vector<std::string>)> ensureMeshLoaded;
  std::function<void()> onCollisionChange;
};

struct ObjectSerializeUtil {
  std::function<std::string(int)> textureName;
};

struct ObjectRemoveUtil {
  std::function<void()> rmEmitter;
};

GameObjectUICommon parseCommon(GameobjAttributes& attr, std::map<std::string, MeshRef>& meshes);
void addSerializeCommon(std::vector<std::pair<std::string, std::string>>& pairs, GameObjectUICommon& common);
TextureInformation texinfoFromFields(GameobjAttributes& attr, std::function<Texture(std::string)> ensureTextureLoaded);

#endif