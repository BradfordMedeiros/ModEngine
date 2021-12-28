#ifndef MOD_WORLDCHUNKING
#define MOD_WORLDCHUNKING

#include <iostream>
#include "./worldloader.h"
#include "./scene/scene_offline.h"
#include "./scene/scene.h"

void rechunkAllCells(World& world, DynamicLoading& loadingInfo, int newchunksize, SysInterface interface);

#endif
