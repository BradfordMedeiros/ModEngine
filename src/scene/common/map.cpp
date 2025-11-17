#include "./map.h"

std::string readFileOrPackage(std::string filepath);

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
	return BrushFace{
		.point1 = glm::vec3(1.f, 0.f, 0.f),
    	.point2 = glm::vec3(2.f, 0.f, 0.f),
    	.point3 = glm::vec3(3.f, 0.f, 0.f),
    	.uvOffset = glm::vec2(1.f, 2.f),
    	.textureScale = glm::vec2(3.f, 4.f),
    	.rotation = 45.f,
    	.texture = "testtexture",
	};
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

MapData parseMapData(std::string filepath){
	auto content = readFileOrPackage(filepath);

	auto contentNoComments = stripComments(content);
	//std::cout << "stripped comments: \n" << contentNoComments << std::endl;

	std::vector<Entity> entities;
	auto rawEntitiesContents = rawEntities(contentNoComments);
	for (auto& rawEntity : rawEntitiesContents){
		Entity entity{
			.layerId = 0,
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

	//std::cout << "rawEntities: \n" << print(rawEntities(stripComments(content))) << std::endl; 
	return MapData{};
}