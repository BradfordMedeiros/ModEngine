#include "./obj_prefab.h"

std::vector<AutoSerialize> prefabAutoserializer {
};

std::vector<Token> prefabAdditionalTokens(GameobjAttributes& attributes){
	// allKeysAndAttributes
	std::vector<Token> addTokens;
	auto allKeysAndAttr = allKeysAndAttributes(attributes);
	// std::string attribute;
  // AttributeValue payload;
  for (auto &keyAndAttr : allKeysAndAttr){
  	auto attribute = keyAndAttr.field;
  	if (attribute.at(0) == '+'){
  		auto tokenTarget = attribute.substr(1, attribute.size());
  		auto tokenPayload = keyAndAttr.attributeValue;
  		auto payload = std::get_if<std::string>(&tokenPayload);
  		modassert(payload != NULL, "prefab attribute not string value");
  		auto values = split(*payload, ':');
  		modassert(values.size() == 2, std::string("invalid additive prefab attribute: " + *payload));
  		addTokens.push_back(Token {
  			.target = tokenTarget,
  			.attribute = values.at(0),
  			.payload = values.at(1),
  		});
  	}

  }
	return addTokens;
}

GameObjectPrefab createPrefabObj(GameobjAttributes& attr, ObjectTypeUtil& util){
	auto additionalTokens = prefabAdditionalTokens(attr);
	auto sceneId = util.loadScene(attr.stringAttributes.at("scene"), additionalTokens);
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

bool setPrefabAttribute(GameObjectPrefab& obj, const char* field, AttributeValue value, ObjectSetAttribUtil& util){
  return autoserializerSetAttrWithTextureLoading((char*)&obj, prefabAutoserializer, field, value, util);
}


std::optional<AttributeValuePtr> getPrefabAttribute(GameObjectPrefab& obj, const char* field){
  //modassert(false, "getPrefabAttribute not yet implemented");
  return std::nullopt;
}