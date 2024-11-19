#include "./scene_lighting.h"

int voxelCellWidth = 8;
int numCellsDim = 8;

int xyzToIndex(int x , int y, int z){
	return x + (numCellsDim * y) + (numCellsDim * numCellsDim * z);
}

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

std::vector<LightingCell> generateLightingCellsDebug(int size){
  auto cells = generateLightingCells(size);
  for (int y = 0; y < size; y++){
  	if (y == 0){
  		continue;
  	}
  	for (int x = 0; x < size; x++){
  		auto nearIndex = xyzToIndex(x, y, 0);
  		auto farIndex = xyzToIndex(x, y, (size - 1));
  		cells.at(nearIndex).color = glm::vec3(1.f, 1.f, 1.f);
   		cells.at(farIndex).color = glm::vec3(1.f, 1.f, 1.f);
  	}
  	for (int z = 0; z < size ; z++){
  		auto nearIndex = xyzToIndex(0, y, z);
   		auto farIndex = xyzToIndex((size - 1), y, z);
  		cells.at(nearIndex).color = glm::vec3(1.f, 1.f, 1.f);
  		cells.at(farIndex).color = glm::vec3(1.f, 1.f, 1.f);
  	}
  }
  return cells;
}


VoxelLightingData lightingData {
  .voxelCellWidth = voxelCellWidth,
  .cells = generateLightingCellsDebug(numCellsDim),  // this is hardcoded in the shader
};

std::string printDebugVoxelLighting(){
	std::string data = "";
	for (auto &cell : lightingData.cells){
		data += "(" + std::to_string(cell.lightIndex) + " = " + print(cell.color) + ") ";
	}
	return data;
}

std::optional<int> lightingPositionToIndex(glm::vec3 position, glm::ivec3 offset){
	auto x = offset.x + static_cast<int>(convertBase(
		position.x, 
		lightingData.voxelCellWidth * numCellsDim * -0.5, lightingData.voxelCellWidth * numCellsDim * 0.5, 
		0, numCellsDim
	));
	auto y = offset.y + static_cast<int>(convertBase(
		position.y, 
		lightingData.voxelCellWidth * numCellsDim * -0.5, lightingData.voxelCellWidth * numCellsDim * 0.5, 
		0, numCellsDim
	));
	auto z = offset.z + static_cast<int>(convertBase(
		position.z, 
		lightingData.voxelCellWidth * numCellsDim * -0.5, lightingData.voxelCellWidth * numCellsDim * 0.5, 
		0, numCellsDim
	));

	if (x < 0 || x >= numCellsDim){
		return std::nullopt;
	}
	if (y < 0 || y >= numCellsDim){
		return std::nullopt;
	}
	if (z < 0 || z >= numCellsDim){
		return std::nullopt;
	}
	auto index = xyzToIndex(x, y, z);
	modlog("voxel lighting lightingPositionToIndex", std::to_string(index) + " " + print(glm::vec3(x, y, z)));
	return index;
}

void addVoxelLight(objid lightIndex, glm::vec3 position, int radius){
	glm::vec3 color(1.f, 1.f, 1.f);

	modlog("voxel lighting add: ", std::to_string(lightIndex));
	for (int x = (-radius + 1); x < radius; x++){
		for (int y = (-radius + 1); y < radius; y++){ 
			for (int z = (-radius + 1); z < radius; z++){
				auto lightValue = lightingPositionToIndex(glm::vec3(position.x, position.y, position.z), glm::ivec3(x, y, z));
				if (!lightValue.has_value()){
					continue;
				}
				auto index = lightValue.value();
				modassert(lightingData.cells.at(index).lightIndex == 0, "light cell already occupied");
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
void updateVoxelLightPosition(objid lightIndex, glm::vec3 position, int radius){
	modlog("update voxel light position", print(position));
	removeVoxelLight(lightIndex);
	addVoxelLight(lightIndex, position, radius);
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
