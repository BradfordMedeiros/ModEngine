#include "./map.h"

std::string readFileOrPackage(std::string filepath);
std::string serializeAttributeValue(AttributeValue& value);
void setDebugPoints(std::vector<glm::vec3> points, std::vector<glm::vec3> colors);

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

glm::vec3 parseVecTrenchbroom(std::string positionRaw){;
	return parseVec(positionRaw);

  float x, y, z;
  std::istringstream in(positionRaw);
  in >> x >> y >> z;
  return glm::vec3(x, z, y);
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
			glm::vec3 vec = parseVecTrenchbroom(stringValue);
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



std::string getEntityName(std::string& baseName, std::optional<std::string>& submodel){
	if (submodel.has_value()){
		return baseName + "/" + submodel.value();
	}
	return baseName;
}

void compileBrushes(MapData& mapData, std::string& rawMapData, std::string path){
	realfiles::saveFile(path, rawMapData);
}


void compileRawScene(std::string filepath, std::string baseFile,  std::string mapFile,  std::function<void(Entity& entity, bool* _shouldWrite, std::vector<GameobjAttributeOpts>& _attributes, std::string* _modelName)> callback){
	std::string content = "########## Base file content: " + baseFile + " ##########\n\n" + readFileOrPackage(baseFile) + "\n\n";

	std::string generatedContent = "##########  Generated content: + " + mapFile + "\n\n";

	std::string generatedScene;

	auto rawMapContent = readFileOrPackage(mapFile);

	auto mapData = parseMapData(mapFile);
	for (auto& entity: mapData.entities){
		bool shouldWrite = false;
		std::string modelName = "";
		std::vector<GameobjAttributeOpts> attributes;
		callback(entity, &shouldWrite, attributes, &modelName);

		bool userSpecifiedPosition = false;
		if (shouldWrite){
		    auto origin = getValue(entity, "origin");
		    auto classname = getValue(entity, "classname");

		    glm::vec3 position = origin.has_value() ? parseVecTrenchbroom(*origin.value()) : glm::vec3(0.f, 0.f, 0.f);
		    modassert(classname.has_value(), "no classname");

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

	realfiles::saveFile(filepath, content + generatedContent + generatedScene);

	compileBrushes(mapData, rawMapContent, "./build/temp.brush");
}

void addPointsToSimpleMesh(std::vector<glm::vec3>& points, std::vector<glm::vec2>& _uvCoords,  std::vector<unsigned int>& _indexs, bool addUvs){
  for (int i = 0; i < points.size(); i++){
  	int remainder = i % 6;
  	if (addUvs){
  		if (remainder == 0){
    	  _uvCoords.push_back(glm::vec2(0.f, 0.f));
  		}else if (remainder == 1){
    	  _uvCoords.push_back(glm::vec2(0.f, 1.f));
  		}else if (remainder == 2){
    	  _uvCoords.push_back(glm::vec2(1.f, 0.f));
  		}else if (remainder == 3){
    	  _uvCoords.push_back(glm::vec2(1.f, 0.f));
  		}else if (remainder == 4){
    	  _uvCoords.push_back(glm::vec2(0.f, 1.f));
  		}else if (remainder == 5){
    	  _uvCoords.push_back(glm::vec2(1.f, 1.f));
  		}else{
  			modassert(false, "programming error %");
  		}
  	}
    _indexs.push_back(i);
  }
}


struct BrushPlane {
	float distanceToPoint;
	glm::vec3 normal;
	glm::vec3 pointOnPlane;
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
		bool insidePlane = projection >= 0.f;

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

glm::vec2 calculateUv(BrushFace& brushFace, BrushPlane& brushPlane, glm::vec3 vertex) {
    // Plane normal

		vertex.x /= 1024;
		vertex.y /= 1024;
		vertex.z /= 1024;

    glm::vec3 N = brushPlane.normal;

    // Pick a stable "up" vector to avoid degeneracy
    glm::vec3 up = (fabs(N.z) > 0.999f) ? glm::vec3(0, 1, 0) : glm::vec3(0, 0, 1);

    // Construct local U/V axes on the plane
    glm::vec3 U = glm::normalize(glm::cross(up, N));
    glm::vec3 V = glm::normalize(glm::cross(N, U));

    // Project the vertex onto the plane axes
    float u = glm::dot(vertex, U);
    float v = glm::dot(vertex, V);

    // Apply TrenchBroom scale and shift exactly as in the .map file
    // scale = world units per texture repeat
    // shift = world-space offset along the axis
    u = (u + brushFace.uvOffset.x) / brushFace.textureScale.x;
    v = (v + brushFace.uvOffset.y) / brushFace.textureScale.y;

    return glm::vec2(u, v);
}

struct FaceVertexAssignment {
    BrushFace* face;       
    std::vector<VertexWithData> vertices; 
};
std::vector<FaceVertexAssignment> assignVerticesToFaces(std::vector<BrushFace>& faces, std::vector<glm::vec3>& candidateVertices){
    std::vector<FaceVertexAssignment> assignments;

    for (auto& face : faces) {
       	auto brushPlane = brushFaceToPlane(face);

        std::vector<VertexWithData> vertsOnFace;

        // Pick vertices that lie on the plane of this face
        for (auto& v : candidateVertices) {
            float distance = glm::dot(brushPlane.normal, v) + brushPlane.distanceToPoint; // plane equation
            if (fabs(distance) < 0.001f) { // tolerance for floating point
                vertsOnFace.push_back(VertexWithData {
                	.pos = v,
                	.uv = calculateUv(face, brushPlane, v),
                });
            }
        }

        sortVerticesCCW(vertsOnFace, brushPlane.normal);

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
        });
    }
    return triangles;
}


MeshData generateMeshRaw(std::vector<glm::vec3>& verts, std::vector<glm::vec2>& uvCoords, std::vector<unsigned int>& indices);
ModelDataCore loadModelCoreBrush(std::string modelPath){
	std::vector<glm::vec3> debugPoints;
	std::vector<glm::vec3> debugColors;

  std::string brushModel = "worldspawn";

  std::cout << "loadModelCoreBrush: " << modelPath << ", model = " << brushModel << std::endl;
  auto brushDataRaw2 = readFileOrPackage(modelPath);

  auto mapData = parseMapData(modelPath);
  auto entities = getEntitiesByClassName(mapData, brushModel.c_str());
  modassert(entities.size() == 1, std::string("unexpected number of entities for brush model: " + std::to_string(entities.size())));

  Entity& entity = *entities.at(0);

  std::vector<glm::vec3> points;
  std::vector<glm::vec2> uvCoords;

  for (auto& brush : entity.brushes){
 		std::vector<bool> isForced;
  	auto candidateVertices = getAllIntersections(brush, false, isForced);

  	{
  		auto allVertices = getAllIntersections(brush, true, isForced);
  		for (int i = 0; i < allVertices.size(); i++){
  			auto& vertex = allVertices.at(i);
  			debugPoints.push_back(vertex);
  			auto isInside = isForced.at(i);
  			debugColors.push_back(isInside ? glm::vec3(1.f, 1.f, 1.f) : glm::vec3(0.f, 1.f, 0.f));
  		}  		
  	}

  	std::cout << "candidateVertices: size = " << candidateVertices.size() << std::endl;

  	auto faceVertices = assignVerticesToFaces(brush.brushFaces, candidateVertices);

  	std::cout << "face vertices size: " << faceVertices.size() << std::endl;
  	for(auto& faceVertice : faceVertices){
	    std::cout << "Face has " << faceVertice.vertices.size() << " vertices\n";

	  	auto triangles = triangulateFace(faceVertice);
	  	for (auto& triangle : triangles){
		  	points.push_back(triangle.vertex0);
		  	uvCoords.push_back(triangle.vertex0Uv);

		  	points.push_back(triangle.vertex1);
		  	uvCoords.push_back(triangle.vertex1Uv);

		  	points.push_back(triangle.vertex2);
		  	uvCoords.push_back(triangle.vertex2Uv);
	  	}
  	}

  }

  
  std::cout << "size points: " << points.size() << std::endl;

  std::vector<unsigned int> indexs;
  addPointsToSimpleMesh(points, uvCoords, indexs, false);
  modassert(points.size() == uvCoords.size(), "unexpected diff in points and uv coords size");

  // Just collect all similar textures, render them as separate meshes

  auto generatedMesh = generateMeshRaw(points, uvCoords, indexs);
  ModelDataCore modelDataCore2 {
    .modelData = ModelData {
      .meshIdToMeshData = {{ 0, generatedMesh }},
      .nodeToMeshId = {{ 0, { 0 }}},
      .childToParent = {},
      .nodeTransform = {
        { 0, Transformation {
          .position = glm::vec3(0.f, 0.f, 0.f),
          .scale = glm::vec3(1.f, 1.f, 1.f),
          .rotation = MOD_ORIENTATION_FORWARD,
        }}
      },
      .names = {{ 0, "test" }},
      .animations = {},      
    },
    .loadedRoot = "test",
  };

 	setDebugPoints(debugPoints, debugColors);

  return modelDataCore2;
}
