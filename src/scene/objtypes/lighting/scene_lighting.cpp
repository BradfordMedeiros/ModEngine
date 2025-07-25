#include "./scene_lighting.h"

extern int currentTick;

int voxelCellWidth = 8;
int numCellsDim = 128;

//int voxelCellWidth = 16;
//int numCellsDim = 8;

int xyzToIndex(int x , int y, int z){
	return x + (numCellsDim * y) + (numCellsDim * numCellsDim * z);
}

std::vector<LightingCell> generateLightingCells(int size, int lightIndex = -1){
  std::vector<LightingCell> cells;
  for (int x = 0; x < size; x++){
    for (int y = 0; y < size; y++){
      for (int z = 0; z < size; z++){
      	cells.push_back(LightingCell {
          .lightIndex = lightIndex,
        });
      }
    }
  }
  return cells;
}

std::vector<LightingCell> generateLightingCellsDebug(int size){
  auto cells = generateLightingCells(size, -2);
  for (int y = 0; y < size; y++){
  	if (y == 0){
  		continue;
  	}
  	for (int x = 0; x < size; x++){
  		auto nearIndex = xyzToIndex(x, y, 0);
  		auto farIndex = xyzToIndex(x, y, (size - 1));
  		cells.at(nearIndex).lightIndex = -2;
   		cells.at(farIndex).lightIndex = -2;
  	}
  	for (int z = 0; z < size ; z++){
  		auto nearIndex = xyzToIndex(0, y, z);
   		auto farIndex = xyzToIndex((size - 1), y, z);
  		cells.at(nearIndex).lightIndex = -2;
   		cells.at(farIndex).lightIndex = -2;
  	}
  }
  return cells;
}

std::set<objid> allCells(int size){
	std::set<objid> ids;
	int index = 0;
  for (int x = 0; x < size; x++){
    for (int y = 0; y < size; y++){
      for (int z = 0; z < size; z++){
      	ids.insert(index);
      	index++;
      }
    }
  }
  return ids;
}

VoxelLightingData lightingData {
	.lightsPerVoxel = 1,
  .voxelCellWidth = voxelCellWidth,
  .numCellsDim = numCellsDim,
  .cells = generateLightingCells(numCellsDim),  // this is hardcoded in the shader
  .defaultLightIndex = 0,
  .offset = glm::vec3(0.f, 0.f, 0.f),
  .lastLightPosition = {},
  .needsUpdate = allCells(numCellsDim),
};

std::string printDebugVoxelLighting(){
	std::string data = "";
	for (auto &cell : lightingData.cells){
		data += "(" + std::to_string(cell.lightIndex) + " ) ";
	}
	return data;
}

std::optional<int> lightingPositionToIndex(glm::vec3 position, glm::ivec3 offset){
	auto x = offset.x + static_cast<int>(convertBase(
		position.x + lightingData.offset.x, 
		lightingData.voxelCellWidth * numCellsDim * -0.5, lightingData.voxelCellWidth * numCellsDim * 0.5, 
		0, numCellsDim
	));
	auto y = offset.y + static_cast<int>(convertBase(
		position.y + lightingData.offset.y, 
		lightingData.voxelCellWidth * numCellsDim * -0.5, lightingData.voxelCellWidth * numCellsDim * 0.5, 
		0, numCellsDim
	));
	auto z = offset.z + static_cast<int>(convertBase(
		position.z + lightingData.offset.z, 
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
	//modlog("voxel lighting lightingPositionToIndex", std::to_string(index) + " " + print(glm::vec3(x, y, z)));
	return index;
}

void addVoxelLight(objid lightIndex, glm::vec3 position, int requestedRadius){
	lightingData.lastLightPosition[lightIndex] = position;
	if (lightIndex == lightingData.defaultLightIndex){
		return;
	}
	modlog("voxel lighting add: ", std::to_string(lightIndex));
	int radius = requestedRadius;
	if (radius <= 0){
		radius = 1; // max size 
	}
	glm::vec3 color(1.f, 1.f, 1.f);

	//modlog("voxel lighting add: ", std::to_string(lightIndex));
	for (int x = (-radius + 1); x < radius; x++){
		for (int y = (-radius + 1); y < radius; y++){ 
			for (int z = (-radius + 1); z < radius; z++){
				auto lightValue = lightingPositionToIndex(glm::vec3(position.x, position.y, position.z), glm::ivec3(x, y, z));
				if (!lightValue.has_value()){
					continue;
				}
				auto index = lightValue.value();
				//modassert(lightingData.cells.at(index).lightIndex == 0, "light cell already occupied");
				modassert(index >= 0 && index < lightingData.cells.size(), std::string("Invalid light index, got = ") + std::to_string(index));
				lightingData.cells.at(index) = LightingCell {
					.lightIndex = lightIndex,
				};
				lightingData.needsUpdate.insert(index);
			}
		}
	}

	//std::cout << "voxel lighting lighting data: " << print(printDebugVoxelLighting()) << std::endl;
}
void removeVoxelLight(objid lightIndex, bool removeDefaultLight){
	lightingData.lastLightPosition.erase(lightIndex);

	modlog("voxel lighting remove: ", std::to_string(lightIndex));
	for (int i = 0 ; i < lightingData.cells.size(); i++){
		LightingCell& cell = lightingData.cells.at(i);
		if (cell.lightIndex == lightIndex){
			cell.lightIndex = -1;
			lightingData.needsUpdate.insert(i);
		}
	}
	if (removeDefaultLight && lightingData.defaultLightIndex == lightIndex){
		lightingData.defaultLightIndex = 0;
	}
	//std::cout << "voxel lighting lighting data: " << print(printDebugVoxelLighting()) << std::endl;
}

// obviously this could be more efficient
// eg could keep a mapping of cell ids to shortcut to them
void updateVoxelLightPosition(objid lightIndex, glm::vec3 position, int radius){

	// This is necessary because eg the light can rotate or sway. 
	// Expensive still if it's going to move out of the cell it is in originally.
	// Theoretically could bring down the cost eg if move by one cell, imagine a swinging light, but not sure I have that use case
	if (lightingData.lastLightPosition.find(lightIndex) != lightingData.lastLightPosition.end()){
		auto oldPosition = lightingData.lastLightPosition.at(lightIndex);
		auto oldIndex = lightingPositionToIndex(oldPosition, glm::ivec3(0.f, 0.f, 0.f));
		auto newIndex = lightingPositionToIndex(position, glm::ivec3(0.f, 0.f, 0.f));
		if (oldIndex == newIndex){
			return; 
		}
	}

	modlog("update voxel light position", std::to_string(lightIndex) + ", " + print(position));
	removeVoxelLight(lightIndex, false);
	addVoxelLight(lightIndex, position, radius);
}

void recalculateLights(std::vector<LightUpdate>& allUpdates){
	lightingData.cells = generateLightingCells(numCellsDim);
	for (auto &update : allUpdates){
		addVoxelLight(update.lightIndex, update.position, update.radius);
	}
}

int getLightingCellWidth(){
	return lightingData.voxelCellWidth;
}

int getLightingNumCellsDim(){
	return lightingData.numCellsDim;
}

int getLightingNumCellsTotal(){
	return lightingData.numCellsDim * lightingData.numCellsDim * lightingData.numCellsDim;
}

// should make not update all lights every frame only those that change
std::vector<LightingUpdate> getLightUpdates(){
	//modlog("voxel lighting updates size = : ", std::to_string(lightingData.cells.size()));
  std::vector<LightingUpdate> lightUpdates;
	for (auto cellIndex : lightingData.needsUpdate){
  	LightingCell& lightingCell = lightingData.cells.at(cellIndex);
    lightUpdates.push_back(LightingUpdate {
     	.index = cellIndex,
     	.lightIndex = lightingCell.lightIndex,
    });
	}
  return lightUpdates;
}

VoxelLightingData& getVoxelLightingData(){
	return lightingData;
}

void setGlobalLight(objid id){
	getVoxelLightingData().defaultLightIndex = id;
}

void setVoxelLighting(int voxelCellWidth, glm::vec3 worldOffset){
	getVoxelLightingData().voxelCellWidth = voxelCellWidth;
	getVoxelLightingData().offset = worldOffset;
}