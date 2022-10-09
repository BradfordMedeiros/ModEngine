#include "./obj_prefab.h"

std::vector<AutoSerialize> prefabAutoserializer {
};

GameObjectPrefab createPrefabObj(GameobjAttributes& attr, ObjectTypeUtil& util){
	auto sceneId = util.loadScene(attr.stringAttributes.at("scene"));
	return GameObjectPrefab {
		.sceneId = sceneId,
	};
}
void prefabObjAttr(GameObjectPrefab& prefabObj, GameobjAttributes& _attributes){

}
std::vector<std::pair<std::string, std::string>> serializePrefabObj(GameObjectPrefab& obj, ObjectSerializeUtil& util){
	return {};
}
void removePrefabObj(GameObjectPrefab& prefabObj, ObjectRemoveUtil& util){
	util.unloadScene(prefabObj.sceneId);
}
bool setPrefabAttributes(GameObjectPrefab& prefabObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
	return false;
}