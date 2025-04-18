#ifndef MOD_SCENE_LIGHTING
#define MOD_SCENE_LIGHTING

#include <vector>
#include <glm/glm.hpp>

#include "../../../common/util.h"
#include "../../../translations.h"

struct LightingCell {
  objid lightIndex;
};
struct VoxelLightingData {
  int lightsPerVoxel;
  int voxelCellWidth;
  int numCellsDim;
  std::vector<LightingCell> cells;
};  

struct LightingUpdate {
  int index;
  objid lightIndex;
};

int getLightingCellWidth();
std::vector<LightingUpdate> getLightUpdates();
void addVoxelLight(objid lightIndex, glm::vec3 position, int radius);
void removeVoxelLight(objid lightIndex);
void updateVoxelLightPosition(objid lightIndex, glm::vec3 position, int radius);

VoxelLightingData& getVoxelLightingData();

#endif
