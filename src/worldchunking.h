#ifndef MOD_WORLDCHUNKING
#define MOD_WORLDCHUNKING

#include <iostream>
#include "./worldloader.h"
#include "./scene/scene_offline.h"
#include "./scene/scene.h"

void rechunkAllObjects(World& world, DynamicLoading& loadingInfo, int newchunksize);

#endif
