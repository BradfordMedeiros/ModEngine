#ifndef MOD_MAP
#define MOD_MAP

#include <string>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include "../../common/util.h"

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

struct LayerEntity {
    int layerId;
    std::vector<EntityKeyValue> keyValues;
    std::vector<Brush> brushes;
};

struct Entity {
    int layerId;
    std::vector<EntityKeyValue> keyValues;
    std::vector<Brush> brushes; // usually empty unless special case
};

struct MapData {
    std::vector<LayerEntity> layers;
    std::vector<Entity> entities;
};

MapData parseMapData(std::string file);

#endif 
