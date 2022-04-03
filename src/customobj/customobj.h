#ifndef MOD_CUSTOMOBJ
#define MOD_CUSTOMOBJ

#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include <assert.h>

void registerAllBindings();
void createCustomObj(int id, const char* name);
void removeCustomObj(int id);
void renderCustomObj(int id);

#endif