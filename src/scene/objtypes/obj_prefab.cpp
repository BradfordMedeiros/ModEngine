#include "./obj_prefab.h"

std::vector<AutoSerialize> prefabAutoserializer {
};

GameObjectPrefab createPrefabObj(GameobjAttributes& attr, ObjectTypeUtil& util){
	return GameObjectPrefab {};
}
void prefabObjAttr(GameObjectPrefab& soundObj, GameobjAttributes& _attributes){

}
std::vector<std::pair<std::string, std::string>> serializePrefabObj(GameObjectPrefab& obj, ObjectSerializeUtil& util){
	return {};
}
void removePrefabObj(GameObjectPrefab& soundObj, ObjectRemoveUtil& util){

}
bool setPrefabAttributes(GameObjectPrefab& soundObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
	return false;
}