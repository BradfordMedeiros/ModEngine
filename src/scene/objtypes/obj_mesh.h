#ifndef MOD_OBJ_MESH
#define MOD_OBJ_MESH

#include "../../common/util.h"
#include "./obj_util.h"
#include "../serialization.h"

struct GameObjectMesh {
  std::optional<std::string> rootMesh;
  std::vector<std::string> meshNames;
  std::vector<Mesh> meshesToRender;   // @TODO  I shouldn't be storing the actual mesh here.  Instead I should just be referencing global meshes
  std::vector<std::vector<objid>> boneGameObjIdCache;  // TODO - i look these up in the main loop...it might be nice to do during load instead

  bool isDisabled;
  TextureInformation texture;
  TextureLoadingData normalTexture;

  float discardAmount;
  glm::vec3 emissionAmount;
  glm::vec4 tint;
  objid rootidCache;

  bool hasBones;
  glm::mat4 bonesGroupModel;
};

GameObjectMesh createMesh(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeMesh(GameObjectMesh obj, ObjectSerializeUtil& util);
std::optional<AttributeValuePtr> getMeshAttribute(GameObjectMesh& obj, const char* field);
bool setMeshAttribute(GameObjectMesh& meshObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&);

#endif