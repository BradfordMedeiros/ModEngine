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

std::vector<glm::vec4> boundInfoToPoints(BoundInfo boundInfo){
  std::vector<glm::vec4> points;
  points.push_back(glm::vec4(boundInfo.xMin, boundInfo.yMin, boundInfo.zMin, 1.f));
  points.push_back(glm::vec4(boundInfo.xMax, boundInfo.yMin, boundInfo.zMin, 1.f));
  points.push_back(glm::vec4(boundInfo.xMin, boundInfo.yMax, boundInfo.zMin, 1.f));
  points.push_back(glm::vec4(boundInfo.xMax, boundInfo.yMax, boundInfo.zMin, 1.f));
  points.push_back(glm::vec4(boundInfo.xMin, boundInfo.yMin, boundInfo.zMax, 1.f));
  points.push_back(glm::vec4(boundInfo.xMax, boundInfo.yMin, boundInfo.zMax, 1.f));
  points.push_back(glm::vec4(boundInfo.xMin, boundInfo.yMax, boundInfo.zMax, 1.f));
  points.push_back(glm::vec4(boundInfo.xMax, boundInfo.yMax, boundInfo.zMax, 1.f));
  return points;
}

// This should take into account things being centered (or not)
BoundInfo getMaxUnionBoundingInfo(std::vector<BoundInfo> infos){
  if (infos.size() == 0){
    return BoundInfo { .xMin = 0.f, .xMax = 0.f, .yMin = 0.f, .yMax = 0.f, .zMin = 0.f, .zMax = 0.f };
  }
  BoundInfo info = infos.at(0);
  for (int i = 1; i < infos.size(); i++){
    auto currInfo = infos.at(i);
    if (currInfo.xMin < info.xMin){
      info.xMin = currInfo.xMin;
    }
    if (currInfo.yMin < info.yMin){
      info.yMin = currInfo.yMin;
    }
    if (currInfo.zMin < info.zMax){
      info.zMin = currInfo.zMin;
    }
    if (currInfo.xMax > info.xMax){
      info.xMax = currInfo.xMax;
    }
    if (currInfo.yMax > info.yMax){
      info.yMax = currInfo.yMax;
    }
    if (currInfo.zMax > info.zMax){
      info.zMax = currInfo.zMax;
    }
  }
  return info;
}

BoundInfo centerBoundInfo(BoundInfo& currInfo){
  auto halfWidth = (currInfo.xMax - currInfo.xMin) / 2.f;
  auto halfHeight = (currInfo.yMax - currInfo.yMin) / 2.f;
  auto halfDepth = (currInfo.zMax - currInfo.zMin) / 2.f;
  return BoundInfo{
    .xMin = -1 * halfWidth, .xMax = halfWidth,
    .yMin = -1 * halfHeight, .yMax = halfHeight,
    .zMin = -1 * halfDepth, .zMax = halfDepth,
  };
}

BoundInfo transformBoundInfo(BoundInfo boundInfo, glm::mat4 transform){
  auto boundInfoPoints =  boundInfoToPoints(boundInfo);
  auto firstPointTransformed =  transform * boundInfoPoints.at(0);
  float minX = firstPointTransformed.x;
  float maxX = firstPointTransformed.x;
  float minY = firstPointTransformed.y;
  float maxY = firstPointTransformed.y;
  float minZ = firstPointTransformed.z;
  float maxZ = firstPointTransformed.z;

  for (auto point : boundInfoPoints){
    glm::vec4 transformedPoint = transform * point;
    if (transformedPoint.x < minX){
      minX = transformedPoint.x;
    }
    if (transformedPoint.x > maxX){
      maxX = transformedPoint.x;
    }
    if (transformedPoint.y < minY){
      minY = transformedPoint.y;
    }
    if (transformedPoint.y > maxY){
      maxY = transformedPoint.y;
    }
    if (transformedPoint.z < minZ){
      minZ = transformedPoint.z;
    }
    if (transformedPoint.z > maxZ){
      maxZ = transformedPoint.z;
    }  
  } 
  return BoundInfo { .xMin = minX, .xMax = maxX, .yMin = minY, .yMax = maxY, .zMin = minZ, .zMax = maxZ };
}

