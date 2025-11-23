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
    std::vector<Entity> layers;
    std::vector<Entity> entities;
};

MapData parseMapData(std::string file);
std::vector<Entity*> getEntitiesByClassName(MapData& mapData, const char* name);
std::optional<std::string*> getValue(Entity& entity, const char* key);

void writeMapModelFile(MapData& mapData, std::string filepath);
std::vector<Mesh> loadMapModel(MapData& mapData, std::string filepath);

void compileRawScene(std::string filepath, std::string baseFile, std::string mapFile, std::function<void(Entity& entity, bool* shouldWrite, std::vector<GameobjAttribute>& attributes)> callback);

// another one to load the map model file to a mesh


#endif 
