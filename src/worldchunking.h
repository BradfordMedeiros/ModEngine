#ifndef MOD_WORLDCHUNKING
#define MOD_WORLDCHUNKING

#include <iostream>
#include "./worldloader.h"
#include "./scene/scene_offline.h"
#include "./scene/scene.h"

std::string serializeVoxelDefault(World& world, Voxels& voxelData);
void rechunkAllVoxels(World& world, DynamicLoading& loadingInfo, int newchunksize);
void rechunkAllObjects(World& world, DynamicLoading& loadingInfo, int newchunksize);

#endif
