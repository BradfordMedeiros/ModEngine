#include "./boundinfo.h"

boundRatio getBoundRatio(BoundInfo info1, BoundInfo info2){
  float xRatio = (info2.xMax - info2.xMin) / (info1.xMax - info1.xMin);
  float yRatio = (info2.yMax - info2.yMin) / (info1.yMax - info1.yMin);
  float zRatio = (info2.zMax - info2.zMin) / (info1.zMax - info1.zMin);

  float xScaleMidInfo1 = xRatio * (info1.xMax + info1.xMin) / 2;
  float yScaleMidInfo1 = yRatio * (info1.yMax + info1.yMin) / 2;
  float zScaleMidInfo1 = zRatio * (info1.zMax + info1.zMin) / 2;

  float xMidInfo2 = (info2.xMax + info2.xMin) / 2;
  float yMidInfo2 = (info2.yMax + info2.yMin) / 2;
  float zMidInfo2 = (info2.zMax + info2.zMin) / 2;

  float xoffset = xScaleMidInfo1 - xMidInfo2;
  float yoffset = yScaleMidInfo1 - yMidInfo2;
  float zoffset = zScaleMidInfo1 - zMidInfo2;

  boundRatio boundingInfo = {
    .xratio = xRatio,
    .yratio = yRatio,
    .zratio = zRatio,
    .xoffset = -xoffset,
    .yoffset = -yoffset,
    .zoffset = -zoffset,
  };
  return boundingInfo;
}

void printBoundInfo(BoundInfo info){
  std::cout << "x: <" << info.xMin << " | " << info.xMax << ">" << std::endl;
  std::cout << "y: <" << info.yMin << " | " << info.yMax << ">" << std::endl;
  std::cout << "z: <" << info.zMin << " | " << info.zMax << ">" << std::endl;
}

glm::mat4 getMatrixForBoundRatio(boundRatio ratio, glm::mat4 currentMatrix){
  return glm::scale(glm::translate(currentMatrix, glm::vec3(ratio.xoffset, ratio.yoffset, ratio.zoffset)), glm::vec3(ratio.xratio, ratio.yratio, ratio.zratio));
}

glm::vec3 getScaleEquivalent(BoundInfo info1, float width, float height, float depth){ 
  BoundInfo unitInfo = {
    .xMin = 0, .xMax = width,
    .yMin = 0, .yMax = height,
    .zMin = 0, .zMax = depth
  };

  boundRatio ratio = getBoundRatio(info1, unitInfo);

  return glm::vec3(ratio.xratio, ratio.yratio, ratio.zratio);
}