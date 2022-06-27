#ifndef MOD_LAYERS
#define MOD_LAYERS

#include <vector>
#include <map>
#include "./scene/serialization.h"

std::vector<LayerInfo> parseLayerInfo(std::string file, std::function<std::string(std::string)> readFile);
void setLayerOptions(std::vector<LayerInfo>& layers, std::vector<StrValues>& values);

#endif