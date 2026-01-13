#ifndef MOD_MAP
#define MOD_MAP

#include <string>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "../../common/util.h"
#include "../../common/files.h"
#include "./mesh.h"


struct BrushFace {
    glm::vec3 point1;
    glm::vec3 point2;
    glm::vec3 point3;
    glm::vec2 uvOffset;
    glm::vec2 textureScale;
    float rotation;
    std::string texture;
};
struct Brush {
	std::vector<BrushFace> brushFaces;
};

struct EntityKeyValue {
    std::string key;
    std::string value;
};

struct Entity {
    int layerId;
    int index;
    std::vector<EntityKeyValue> keyValues;
    std::vector<Brush> brushes; // usually empty unless special case
};

struct MapData {
    float scale;
    std::vector<Entity> layers;
    std::vector<Entity> entities;
};

MapData parseMapData(std::string file);
MapData parseRawMapData(std::string& fileContent);
std::vector<Entity*> getEntitiesByClassName(MapData& mapData, const char* name);

std::optional<std::string*> getValue(Entity& entity, const char* key);
std::optional<int> getIntValue(Entity& entity, const char* key);
std::optional<glm::vec3> getScaledVec3Value(MapData& mapData, Entity& entity, const char* key);
std::optional<glm::vec3> getUnitVec3Value(MapData& mapData, Entity& entity, const char* key);
std::optional<glm::vec3> getVec3Value(MapData& mapData, Entity& entity, const char* key);

void writeMapModelFile(MapData& mapData, std::string filepath);
std::vector<Mesh> loadMapModel(MapData& mapData, std::string filepath);

struct GameobjAttributeOpts {
  std::string field;
  AttributeValue attributeValue;
  std::optional<std::string> submodel;
};

void compileRawScene(std::string filepath, std::string baseFile, std::string mapFile, std::function<void(MapData& mapData, Entity& entity, bool* shouldWrite, std::vector<GameobjAttributeOpts>& attributes, std::string* _modelName)> callback, std::function<void(MapData& mapData, std::string&)> afterEntities);



#endif 
