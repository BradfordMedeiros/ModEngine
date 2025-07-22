#ifndef MOD_SCENE_LIGHTING
#define MOD_SCENE_LIGHTING

#include <vector>
#include <glm/glm.hpp>

#include "../../../common/util.h"
#include "../../../translations.h"

struct LightingCell {
  objid lightIndex;
  int needsUpdateFrame;
};
struct VoxelLightingData {
  int lightsPerVoxel;
  int voxelCellWidth;
  int numCellsDim;
  std::vector<LightingCell> cells;
  int defaultLightIndex;
  glm::vec3 offset;
};  

struct LightingUpdate {
  int index;
  objid lightIndex;
};

int getLightingCellWidth();
int getLightingNumCellsDim();
int getLightingNumCellsTotal();
std::vector<LightingUpdate> getLightUpdates();
void addVoxelLight(objid lightIndex, glm::vec3 position, int radius);
void removeVoxelLight(objid lightIndex, bool removeDefaultLight);
void updateVoxelLightPosition(objid lightIndex, glm::vec3 position, int radius);

struct LightUpdate { 
  objid lightIndex;
  glm::vec3 position;
  int radius;
};
void recalculateLights(std::vector<LightUpdate>& allUpdates);

VoxelLightingData& getVoxelLightingData();
void setGlobalLight(objid);
void setVoxelLighting(int voxelCellWidth, glm::vec3 worldOffset);

#endif
