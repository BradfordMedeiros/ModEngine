#ifndef MOD_CUSTOMOBJ
#define MOD_CUSTOMOBJ

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <assert.h>
#include "./customobj_binding.h"
#include "./customobj_sample.h"

void registerAllBindings(std::vector<CustomObjBinding> pluginBindings);
void createCustomObj(int id, const char* name);
void removeCustomObj(int id);
void renderCustomObj(int id);

#endif