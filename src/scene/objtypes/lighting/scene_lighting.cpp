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
      }
    }
  }
  return cells;
}


VoxelLightingData lightingData {
  .voxelCellWidth = 8,
  .cells = generateLightingCells(8),  // this is hardcoded in the shader
};

std::string printDebugVoxelLighting(){
	std::string data = "";
	for (auto &cell : lightingData.cells){
		data += "(" + std::to_string(cell.lightIndex) + " = " + print(cell.color) + ") ";
	}
	return data;
}

int lightingPositionToIndex(glm::vec3 position){
	auto x = convertBase(
		position.x, 
		lightingData.voxelCellWidth * 8 * -0.5, lightingData.voxelCellWidth * 8 * 0.5, 
		0, 8
	);
	auto y = convertBase(
		position.y, 
		lightingData.voxelCellWidth * 8 * -0.5, lightingData.voxelCellWidth * 8 * 0.5, 
		0, 8
	);
	auto z = convertBase(
		position.z, 
		lightingData.voxelCellWidth * 8 * -0.5, lightingData.voxelCellWidth * 8 * 0.5, 
		0, 8
	);

	int numCellsDim = lightingData.voxelCellWidth;
	return x + (numCellsDim * y) + (numCellsDim * numCellsDim * z);
}

void addVoxelLight(objid lightIndex, glm::vec3 position){
	int radius = 3;
	glm::vec3 color(1.f, 1.f, 1.f);

	modlog("voxel lighting add: ", std::to_string(lightIndex));
	for (int x = 0; x < radius; x++){
		for (int y = 0; y < radius; y++){
			for (int z = 0; z < radius; z++){
				auto index = lightingPositionToIndex(glm::vec3(position.x + x, position.y + y, position.z + z));
				modassert(index >= 0 && index < lightingData.cells.size(), std::string("Invalid light index, got = ") + std::to_string(index));
				lightingData.cells.at(index) = LightingCell {
					.lightIndex = lightIndex,
					.color = color,
				};
			}
		}
	}

	std::cout << "voxel lighting lighting data: " << print(printDebugVoxelLighting()) << std::endl;

}
void removeVoxelLight(objid lightIndex){
	modlog("voxel lighting remove: ", std::to_string(lightIndex));
	for (auto &cell : lightingData.cells){
		if (cell.lightIndex == lightIndex){
			cell.lightIndex = 0;
			cell.color = glm::vec3(0.f, 0.f, 0.f);
		}
	}

	std::cout << "voxel lighting lighting data: " << print(printDebugVoxelLighting()) << std::endl;
}

// obviously this could be more efficient
// eg could keep a mapping of cell ids to shortcut to them
void updateVoxelLightPosition(objid lightIndex, glm::vec3 position){
	modlog("update voxel light position", print(position));
	removeVoxelLight(lightIndex);
	addVoxelLight(lightIndex, position);
}

int getLightingCellWidth(){
	return lightingData.voxelCellWidth;
}


// should make not update all lights every frame only those that change
std::vector<LightingUpdate> getLightUpdates(){
	modlog("voxel lighting updates size = : ", std::to_string(lightingData.cells.size()));

  std::vector<LightingUpdate> lightUpdates;
  for (int i = 0; i < lightingData.cells.size(); i++){
    lightUpdates.push_back(LightingUpdate {
      .index = i,
      .color = lightingData.cells.at(i).color,
    });
  }
  return lightUpdates;
}
