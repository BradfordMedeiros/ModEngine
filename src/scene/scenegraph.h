#ifndef MOD_SCENEGRAPH
#define MOD_SCENEGRAPH

#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <functional>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include "./common/mesh.h"
#include "../common/util.h"
#include "./serialization.h"

struct GameObject {
  objid id;
  std::string name;
  Transformation transformation;
  physicsOpts physicsOptions;
  std::string lookat;
  std::string layer;
  std::string script;
  std::string fragshader;
  bool netsynchronize;
  glm::vec3 tint;
};
struct GameObjectH {
  objid id;
  objid parentId;
  std::set<objid> children;
  objid groupId;       // grouping mechanism for nodes.  It is either its own id, or explicitly stated when created. 
  bool linkOnly;
};

struct Scene {
  objid rootId;
  bool isNested;
  std::map<objid, GameObject> idToGameObjects;
  std::map<objid, GameObjectH> idToGameObjectsH;
  std::map<std::string, objid> nameToId;
  std::vector<LayerInfo> layers;
};

struct SceneDeserialization {
  Scene scene;
  std::map<std::string, SerializationObject> serialObjs;
};

std::string serializeObject(Scene& scene, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds, objid id, std::string name = "");
std::string serializeScene(Scene& scene, std::function<std::vector<std::pair<std::string, std::string>>(objid)> getAdditionalFields, bool includeIds);
SceneDeserialization deserializeScene(
  std::string content, 
  std::vector<Field> fields,
  std::function<objid()> getNewObjectId
);

SerializationObject serialObjectFromFields(
  std::string layer,
  std::vector<Field> fields,
  GameobjAttributes attributes
);

void addSerialObjectToScene(Scene& scene, std::string name, SerializationObject& serialObj, std::function<objid()> getNewObjectId);
void addChildLink(Scene& scene, objid childId, objid parentId);

std::map<std::string, SerializationObject> addSubsceneToRoot(
  Scene& scene, 
  objid rootId,
  objid rootIdNode, 
  std::map<objid, objid> childToParent, 
  std::map<objid, Transformation> gameobjTransforms, 
  std::map<objid, std::string> names, 
  std::map<objid, std::map<std::string, std::string>> additionalFields,
  std::function<objid()> getNewObjectId,
  glm::vec3 parentTint
);

std::vector<objid> idsToRemoveFromScenegraph(Scene& scene, objid);
void removeObjectsFromScenegraph(Scene& scene, std::vector<objid> objects);
std::vector<objid> listObjInScene(Scene& scene);

void traverseScene(Scene& scene, glm::mat4 initialModel, glm::vec3 totalScale, std::function<void(objid, glm::mat4, glm::mat4, bool, std::string, glm::vec3)> onObject, std::function<void(objid, glm::mat4, glm::vec3)> traverseLink);  

Transformation getTransformationFromMatrix(glm::mat4 matrix);

std::vector<objid> getIdsInGroup(Scene& scene, objid groupId);

std::map<std::string, std::string> scenegraphAttributes(Scene& scene, objid id);
void setScenegraphAttributes(Scene& scene, objid id, std::map<std::string, std::string> attributes);


std::map<std::string, SerializationObject> deserializeSceneTokens(std::vector<Token> tokens);
SerializationObject getDefaultObject(std::string layer);

#endif

