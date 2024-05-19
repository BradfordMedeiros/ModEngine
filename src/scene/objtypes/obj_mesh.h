#ifndef MOD_OBJ_MESH
#define MOD_OBJ_MESH

#include "../../common/util.h"
#include "./obj_util.h"
#include "../serialization.h"

struct GameObjectMesh {
  std::vector<std::string> meshNames;
  std::vector<Mesh> meshesToRender;   // @TODO  I shouldn't be storing the actual mesh here.  Instead I should just be referencing global meshes
  bool isDisabled;
  TextureInformation texture;
  TextureLoadingData normalTexture;
  float discardAmount;
  float emissionAmount;
  glm::vec4 tint;
};

GameObjectMesh createMesh(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeMesh(GameObjectMesh obj, ObjectSerializeUtil& util);
std::optional<AttributeValuePtr> getMeshAttribute(GameObjectMesh& obj, const char* field);
bool setMeshAttribute(GameObjectMesh& meshObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&);

#endif