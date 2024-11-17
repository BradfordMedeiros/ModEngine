#include "./scene_lighting.h"

std::vector<LightingCell> generateLightingCells(int size){
  std::vector<LightingCell> cells;
  for (int x = 0; x < size; x++){
    for (int y = 0; y < size; y++){
      for (int z = 0; z < size; z++){
      	cells.push_back(LightingCell {
          .lightIndex = 0,
          .color = glm::vec3(0.f, 0.f, 0.f),
        });

      	// gradient code
        //float red = (x % size) / static_cast<float>(size);
        //float green = (y % size) / static_cast<float>(size);
        //float blue = (z % size) / static_cast<float>(size);
        //cells.push_back(LightingCell {
        //  .lightIndex = 0,
        //  .color = glm::vec3(red, green, blue),
        //});
      }
    }
  }
  return cells;
}


VoxelLightingData lightingData {
  .voxelCellWidth = 8,
  .cells = generateLightingCells(8),  // this is hardcoded in the shader
};

int lightingPositionToIndex(glm::vec3 position){
	return 0;
}

void addLight(objid lightIndex, glm::vec3 position, glm::vec3 color, float radius){
	auto index = lightingPositionToIndex(position);
	lightingData.cells.at(index) = LightingCell {
		.lightIndex = lightIndex,
		.color = color,
	};
}
void removeLight(objid lightIndex){

}

int getLightingCellWidth(){
	return lightingData.voxelCellWidth;
}

std::vector<LightingUpdate> getLightUpdates(){
  std::vector<LightingUpdate> lightUpdates;
  for (int i = 0; i < lightingData.cells.size(); i++){
    lightUpdates.push_back(LightingUpdate {
      .index = i,
      .color = lightingData.cells.at(i).color,
    });
  }
  return lightUpdates;
}
