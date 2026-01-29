#include "./map.h"

std::string readFileOrPackage(std::string filepath);
std::string serializeAttributeValue(AttributeValue& value);
void setDebugPoints(std::vector<glm::vec3> points, std::vector<std::optional<glm::vec3>> pointsTo, std::vector<glm::vec3> colors);
MeshData generateMeshRaw(std::vector<glm::vec3>& verts, std::vector<glm::vec2>& uvCoords, std::vector<unsigned int>& indexs, std::string* texture, std::string* normalTexture);
std::optional<std::string> lookupNormalTexture(std::string textureName);
TextureSizeInfo loadTextureSizeInfo(std::string textureFilePath);
bool fileExistsFromPackage(std::string filepath);

std::string textureForMapTextureName(std::string mapTexture){
	auto jpgImage = std::string("../gameresources/textures/") + mapTexture + ".jpg";
	auto pngImage = std::string("../gameresources/textures/") + mapTexture + ".png";

	if (fileExistsFromPackage(jpgImage)){
		return jpgImage;
	}
	if (fileExistsFromPackage(pngImage)){
		return pngImage;
	}
	modassert(false, std::string("textureForMapTextureName missing image: ") + mapTexture);
	return "";
}

const float EPSILON = 0.001f;

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

glm::vec3 parseVecTrenchbroom(std::string positionRaw, float scale){;
  float x, y, z;
  std::istringstream in(positionRaw);
  in >> x >> y >> z;
  return glm::vec3(x / scale, z / scale, (-1 * y) / scale);
}

float parseFloatTrenchbroom(std::string& value){
	return std::atof(value.c_str());
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


	modassert(values.size() == 6, std::string("map parsing, unexpected number of values: ") + std::to_string(values.size()));
	std::cout << "calculate: values: " << print(values) << std::endl;
	// TODO store this in a double for now
	brushFace.uvOffset = glm::vec2(std::atof(values.at(1).c_str()), std::atof(values.at(2).c_str()));

	brushFace.textureScale = glm::vec2(std::atof(values.at(4).c_str()), std::atof(values.at(5).c_str()));
	std::cout << "calculate brush scale: " << print(brushFace.textureScale) << std::endl;
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

MapData parseRawMapData(std::string& content){
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


	float scale = 10.f;
	for (auto& entity : entities){
		auto value = getKeyValue(entity.keyValues, "classname");
		if (value.has_value() && *value.value() == "worldspawn"){
			auto value = getKeyValue(entity.keyValues, "worldscale");
			if (value.has_value()){
				auto worldspawnScale = parseFloatTrenchbroom(*value.value());
				scale = worldspawnScale;
			}
		}
	}

	for (auto &entity : entities){
		for (auto& brush : entity.brushes){
			for (auto& brushFace : brush.brushFaces){
				brushFace.point1 /= scale;
				brushFace.point2 /= scale;
				brushFace.point3 /= scale;
			}
		}
	}

	MapData mapData {
		.scale = scale,
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

MapData parseMapData(std::string filepath){
  auto content = readFileOrPackage(filepath);
  return parseRawMapData(content);
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

std::optional<int> getIntValue(Entity& entity, const char* key){
	auto value = getKeyValue(entity.keyValues, key);
	if (!value.has_value()){
		return std::nullopt;
	}

	return std::atoi(value.value() -> c_str());
}

std::optional<glm::vec3> getScaledVec3Value(MapData& mapData, Entity& entity, const char* key){
	auto value = getKeyValue(entity.keyValues, key);
	if (!value.has_value()){
		return std::nullopt;
	}

	glm::vec3 position = parseVecTrenchbroom(*value.value(), mapData.scale);
	return position;
}

std::optional<glm::vec3> getUnitVec3Value(MapData& mapData, Entity& entity, const char* key){
	auto value = getKeyValue(entity.keyValues, key);
	if (!value.has_value()){
		return std::nullopt;
	}

	glm::vec3 position = parseVecTrenchbroom(*value.value(), 1.f);
	return position;
}

std::optional<glm::vec3> getVec3Value(MapData& mapData, Entity& entity, const char* key){
	auto value = getKeyValue(entity.keyValues, key);
	if (!value.has_value()){
		return std::nullopt;
	}

	glm::vec3 position = parseVec(*value.value());
	return position;
}


std::string getEntityName(std::string& baseName, std::optional<std::string>& submodel){
	if (submodel.has_value()){
		return baseName + "/" + submodel.value();
	}
	return baseName;
}

void compileBrushes(MapData& mapData, std::string& rawMapData, std::string path){
	realfiles::saveFile(path, rawMapData);
}


void compileRawScene(std::string filepath, std::string baseFile,  std::string mapFile, std::string brushFileOut, std::function<void(MapData& mapData, Entity& entity, bool* _shouldWrite, std::vector<GameobjAttributeOpts>& _attributes, std::string* _modelName)> callback, std::function<void(MapData& mapData, std::string&)> afterEntities){
	std::string content = "########## Base file content: " + baseFile + " ##########\n\n" + readFileOrPackage(baseFile) + "\n\n";

	std::string generatedContent = "##########  Generated content: + " + mapFile + "\n\n";

	std::string generatedScene;

	auto rawMapContent = readFileOrPackage(mapFile);

	auto mapData = parseMapData(mapFile);
	for (auto& entity: mapData.entities){
		bool shouldWrite = false;
		std::string modelName = "";
		std::vector<GameobjAttributeOpts> attributes;
		callback(mapData, entity, &shouldWrite, attributes, &modelName);

		bool userSpecifiedPosition = false;
		if (shouldWrite){
		    auto classname = getValue(entity, "classname");
		    modassert(classname.has_value(), "no classname");

		    auto origin = getValue(entity, "origin");
		    glm::vec3 position = origin.has_value() ? parseVecTrenchbroom(*origin.value(), mapData.scale) : glm::vec3(0.f, 0.f, 0.f);

		    std::string entityName = modelName != "" ?  modelName : (std::string("entity_") + *classname.value() + "_" + std::to_string(entity.index));

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

	auto sceneFileData = content + generatedContent + generatedScene + "\n";
	afterEntities(mapData, sceneFileData);
	realfiles::saveFile(filepath, sceneFileData);

	compileBrushes(mapData, rawMapContent, brushFileOut);
}

void addPointsToSimpleMesh(std::vector<glm::vec3>& points, std::vector<unsigned int>& _indexs){
  for (int i = 0; i < points.size(); i++){
    _indexs.push_back(i);
  }
}


struct BrushPlane {
	float distanceToPoint;
	glm::vec3 normal;
	glm::vec3 pointOnPlane;
	glm::vec3 pointOnPlane2;
	glm::vec3 pointOnPlane3;
};

BrushPlane brushFaceToPlane(BrushFace& brushFace){
	glm::vec3 normal = glm::normalize(glm::cross(brushFace.point2 - brushFace.point1, brushFace.point3 - brushFace.point1));
	float distanceToPoint = -1 * glm::dot(normal, brushFace.point1);
	
	std::cout << "distanceToPoint: " << distanceToPoint << std::endl;
	//if (distanceToPoint > 0.f){
	//	// This means trenchbroom flipped a normal on us, it is inward facing, let's just reverse it
	//	normal = -1.f * normal;
	//	distanceToPoint *= -1;
	//	//modassert(distanceToPoint <= 0.f, std::string("unexpected value brushFaceToPlane: ") + std::to_string(distanceToPoint));
	//}


	BrushPlane plane {
		.distanceToPoint = distanceToPoint,
		.normal = normal,
		.pointOnPlane = brushFace.point1,
		.pointOnPlane2 = brushFace.point2,
		.pointOnPlane3 = brushFace.point3,
	};
	return plane;
}

std::optional<glm::vec3> intersectPlanes(BrushPlane& a, BrushPlane& b, BrushPlane& c){
    glm::vec3 n1n2 = glm::cross(a.normal, b.normal);
    glm::vec3 n2n3 = glm::cross(b.normal, c.normal);
    glm::vec3 n3n1 = glm::cross(c.normal, a.normal);

    float denom = glm::dot(a.normal, n2n3);
    if (fabs(denom) < EPSILON){
    	return std::nullopt; // planes are parallel or degenerate
    } 

    return ( (-a.distanceToPoint * n2n3) - (b.distanceToPoint * n3n1) - (c.distanceToPoint * n1n2) ) / denom;
}



bool insideBrushPlanes(std::vector<BrushPlane>& brushPlanes, glm::vec3 point){
	std::cout << "\ninsideBrushPlanes" << std::endl;
	for (auto& plane : brushPlanes){
    float projection = glm::dot(plane.normal, point - plane.pointOnPlane);
		bool insidePlane = projection >= (-1 * EPSILON);

		std::cout << "insideBrushPlanes: plane point = " << print(plane.pointOnPlane) << " normal = " << print(plane.normal) << ", distanceToPoint = " << plane.distanceToPoint << ", point = " << print(point) + ",dp = " << projection << ", insidePlane = " << insidePlane << std::endl;
    	if (!insidePlane) {
    		return false;
    	}
	}
	return true;
}

std::vector<glm::vec3> getAllIntersections(Brush& brush, bool includeAll, std::vector<bool>& isInsidePlanes){
	std::vector<BrushPlane> brushPlanes;
	for (auto& brushFace : brush.brushFaces){
		auto brushPlane = brushFaceToPlane(brushFace);
		brushPlanes.push_back(brushPlane);
	}

	std::vector<glm::vec3> candidateVertices;
	for (int i = 0; i < brushPlanes.size(); i++){
		for (int j = (i + 1); j < brushPlanes.size(); j++){
			for (int k = (j + 1); k < brushPlanes.size(); k++){
				auto intersection = intersectPlanes(brushPlanes.at(i), brushPlanes.at(j), brushPlanes.at(k));
				if (intersection.has_value()){
					auto insidePlane = insideBrushPlanes(brushPlanes, intersection.value());
					if (includeAll){
						candidateVertices.push_back(intersection.value());
						isInsidePlanes.push_back(insidePlane);;
					}else{
						if (insidePlane){
							candidateVertices.push_back(intersection.value());
						}
					}
				}
			}
		}
	}

	return candidateVertices;
}

struct VertexWithData {
	glm::vec3 pos;
	glm::vec2 uv;
};

void sortVerticesCCW(std::vector<VertexWithData>& vertices, const glm::vec3& normal) {
	if (vertices.size() == 0){
		return;
	}
  glm::vec3 centroid(0.f, 0.f, 0.f);
   {
   	for (auto& vertex : vertices) {
   		centroid += vertex.pos;
   	};
   	centroid /= (float)vertices.size();
  }

  // Pick a reference axis on the plane
 	glm::vec3 ref = glm::normalize(vertices.at(0).pos - centroid);

  // Sort by angle around normal
  std::sort(vertices.begin(), vertices.end(), [&](const VertexWithData& vertexOne, const VertexWithData& vertexTwo) {
      glm::vec3 va = glm::normalize(vertexOne.pos - centroid);
      glm::vec3 vb = glm::normalize(vertexTwo.pos - centroid);
      float angle = glm::atan(glm::dot(glm::cross(ref, va), normal), glm::dot(ref, va));
      float angleB = glm::atan(glm::dot(glm::cross(ref, vb), normal), glm::dot(ref, vb));
      return angle < angleB;
  });
}

static const glm::vec3 quakeBaseAxes[6][3] = {
    {{ 0,  0,  1}, { 1,  0,  0}, { 0, -1,  0}}, // floor
    {{ 0,  0, -1}, { 1,  0,  0}, { 0, -1,  0}}, // ceiling
    {{ 1,  0,  0}, { 0,  1,  0}, { 0,  0, -1}}, // east
    {{-1,  0,  0}, { 0,  1,  0}, { 0,  0, -1}}, // west
    {{ 0,  1,  0}, { 1,  0,  0}, { 0,  0, -1}}, // north
    {{ 0, -1,  0}, { 1,  0,  0}, { 0,  0, -1}}  // south
};

int chooseQuakeAxis(const glm::vec3& n)
{
    float ax = fabs(n.x);
    float ay = fabs(n.y);
    float az = fabs(n.z);

    if (az >= ax && az >= ay)
        return (n.z > 0) ? 0 : 1;
    if (ax >= ay)
        return (n.x > 0) ? 2 : 3;
    return (n.y > 0) ? 4 : 5;
}

void buildQuakeTexAxes(
    glm::vec3& faceNormal,
    float rotationDegrees,
    float scaleX,
    float scaleY,
    glm::vec3* _U,
    glm::vec3* _V
)
{
    int axis = chooseQuakeAxis(faceNormal);

    glm::vec3 U = quakeBaseAxes[axis][1];
    glm::vec3 V = quakeBaseAxes[axis][2];

    // --- rotation (2D in-plane, NOT generic 3D) ---
    float ang = glm::radians(rotationDegrees);
    float s = sinf(ang);
    float c = cosf(ang);

    glm::vec3 U2 =  U * c - V * s;
    glm::vec3 V2 =  U * s + V * c;

    U = U2;
    V = V2;

    U /= scaleX;
    V /= scaleY;

    *_U = U;
    *_V = V;
}


glm::vec2 calculateUv(BrushFace& brushFace, BrushPlane& brushPlane,  glm::vec3 vertex, float textureWidth, float textureHeight) {
   	glm::vec3 U(0.f, 0.f, 0.f);
   	glm::vec3 V(0.f, 0.f, 0.f);

   	auto flippedNormal = -1.f * brushPlane.normal;
   	buildQuakeTexAxes(brushPlane.normal, brushFace.rotation, brushFace.textureScale.x, brushFace.textureScale.y, &U, &V);

    float u = glm::dot(vertex, U) + brushFace.uvOffset.x;
    float v = glm::dot(vertex, V) + brushFace.uvOffset.y;

    u /= textureWidth;
    v /= textureHeight;

    return glm::vec2(u, -1.f * v);
}


struct FaceVertexAssignment {
    BrushFace* face;       
    std::vector<VertexWithData> vertices; 
};
std::vector<FaceVertexAssignment> assignVerticesToFaces(std::vector<BrushFace>& faces, std::vector<glm::vec3>& candidateVertices, float scale){
    std::vector<FaceVertexAssignment> assignments;

    for (auto& face : faces) {
       	auto brushPlane = brushFaceToPlane(face);
       	auto textureInfo = loadTextureSizeInfo(textureForMapTextureName(face.texture));

        std::vector<VertexWithData> vertsOnFace;

        // Pick vertices that lie on the plane of this face
        for (auto& v : candidateVertices) {
            float distance = glm::dot(brushPlane.normal, v) + brushPlane.distanceToPoint; // plane equation
            if (fabs(distance) < EPSILON) { // tolerance for floating point
                vertsOnFace.push_back(VertexWithData {
                	.pos = v,
                	.uv = calculateUv(face, brushPlane, v * scale, textureInfo.width, textureInfo.height),
                });
            }
        }

        sortVerticesCCW(vertsOnFace, -1.f * brushPlane.normal);

        assignments.push_back(FaceVertexAssignment{ 
        	.face = &face, 
        	.vertices = vertsOnFace,
        });
    }

    return assignments;
}

struct Triangle {
    glm::vec3 vertex0;
    glm::vec3 vertex1;
    glm::vec3 vertex2;

    glm::vec2 vertex0Uv;
    glm::vec2 vertex1Uv;
    glm::vec2 vertex2Uv;

    std::string* texture;
};

std::vector<Triangle> triangulateFace(FaceVertexAssignment& faceAssignment) {
    if (faceAssignment.vertices.size() < 3) {
    	//modassert(false, std::string("face assignment invalid number of vertices < 3: num = ") + std::to_string(faceAssignment.vertices.size()));
    	return {};
    }

    // Fan from the center, vertices form a perimeter so the triangles are the edge and connect to the center
    std::vector<Triangle> triangles;
    for (size_t i = 1; i + 1 < faceAssignment.vertices.size(); ++i) {
        triangles.push_back(Triangle {
        	.vertex0 = faceAssignment.vertices.at(0).pos,
        	.vertex1 = faceAssignment.vertices.at(i).pos,
        	.vertex2 = faceAssignment.vertices.at(i + 1).pos,

        	.vertex0Uv = faceAssignment.vertices.at(0).uv,
        	.vertex1Uv = faceAssignment.vertices.at(i).uv,
        	.vertex2Uv = faceAssignment.vertices.at(i + 1).uv,

        	.texture = &faceAssignment.face -> texture,
        });
    }
    return triangles;
}

struct MapRawValue {
	std::vector<glm::vec3> points;
	std::vector<glm::vec2> uvCoords;
};

glm::vec3 changeCoord(glm::vec3 pos){
	return glm::vec3(pos.x, pos.z, -1 * pos.y);
}


struct ModelAndPath {
	std::string brushFile;
	std::optional<int> entityIndex;
};

ModelAndPath parseModelPath(std::string modelPath){
	auto parts = split(modelPath, ',');

	if (parts.size() == 1){
		auto filename = parts.at(0);
		return ModelAndPath {
			.brushFile = parts.at(0),
			.entityIndex = std::nullopt,
		};
	}
	modassert(parts.size() == 2, "parseModelPath invalid parts size");

	auto realBrushFile = parts.at(0);
	auto indexPart = parts.at(1);
	auto entityIndexParts = split(indexPart, '.');
	modassert(entityIndexParts.size() == 2, "parseModelPath invalid part size");

	return ModelAndPath {
		.brushFile = realBrushFile,
		.entityIndex = std::atoi(entityIndexParts.at(0).c_str()),
	};
}

std::string print(ModelAndPath& modelpath){
	std::string data;
	data += "brushFile = " + modelpath.brushFile + "\n";
	data += "entityIndex = " + print(modelpath.entityIndex) + "\n";
	return data;
}

Entity& getEntityByName(MapData& mapData, const char* name){
  auto entities = getEntitiesByClassName(mapData, name);
  modassert(entities.size() == 1, std::string("unexpected number of entities for brush model: " + std::to_string(entities.size())));
  Entity& entity = *entities.at(0);
  return entity;
}

Entity& getEntityByIndex(MapData& mapData, int index){
	return mapData.entities.at(index);
}

struct MapBoundingBox {
	std::optional<float> minX;
	std::optional<float> maxX;
	std::optional<float> minY;
	std::optional<float> maxY;
	std::optional<float> minZ;
	std::optional<float> maxZ;
};
void addPoint(MapBoundingBox& box, glm::vec3 point){
	if (!box.minX.has_value()){
		box.minX = point.x;
	}
	if (!box.maxX.has_value()){
		box.maxX = point.x;
	}
	if (!box.minY.has_value()){
		box.minY = point.y;
	}
	if (!box.maxY.has_value()){
		box.maxY = point.y;
	}
	if (!box.minZ.has_value()){
		box.minZ = point.z;
	}
	if (!box.maxZ.has_value()){
		box.maxZ = point.z;
	}

	if (box.minX.value() > point.x){
		box.minX = point.x;
	}
	if (box.maxX.value() < point.x){
		box.maxX = point.x;
	}

	if (box.minY.value() > point.y){
		box.minY = point.y;
	}
	if (box.maxY.value() < point.y){
		box.maxY = point.y;
	}
	
	if (box.minZ.value() > point.z){
		box.minZ = point.z;
	}
	if (box.maxZ.value() < point.z){
		box.maxZ = point.z;
	}
}

glm::vec3 calcMidpoints(MapBoundingBox& boundingBox){
	modassert(boundingBox.minX.has_value(), "boundingBox no minX");
	modassert(boundingBox.maxX.has_value(), "boundingBox no maxX");
	modassert(boundingBox.minY.has_value(), "boundingBox no minY");
	modassert(boundingBox.maxY.has_value(), "boundingBox no maxY");
	modassert(boundingBox.minZ.has_value(), "boundingBox no minZ");
	modassert(boundingBox.maxZ.has_value(), "boundingBox no maxZ");

	float midpointX = 0.5f * (boundingBox.maxX.value() + boundingBox.minX.value());
	float midpointY = 0.5f *  (boundingBox.maxY.value() + boundingBox.minY.value());
	float midpointZ = 0.5f * (boundingBox.maxZ.value() + boundingBox.minZ.value());
	return glm::vec3(midpointX, midpointY, midpointZ);
}

ModelDataCore loadModelCoreBrush(std::string modelPath){
	std::vector<glm::vec3> debugPoints;
  std::vector<std::optional<glm::vec3>> debugPointsTo;
	std::vector<glm::vec3> debugColors;

  std::cout << "loadModelCoreBrush: " << modelPath << std::endl;

  auto parsedPath = parseModelPath(modelPath);
  std::cout << print(parsedPath ) << std::endl;

  auto mapData = parseMapData(parsedPath.brushFile);

  Entity& entity = parsedPath.entityIndex.has_value() ? getEntityByIndex(mapData, parsedPath.entityIndex.value()) : getEntityByName(mapData, "worldspawn");

  std::unordered_map<std::string, MapRawValue> meshForTextures;

  MapBoundingBox boundingBox{};

  for (auto& brush : entity.brushes){
  	{
  		std::vector<BrushPlane> brushPlanes;
			for (auto& brushFace : brush.brushFaces){
				auto brushPlane = brushFaceToPlane(brushFace);
				brushPlanes.push_back(brushPlane);
			}

			for(auto& brushPlane : brushPlanes){

				auto pointOne = brushPlane.pointOnPlane;
				auto pointTwo = brushPlane.pointOnPlane2;
				auto pointThree = brushPlane.pointOnPlane3;

				auto midpoint = (pointOne + pointTwo + pointThree) / 3.f;;

				debugPoints.push_back(midpoint);
				debugPointsTo.push_back(midpoint + (glm::normalize(brushPlane.normal) * 10.f));
				debugColors.push_back(glm::vec3(1.f, 0.f, 0.f));

				debugPoints.push_back(pointOne);
				debugPointsTo.push_back(pointTwo);
				debugColors.push_back(glm::vec3(0.f, 0.f, 1.f));

				debugPoints.push_back(pointOne);
				debugPointsTo.push_back(pointThree);
				debugColors.push_back(glm::vec3(0.f, 0.f, 1.f));

				debugPoints.push_back(pointTwo);
				debugPointsTo.push_back(pointThree);
				debugColors.push_back(glm::vec3(0.f, 0.f, 1.f));
			}

  	}
 		std::vector<bool> isForced;
  	auto candidateVertices = getAllIntersections(brush, false, isForced);

  	{
  		auto allVertices = getAllIntersections(brush, true, isForced);
  		for (int i = 0; i < allVertices.size(); i++){
  			auto& vertex = allVertices.at(i);
  			debugPoints.push_back(vertex);
  			auto isInside = isForced.at(i);
  			debugColors.push_back(isInside ? glm::vec3(1.f, 1.f, 1.f) : glm::vec3(0.f, 1.f, 0.f));
 			  debugPointsTo.push_back(std::nullopt);
  		}  		
  	}

  	std::cout << "candidateVertices: size = " << candidateVertices.size() << std::endl;

  	auto faceVertices = assignVerticesToFaces(brush.brushFaces, candidateVertices, mapData.scale);
  	std::cout << "face vertices size: " << faceVertices.size() << std::endl;

  	for(auto& faceVertice : faceVertices){
	    std::cout << "Face has " << faceVertice.vertices.size() << " vertices\n";
	  	auto triangles = triangulateFace(faceVertice);

	  	for (auto& triangle : triangles){
	  		auto texture = textureForMapTextureName(*triangle.texture);
	  		if (meshForTextures.find(texture) == meshForTextures.end()){
	  			meshForTextures[texture] = MapRawValue{};
	  		}

	  		auto& meshForTexture = meshForTextures.at(texture);
		  	meshForTexture.points.push_back(changeCoord(triangle.vertex0));
		  	meshForTexture.uvCoords.push_back(triangle.vertex0Uv);
		  	addPoint(boundingBox, changeCoord(triangle.vertex0));

		  	meshForTexture.points.push_back(changeCoord(triangle.vertex1));
		  	meshForTexture.uvCoords.push_back(triangle.vertex1Uv);
		  	addPoint(boundingBox, changeCoord(triangle.vertex1));

		  	meshForTexture.points.push_back(changeCoord(triangle.vertex2));
		  	meshForTexture.uvCoords.push_back(triangle.vertex2Uv);
		  	addPoint(boundingBox, changeCoord(triangle.vertex2));

	  	}
  	}
  }

  modassert(entity.brushes.size() > 0, std::string("loading an entity brush with 0 brushes: ") + modelPath + ", path = " + print(parsedPath));
 	auto boundingOffset = calcMidpoints(boundingBox);
  
  ModelDataCore modelDataCore2 {
    .modelData = ModelData {
      .meshIdToMeshData = {},
      .nodeToMeshId = {},
      .childToParent = {},
      .nodeTransform = {
        { 0, Transformation {
          .position = boundingOffset,
          .scale = glm::vec3(1.f, 1.f, 1.f),
          .rotation = MOD_ORIENTATION_FORWARD,
        }}
      },
      .names = {{ 0, "test" }},
      .animations = {},
	    .sponsorRootPosition = true,
    },
    .loadedRoot = "test",
  };

  int meshId = 0;
  std::vector<int> meshIds;
  for (auto& [texturePath, meshForTexture] : meshForTextures){
  	std::vector<unsigned int> indexs;
  	addPointsToSimpleMesh(meshForTexture.points, indexs);
  	modassert(meshForTexture.points.size() == meshForTexture.uvCoords.size(), "unexpected diff in points and uv coords size");
  	
  	for (int i = 0; i < meshForTexture.points.size(); i++){
  		meshForTexture.points.at(i) -= boundingOffset;
  	}

  	std::string texture = texturePath;
  	auto normalTexture = lookupNormalTexture(texture);
  	//modassert(normalTexture.has_value(), std::string("normal for texture does not exist: ") + texture);
  	auto generatedMesh = generateMeshRaw(meshForTexture.points, meshForTexture.uvCoords, indexs, &texture, normalTexture.has_value() ? &normalTexture.value() : NULL);
  	meshIds.push_back(meshId);
  	modelDataCore2.modelData.meshIdToMeshData[meshId] = generatedMesh;
   	
  	meshId++;
  }
 	modelDataCore2.modelData.nodeToMeshId[0] = meshIds;

 	std::cout << "loadModelCoreBrush num meshes: " << modelDataCore2.modelData.nodeToMeshId.at(0).size() << std::endl;

 	//setDebugPoints(debugPoints, debugPointsTo, debugColors);

  return modelDataCore2;
}
