#ifndef MOD_BOUNDINFO
#define MOD_BOUNDINFO

#include <iostream>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include "../../../common/util.h"
#include "../../../translations.h"

struct BoundInfo {
  float xMin, xMax;
  float yMin, yMax;
  float zMin, zMax;
};

struct boundRatio {
    float xratio;
    float yratio;
    float zratio;
    float xoffset;
    float yoffset;
    float zoffset;
};

boundRatio getBoundRatio(BoundInfo info1, BoundInfo info2);
BoundInfo getMaxUnionBoundingInfo(std::vector<BoundInfo> infos);
std::string print(BoundInfo& info);
void printBoundInfo(BoundInfo info);
glm::mat4 getMatrixForBoundRatio(boundRatio ratio, glm::mat4 currentMatrix);
glm::vec3 getScaleEquivalent(BoundInfo info1, float width, float height, float depth);
BoundInfo centerBoundInfo(BoundInfo& info);
BoundInfo transformBoundInfo(BoundInfo boundInfo, glm::mat4 transform);

#endif 

