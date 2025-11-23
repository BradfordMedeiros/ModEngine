#include "./map.h"

std::string readFileOrPackage(std::string filepath);
std::string serializeAttributeValue(AttributeValue& value);

struct ParsedEntity {
	std::vector<EntityKeyValue> keyValues;
	std::vector<Brush> brushes;
};

std::string stripComments(std::string& content){
	std::string newContent;
	bool inCommentMode = false;
	for (int i = 0; i < content.size(); i++){
		char currentChar = content.at(i);

		if (inCommentMode){
			if (currentChar == '\n'){
				inCommentMode = false;
				continue;
			}
			continue;
		}

		if (currentChar == '/' && content.at(i + 1) == '/'){
			i++;
			inCommentMode = true;
			continue;
		}
		newContent += currentChar;
	}
	return newContent;
}

struct RawEntity {
	std::string keyValueData;
	std::vector<std::string> brushData;
};

std::vector<RawEntity> rawEntities(std::string& content){
	int numLeftBraces = 0;
	std::string currentEntityContent;
	std::string currentBrushContent;
	std::vector<RawEntity> rawEntitys;
	RawEntity rawEntity;

	bool justAddedElement = false;
	for (int i = 0; i < content.size(); i++){
		char currentChar = content.at(i);
		if (currentChar == '{'){
			numLeftBraces++;
			continue;
		}

		if (numLeftBraces > 0 && currentChar == '}'){
			if (numLeftBraces == 1){
				rawEntity.keyValueData = currentEntityContent;
				rawEntitys.push_back(rawEntity);
				rawEntity = RawEntity{};
				justAddedElement = true;
				currentEntityContent = "";
				numLeftBraces--;
				continue;
			}else if (numLeftBraces == 2){
				rawEntity.brushData.push_back(currentBrushContent);
				currentBrushContent = "";
				justAddedElement = true;
				numLeftBraces--;
				continue;
			}else{
				std::cout << "unexpected brace count 1" << numLeftBraces << std::endl;
				exit(1);
			}
		}

		bool addedElementLastFrame = justAddedElement;
		justAddedElement = false;
 
		if (numLeftBraces == 0){
			// do nothing, probably whitespace
		}else if (numLeftBraces == 1){
			if (!(addedElementLastFrame && (currentChar != ' ' || currentChar != '\n'))){
				currentEntityContent += currentChar;	
			}
		}else if (numLeftBraces == 2){
			if (!(addedElementLastFrame && (currentChar == ' ' || currentChar != '\n'))){
				currentBrushContent += currentChar;
			}
		}else {
			std::cout << "unexpected brace count 2: " << numLeftBraces << std::endl;
			exit(1);
		}
	}
	return rawEntitys;
}

std::vector<EntityKeyValue> parseKeyValues(std::string& content){
	std::vector<EntityKeyValue> keyValues;
	auto keyValueStrs = split(content, '\n');
	for (auto& keyValueStr : keyValueStrs){
		if (keyValueStr != " " && keyValueStr != ""){
			bool inContent = false;
			bool addedKey = false;
			bool addedValue = false;
			std::string content;
			EntityKeyValue entityKeyValue;
			for (int i = 0; i < keyValueStr.size(); i++){
			    auto value = keyValueStr.at(i);
				if (value == '"'){
					if (inContent){
						if (!addedKey){
							entityKeyValue.key = content;
							content = "";
							addedKey = true;
						}else{
							// maybe i should parse the type here
							entityKeyValue.value = content;
							addedValue = true;
							break;
						}
					}	
					inContent = !inContent;
				}else{
					if (inContent){
						content += value;
					}
				}
			}
			modassert(addedKey && addedValue, "did not have a key and value");
			keyValues.push_back(entityKeyValue);
		}
	}
	return keyValues;
}

BrushFace parseBrushFace(std::string& content){
	std::cout << "brush face: " << content << std::endl;

	bool foundLeftBrace = false;
	std::string stringValue;

	int count = 0;

	bool valid = true;

	std::vector<glm::vec3> points;
	for (int i = 0; i < content.size(); i++){
		if (count == 3){
			stringValue += content.at(i);
			continue;
		}
		auto value = content.at(i);
		if (value == '('){
			foundLeftBrace = true;
		}else if (value == ')'){
			foundLeftBrace = false;
			std::cout << "brush face value is: [" << stringValue << "]" << std::endl;
			glm::vec3 vec = parseVec(stringValue);
			points.push_back(vec);
			stringValue = "";
			count++;
		}else if (foundLeftBrace){
			stringValue += value;
		}else if (value == ' '){
		}else{
			valid = false;
		}

		if (!valid){
			std::cout << "brush face unexpected value: [" << value << "]" << std::endl;
			exit(1);
		}
	}

	auto values = filterWhitespace(split(stringValue, ' '));

	BrushFace brushFace {};
	brushFace.point1 = points.at(0);
	brushFace.point2 = points.at(1);
	brushFace.point3 = points.at(2);


	// TODO store this in a double for now
	brushFace.uvOffset = glm::vec2(std::atof(values.at(1).c_str()), std::atof(values.at(2).c_str()));

	brushFace.textureScale = glm::vec2(std::atof(values.at(4).c_str()), std::atof(values.at(5).c_str()));
	brushFace.rotation = std::atof(values.at(3).c_str());

	auto texture = values.at(0);
	brushFace.texture = texture;

	std::cout << "brush face rest: [" << print(values.at(0)) << "]" << std::endl;
	return brushFace;
}

std::vector<BrushFace> parseBrushFaces(std::string& rawBrushFaces){
	std::vector<BrushFace> brushFaces;
	//std::cout << "==? raw brush content = " << rawBrushFaces << std::endl;
	std::vector<std::string> values;
	std::string content;
	for (int i = 0; i < rawBrushFaces.size(); i++){
		auto value = rawBrushFaces.at(i);
		if (value == '\n'){
			values.push_back(content);
			content = "";
		}else{
			content += value;
		}
	}

	for (auto& value : values){
		if (!(value.size() == 0 || (value.size() == 1 && value.at(1) == ' '))){
			//std::cout << "||brushface = " << value << std::endl;
			brushFaces.push_back(parseBrushFace(value));
		}
	}

	return brushFaces;
}

std::vector<Brush> parseBrush(std::vector<std::string> rawBrushes){
	std::vector<Brush> brushes;
	for (auto& rawBrushFaces : rawBrushes){ 
		std::vector<BrushFace> brushFaces = parseBrushFaces(rawBrushFaces);
		brushes.push_back(Brush {
			.brushFaces = brushFaces,
		});
	}
	return brushes;
}

std::string print(BrushFace& brushFace){
	return print(brushFace.point1) + "|" + print(brushFace.point2) + "|" + print(brushFace.point3) + "|" + print(brushFace.uvOffset) + "|" + print(brushFace.textureScale) + "|" + std::to_string(brushFace.rotation) + "|" + brushFace.texture;
}


std::optional<std::string*> getKeyValue(std::vector<EntityKeyValue>& keyValues, const char* name){
	for (auto& entityKeyValue : keyValues){
		if (entityKeyValue.key == name){
			return &entityKeyValue.value;
		}
	}
	return std::nullopt;
}

bool isLayerEntity(Entity& entity, int* layerId){
	*layerId = 0;

	auto type = getKeyValue(entity.keyValues, "_tb_type"); // _tb_layer
	auto sortIndex = getKeyValue(entity.keyValues, "_tb_layer_sort_index"); // _tb_name

	bool isLayer = false;
	if (type.has_value() && *(type.value()) == "_tb_layer"){
		auto name = getKeyValue(entity.keyValues, "_tb_name"); // _tb_name
		auto layerId = getKeyValue(entity.keyValues, "_tb_id");
		std::cout << "entity is a layer: " << *(name.value()) << ", id = " << *(layerId.value()) << std::endl;
		isLayer = true;
	}

	return isLayer;
}

MapData parseMapData(std::string filepath){
	auto content = readFileOrPackage(filepath);

	auto contentNoComments = stripComments(content);
	//std::cout << "stripped comments: \n" << contentNoComments << std::endl;

	std::vector<Entity> entities;
	auto rawEntitiesContents = rawEntities(contentNoComments);
	for (int i = 0; i < rawEntitiesContents.size(); i++){
		auto& rawEntity = rawEntitiesContents.at(i);
		Entity entity{
			.layerId = 0,
			.index = i,
		};
		auto keyValues = parseKeyValues(rawEntity.keyValueData);
		entity.keyValues = keyValues;

		auto brushes = parseBrush(rawEntity.brushData);
		entity.brushes = brushes;
		entities.push_back(entity);
	}

	std::cout << "entities length: " << entities.size() << std::endl;
	for (int i = 0; i < entities.size(); i++){
		auto& entity = entities.at(i);
		std::cout << "|entity index = " << i << std::endl;
		for (auto &keyAndValue : entity.keyValues){
			std::cout << "||entity key = [" << keyAndValue.key << "] , value = [" << keyAndValue.value << "]" << std::endl;
		}
		for (int j = 0; j < entity.brushes.size(); j++){
			std::cout << "||entity brush = " << j << std::endl;
			auto& brush = entity.brushes.at(j);
			for (auto k = 0; k < brush.brushFaces.size(); k++){
				std::cout << "|||entity brush face = " << k << ", " << print(brush.brushFaces.at(k)) << std::endl;
			}
		}
	}

	MapData mapData {
		.layers = {},
		.entities = {},
	};

	for (auto& entity : entities){
		int layerId = 0;
		auto isLayer = isLayerEntity(entity, &layerId);
		entity.layerId = layerId;
		if (isLayer){
			mapData.layers.push_back(entity);
		}else{
			mapData.entities.push_back(entity);
		}
	}



	//std::cout << "rawEntities: \n" << print(rawEntities(stripComments(content))) << std::endl; 
	return mapData;
}

std::vector<Entity*> getEntitiesByClassName(MapData& mapData, const char* name){
	std::vector<Entity*> entities;

	for (auto& entity: mapData.layers){
		auto value = getKeyValue(entity.keyValues, "classname");
		modassert(value.has_value(), "classname does not exist");
		if (*(value.value()) == name){
			entities.push_back(&entity);
		}
	}
	for (auto& entity : mapData.entities){
		auto value = getKeyValue(entity.keyValues, "classname");
		modassert(value.has_value(), "classname does not exist");
		if (*(value.value()) == name){
			entities.push_back(&entity);
		}
	}

	return entities;
}

std::optional<std::string*> getValue(Entity& entity, const char* key){
	return getKeyValue(entity.keyValues, key);
}



std::string getEntityName(std::string& baseName, std::optional<std::string>& submodel){
	if (submodel.has_value()){
		return baseName + "/" + submodel.value();
	}
	return baseName;
}

void compileRawScene(std::string filepath, std::string baseFile,  std::string mapFile,  std::function<void(Entity& entity, bool* _shouldWrite, std::vector<GameobjAttributeOpts>& _attributes)> callback){
	std::string content = "########## Base file content: " + baseFile + " ##########\n\n" + readFileOrPackage(baseFile) + "\n\n";

	std::string generatedContent = "##########  Generated content: + " + mapFile + "\n\n";

	std::string generatedScene;

	auto mapData = parseMapData(mapFile);
	for (auto& entity: mapData.entities){
		bool shouldWrite = false;
		std::vector<GameobjAttributeOpts> attributes;
		callback(entity, &shouldWrite, attributes);

		bool userSpecifiedPosition = false;
		if (shouldWrite){
		    auto origin = getValue(entity, "origin");
		    auto classname = getValue(entity, "classname");

		    glm::vec3 position = origin.has_value() ? parseVec(*origin.value()) : glm::vec3(0.f, 0.f, 0.f);
		    modassert(classname.has_value(), "no classname");

		    std::string entityName = std::string("entity_") + *classname.value() + "_" + std::to_string(entity.index);

		    // I could check uniqueness here
		    for (auto& attribute : attributes){
		    	// probably wrap this is a function to illegal characters better
		    	if (attribute.field == "position" && !attribute.submodel.has_value()){
		    		userSpecifiedPosition = true;
		    	}
  				generatedScene += getEntityName(entityName, attribute.submodel) + ":" + attribute.field + ":" + serializeAttributeValue(attribute.attributeValue) + "\n";
		    }

		    if (!userSpecifiedPosition){
				generatedScene += entityName + ":position:" + serializeVec(position) + "\n\n";
		    }else{
				generatedScene += "\n";
		    }
		}
	}

	realfiles::saveFile(filepath, content + generatedContent + generatedScene);
}