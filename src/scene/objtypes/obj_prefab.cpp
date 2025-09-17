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
  	if (attribute.at(0) == '+'){   // prefabname:+object|attribute:attributevalue
  		auto tokenTarget = attribute.substr(1, attribute.size());
  		
  		auto objectAndAttribute = split(tokenTarget, '|');
  		auto payload = std::get_if<std::string>(&keyAndAttr.attributeValue);
  		modassert(payload != NULL, std::string("invalid type for prefab attr: ") + attribute);
  		modassert(payload -> at(0) == '|', std::string("payload needs | prefix: ") + attribute);
  		auto payloadValue = payload -> substr(1, payload -> size());

  		std::cout << "token target: " << tokenTarget << std::endl;
  		std::cout << "token payloadValue: " << payloadValue << std::endl;
  		
  		addTokens.push_back(Token {
  			.target = objectAndAttribute.at(0),
  			.attribute = objectAndAttribute.at(1),
  			.payload = payloadValue,
  		});
  		
  	}

  }
	return addTokens;
}

GameObjectPrefab createPrefabObj(GameobjAttributes& attr, ObjectTypeUtil& util){
	auto additionalTokens = prefabAdditionalTokens(attr);
	auto scene = getStrAttr(attr, "scene").value();
	auto sceneId = util.loadScene(scene, additionalTokens);
	return GameObjectPrefab {
		.sceneId = sceneId,
	};
}

std::vector<std::pair<std::string, std::string>> serializePrefabObj(GameObjectPrefab& obj, ObjectSerializeUtil& util){
	return {};
}
void removePrefabObj(GameObjectPrefab& prefabObj, ObjectRemoveUtil& util){
	util.unloadScene(prefabObj.sceneId);
}

bool setPrefabAttribute(GameObjectPrefab& obj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&){
  return autoserializerSetAttrWithTextureLoading((char*)&obj, prefabAutoserializer, field, value, util);
}


std::optional<AttributeValuePtr> getPrefabAttribute(GameObjectPrefab& obj, const char* field){
  //modassert(false, "getPrefabAttribute not yet implemented");
  return std::nullopt;
}