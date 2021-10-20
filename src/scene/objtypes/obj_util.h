#ifndef MOD_OBJ_UTIL
#define MOD_OBJ_UTIL

#include <map>
#include "../common/mesh.h"

struct GameObjectUICommon {
  Mesh mesh;
  bool isFocused;
  std::string onFocus;
  std::string onBlur;
};

struct ObjectTypeUtil {
  std::map<std::string, MeshRef>& meshes;
  std::function<Texture(std::string)>& ensureTextureLoaded;
};

GameObjectUICommon parseCommon(GameobjAttributes& attr, std::map<std::string, MeshRef>& meshes);
void addSerializeCommon(std::vector<std::pair<std::string, std::string>>& pairs, GameObjectUICommon& common);

#endif