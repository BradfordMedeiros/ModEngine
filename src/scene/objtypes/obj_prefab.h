#ifndef MOD_OBJ_PREFAB
#define MOD_OBJ_PREFAB

#include "./obj_util.h"

struct GameObjectPrefab{};

GameObjectPrefab createPrefabObj(GameobjAttributes& attr, ObjectTypeUtil& util);
void prefabObjAttr(GameObjectPrefab& soundObj, GameobjAttributes& _attributes);
std::vector<std::pair<std::string, std::string>> serializePrefabObj(GameObjectPrefab& obj, ObjectSerializeUtil& util);
void removePrefabObj(GameObjectPrefab& soundObj, ObjectRemoveUtil& util);
bool setPrefabAttributes(GameObjectPrefab& soundObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util);

#endif