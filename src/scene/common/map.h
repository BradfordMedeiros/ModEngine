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
    std::vector<Brush> brushes;
};

struct MapData {
    float scale;
    std::vector<Entity> entities;
};

MapData parseMapData(std::string file);
MapData parseRawMapData(std::string& fileContent);
std::vector<Entity*> getEntitiesByClassName(MapData& mapData, const char* name);

std::optional<std::string*> getValue(Entity& entity, const char* key);
std::optional<int> getIntValue(Entity& entity, const char* key);
std::optional<float> getFloatValue(Entity& entity, const char* key);
std::optional<float> getScaledFloatValue(MapData& mapData, Entity& entity, const char* key);
std::optional<glm::vec3> getScaledVec3Value(MapData& mapData, Entity& entity, const char* key);
std::optional<glm::vec3> getUnitVec3Value(MapData& mapData, Entity& entity, const char* key);
std::optional<glm::vec3> getVec3Value(Entity& entity, const char* key);
std::optional<glm::vec4> getVec4Value(Entity& entity, const char* key);
glm::vec3 changeCoord(glm::vec3 pos);

bool isLayerEntity(Entity& entity, int* layerId);

void writeMapModelFile(MapData& mapData, std::string filepath);

struct GameobjAttributeOpts {
  std::string field;
  AttributeValue attributeValue;
  std::optional<std::string> submodel;
};

void compileRawScene(std::string filepath, std::string baseFile, std::string mapFile, std::string brushFileOut, std::function<void(MapData& mapData, Entity& entity, bool* shouldWrite, std::vector<GameobjAttributeOpts>& attributes, std::string* _modelName)> callback, std::function<void(MapData& mapData, std::string&)> afterEntities, bool copyMapFile);

glm::quat quatFromTrenchBroomAngles(float pitch, float yaw, float roll);
glm::quat quatFromTrenchBroomAngles2(float pitch, float yaw, float roll);


struct BrushPlane {
    float distanceToPoint;
    glm::vec3 normal;
    glm::vec3 pointOnPlane;
    glm::vec3 pointOnPlane2;
    glm::vec3 pointOnPlane3;
};

struct BrushLightingInfo {
    glm::vec3 lightPosition;
    std::vector<BrushPlane> brushPlanes;
};

enum BrushLightType { BOUNDING, POINT };
struct EntityLightingInfo {
    BrushLightType type;
    glm::vec3 color;
    std::optional<float> radius;
    std::vector<BrushLightingInfo> brushLightingInfo;
};

struct MapLighting {
    std::unordered_map<std::string, EntityLightingInfo> lightzoneToEntity;
};

MapLighting loadBrushLighting(std::string modelPath);


glm::vec3 calculateLightingForPoint(MapLighting& lightingInfo, glm::vec3 point);
   
#endif 
