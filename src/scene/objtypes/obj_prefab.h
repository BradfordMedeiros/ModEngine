#ifndef MOD_OBJ_PREFAB
#define MOD_OBJ_PREFAB

#include "./obj_util.h"

struct GameObjectPrefab{
	objid sceneId;
};

GameObjectPrefab createPrefabObj(GameobjAttributes& attr, ObjectTypeUtil& util);
void prefabObjAttr(GameObjectPrefab& soundObj, GameobjAttributes& _attributes);
std::vector<std::pair<std::string, std::string>> serializePrefabObj(GameObjectPrefab& obj, ObjectSerializeUtil& util);
void removePrefabObj(GameObjectPrefab& soundObj, ObjectRemoveUtil& util);
bool setPrefabAttribute(GameObjectPrefab& obj, const char* field, AttributeValue value, ObjectSetAttribUtil& util);
std::optional<AttributeValuePtr> getPrefabAttribute(GameObjectPrefab& obj, const char* field);

#endif