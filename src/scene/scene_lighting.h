#ifndef MOD_SCENE_LIGHTING
#define MOD_SCENE_LIGHTING

#include <vector>
#include <glm/glm.hpp>

#include "../common/util.h"

struct LightingCell {
  objid lightIndex;
  glm::vec3 color;
};
struct VoxelLightingData {
  int voxelCellWidth;
  std::vector<LightingCell> cells;
};  

struct LightingUpdate {
  int index;
  glm::vec3 color;
};

int getLightingCellWidth();
std::vector<LightingUpdate> getLightUpdates();
void addLight(objid lightIndex, glm::vec3 position, glm::vec3 color, float radius);
void removeLight(objid lightIndex);

#endif
