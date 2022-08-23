#ifndef MOD_OBJ_MESH
#define MOD_OBJ_MESH

#include "../../common/util.h"
#include "./obj_util.h"
#include "../serialization.h"

struct GameObjectMesh {
  std::vector<std::string> meshNames;
  std::vector<Mesh> meshesToRender;   // @TODO  I shouldn't be storing the actual mesh here.  Instead I should just be referencing global meshes
  bool isDisabled;
  bool nodeOnly;
  std::string rootMesh;
  TextureInformation texture;
  float discardAmount;
  float emissionAmount;
  glm::vec4 tint;
  bool isRoot;
};

GameObjectMesh createMesh(GameobjAttributes& attr, ObjectTypeUtil& util);
std::vector<std::pair<std::string, std::string>> serializeMesh(GameObjectMesh obj, ObjectSerializeUtil& util);
void meshObjAttr(GameObjectMesh& meshObj, GameobjAttributes& _attributes);
bool setMeshAttributes(GameObjectMesh& meshObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

#endif