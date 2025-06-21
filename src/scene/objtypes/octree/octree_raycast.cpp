#include "./octree_raycast.h"

extern std::optional<glm::ivec3> selectedIndex;
extern std::optional<glm::ivec3> selectionDim;
extern OctreeSelectionFace editorOrientation;
extern int subdivisionLevel;
extern std::optional<RaycastResult> raycastResult;
extern std::optional<ClosestIntersection> closestRaycast;

Faces getFaces(int x, int y, int z, float size, int subdivisionLevel){
  float adjustedSize = size * glm::pow(0.5f, subdivisionLevel);
  Faces faces {
    .XYClose = adjustedSize * 0.5f,
    .XYFar = adjustedSize * -0.5f,
    .YZLeft = adjustedSize * -0.5f, 
    .YZRight = adjustedSize * 0.5f,
    .XZTop = adjustedSize * 0.5f,
    .XZBottom = adjustedSize * -0.5f,
  };

  float width = faces.YZRight - faces.YZLeft;
  float height = faces.XZTop - faces.XZBottom;
  float depth = faces.XYClose - faces.XYFar;

  faces.center = glm::vec3(adjustedSize * x + (0.5f * width), adjustedSize * y + (0.5f * height), -1.f * (adjustedSize * z + (0.5f * depth)));

  faces.XYClosePoint = faces.center + glm::vec3(0.f, 0.f, faces.XYClose);
  faces.XYFarPoint = faces.center + glm::vec3(0.f, 0.f, faces.XYFar);
  faces.YZLeftPoint = faces.center + glm::vec3(faces.YZLeft, 0.f, 0.f);
  faces.YZRightPoint = faces.center + glm::vec3(faces.YZRight, 0.f, 0.f);
  faces.XZTopPoint = faces.center + glm::vec3(0.f, faces.XZTop, 0.f);
  faces.XZBottomPoint = faces.center + glm::vec3(0.f, faces.XZBottom, 0.f);

  return faces;
}


/* parametric form to solve 
  pos.x + dir.x * t = x
  pos.y + dir.y * t = y
  pos.z + dir.z * t = z
  so if we provide x, y, or z, we can get the other values, 
  since those values are dependent on the value
  howerver, (x,y,z) as fn(t) doesn't necessarily lie on the face,
  but it's the only possible solution at that face
*/
glm::vec3 calculateTForX(glm::vec3 pos, glm::vec3 dir, float x, float* _t){
  float t = (x - pos.x) / dir.x;
  float y = pos.y + (dir.y * t);
  float z = pos.z + (dir.z * t);
  *_t = t;
  return glm::vec3(x, y, z);
}
glm::vec3 calculateTForY(glm::vec3 pos, glm::vec3 dir, float y, float* _t){
  float t = (y - pos.y) / dir.y;
  *_t = t;
  float x = pos.x + (dir.x * t);
  float z = pos.z + (dir.z * t);
  return glm::vec3(x, y, z);
}
glm::vec3 calculateTForZ(glm::vec3 pos, glm::vec3 dir, float z, float* _t){
  float t = (z - pos.z) / dir.z;
  *_t = t;
  float x = pos.x + (dir.x * t);
  float y = pos.y + (dir.y * t);
  return glm::vec3(x, y, z);
}

bool checkIfInCube(Faces& faces, bool checkX, bool checkY, bool checkZ, glm::vec3 point){
  float minX = faces.center.x + faces.YZLeft;
  float maxX = faces.center.x + faces.YZRight;
  float minY = faces.center.y + faces.XZBottom;
  float maxY = faces.center.y + faces.XZTop;
  float minZ = faces.center.z + faces.XYFar;
  float maxZ = faces.center.z + faces.XYClose;
  if (checkX && (point.x > maxX || point.x < minX)){
    return false;
  }
  if (checkY && (point.y > maxY || point.y < minY)){
    return false;
  }
  if (checkZ && (point.z > maxZ || point.z < minZ)){
    return false;
  }
  return true;
}

bool intersectsCube(glm::vec3 fromPos, glm::vec3 toPosDirection, int x, int y, int z, float size, int subdivisionLevel, std::vector<FaceIntersection>& _faceIntersections){
  auto faces = getFaces(x, y, z, size, subdivisionLevel);

  float leftT = 0.f;
  auto intersectionLeft = calculateTForX(fromPos, toPosDirection, faces.YZLeftPoint.x, &leftT);
  bool intersectsLeftFace = checkIfInCube(faces, false, true, true, intersectionLeft);
  if (intersectsLeftFace && leftT >= 0){
    _faceIntersections.push_back(FaceIntersection {
      .face = LEFT,
      .position = intersectionLeft,
    });
  }

  float rightT = 0.f;
  auto intersectionRight = calculateTForX(fromPos, toPosDirection, faces.YZRightPoint.x, &rightT);
  bool intersectsRightFace = checkIfInCube(faces, false, true, true, intersectionRight);
  if (intersectsRightFace && rightT >= 0){
    _faceIntersections.push_back(FaceIntersection {
      .face = RIGHT,
      .position = intersectionRight,
    });
  }

  float topT = 0.f;
  auto intersectionTop = calculateTForY(fromPos, toPosDirection, faces.XZTopPoint.y, &topT);
  bool intersectsTop = checkIfInCube(faces, true, false, true, intersectionTop);
  if (intersectsTop && topT >= 0){
    _faceIntersections.push_back(FaceIntersection {
      .face = UP,
      .position = intersectionTop,
    });
  }

  float bottomT = 0.f;
  auto intersectionBottom = calculateTForY(fromPos, toPosDirection, faces.XZBottomPoint.y, &bottomT);
  bool intersectsBottom = checkIfInCube(faces, true, false, true, intersectionBottom);
  if (intersectsBottom && bottomT >= 0){
    _faceIntersections.push_back(FaceIntersection {
      .face = DOWN,
      .position = intersectionBottom,
    });
  }

  float closeT = 0.f;
  auto intersectionClose = calculateTForZ(fromPos, toPosDirection, faces.XYClosePoint.z, &closeT);
  bool intersectsClose  = checkIfInCube(faces, true, true, false, intersectionClose);
  if (intersectsClose && closeT >= 0){
    _faceIntersections.push_back(FaceIntersection {
      .face = FRONT,
      .position = intersectionClose,
    });
  }

  float farT = 0.f;
  auto intersectionFar = calculateTForZ(fromPos, toPosDirection, faces.XYFarPoint.z, &farT);
  bool intersectsFar  = checkIfInCube(faces, true, true, false, intersectionFar);
  if (intersectsFar && farT >= 0){
    _faceIntersections.push_back(FaceIntersection {
      .face = BACK,
      .position = intersectionFar,
    });
  }

  bool intersectsCube = (intersectsRightFace || intersectsLeftFace || intersectsTop || intersectsBottom || intersectsClose || intersectsFar);
  return intersectsCube; 
}

std::vector<Intersection> subdivisionIntersections(glm::vec3 fromPos, glm::vec3 toPosDirection, float size, int subdivisionLevel, glm::ivec3 offset){
  std::vector<Intersection> intersections;
  for (int x = 0; x < 2; x++){
    for (int y = 0; y < 2; y++){
      for (int z = 0; z < 2; z++){
        // notice that adjacent faces are duplicated here
        std::vector<FaceIntersection> faceIntersections;
        if (intersectsCube(fromPos, toPosDirection, x + offset.x, y + offset.y, z + offset.z, size, subdivisionLevel, faceIntersections)){
          intersections.push_back(Intersection {
            .index = xyzIndexToFlatIndex(glm::ivec3(x, y, z)),
            .faceIntersections = faceIntersections,
          });
        }
      }

    }
  }
  return intersections;
}

void raycastSubdivision(Octree& octree, glm::vec3 fromPos, glm::vec3 toPosDirection, glm::ivec3 offset, int currentSubdivision, int subdivisionDepth, std::vector<RaycastIntersection>& finalIntersections){
  modassert(currentSubdivision <= subdivisionDepth, "current subdivision should not be greater than the target subdivision depth");
  auto intersections = subdivisionIntersections(fromPos, toPosDirection, 1.f, currentSubdivision, offset);

  for (auto &intersection : intersections){
    auto xyzIndex = flatIndexToXYZ(intersection.index);
    if (currentSubdivision == subdivisionDepth){
        finalIntersections.push_back(RaycastIntersection {
          .index = intersection.index,
          .blockOffset = offset,
          .faceIntersections = intersection.faceIntersections,
        });
        continue;
    }
    glm::ivec3 newOffset = (offset + xyzIndex) * 2;
    raycastSubdivision(octree, fromPos, toPosDirection, newOffset, currentSubdivision + 1, subdivisionDepth, finalIntersections); 
  }
}

RaycastResult doRaycast(Octree& octree, glm::vec3 fromPos, glm::vec3 toPosDirection, int subdivisionDepth){
  modassert(subdivisionDepth >= 1, "subdivisionDepth must be >= 1");
  std::vector<RaycastIntersection> finalIntersections;
  raycastSubdivision(octree, fromPos, toPosDirection, glm::ivec3(0, 0, 0), 1, subdivisionDepth, finalIntersections);

  return RaycastResult {
    .fromPos = fromPos,
    .toPosDirection = toPosDirection,
    .subdivisionDepth = subdivisionDepth,
    .intersections = finalIntersections,
  };
}


std::optional<ClosestIntersection> getClosestIntersection(RaycastResult& raycastResult){
  if (raycastResult.intersections.size() == 0){
    return std::nullopt;
  }
  std::optional<ClosestIntersection> closestIntersection = std::nullopt;
  for (int i = 0; i < raycastResult.intersections.size(); i++){
    for (auto &faceIntersection : raycastResult.intersections.at(i).faceIntersections){
      if (!closestIntersection.has_value() ||
        glm::distance(faceIntersection.position, raycastResult.fromPos) < glm::distance(closestIntersection.value().position, raycastResult.fromPos) > 0){
        closestIntersection = ClosestIntersection {
          .face = faceIntersection.face,
          .position = faceIntersection.position,
          .xyzIndex = flatIndexToXYZ(raycastResult.intersections.at(i).index) + raycastResult.intersections.at(i).blockOffset,
          .subdivisionDepth = raycastResult.subdivisionDepth,
        };
      }
    }
  }
  return closestIntersection;
}

bool cellFilledIn(Octree& octree, RaycastIntersection& intersection, int subdivision){
  auto xyzIndex = flatIndexToXYZ(intersection.index) + intersection.blockOffset;;
  auto path = octreePath(xyzIndex.x, xyzIndex.y, xyzIndex.z, subdivision);
  //std::cout << "octreepath: ";
  //for (auto index : path){
  //  std::cout << "(" << index.x << ", " << index.y << ", " << index.z << "), ";
  //}
  //std::cout << std::endl;

  OctreeDivision* currentSubdivision = &octree.rootNode;
  for (auto xyzIndex : path){
    if (currentSubdivision -> divisions.size() == 0){
      return currentSubdivision -> fill == FILL_FULL;
    }
    auto index = xyzIndexToFlatIndex(xyzIndex);
    currentSubdivision = &currentSubdivision -> divisions.at(index);
  }
  return currentSubdivision -> fill == FILL_FULL;
}

RaycastResult filterFilledInCells(Octree& octree, RaycastResult& raycastResult){
  RaycastResult filteredRaycastResult {
    .fromPos = raycastResult.fromPos,
    .toPosDirection = raycastResult.toPosDirection,
    .subdivisionDepth = raycastResult.subdivisionDepth,
  };

  std::vector<RaycastIntersection> intersections;
  for (auto& intersection : raycastResult.intersections){
    if (cellFilledIn(octree, intersection, raycastResult.subdivisionDepth)){
      intersections.push_back(intersection);
    }
  }
  filteredRaycastResult.intersections = intersections;

  return filteredRaycastResult;
}

void setSelection(glm::ivec3 selection1, glm::ivec3 selection2, OctreeSelectionFace face){
  //    auto diff = closestRaycast.value().xyzIndex - newRaycast.value().xyzIndex;
  auto minXIndex = selection1.x < selection2.x ? selection1.x : selection2.x;
  auto minYIndex = selection1.y < selection2.y ? selection1.y : selection2.y;
  auto minZIndex = selection1.z < selection2.z ? selection1.z : selection2.z;

  auto maxXIndex = selection1.x > selection2.x ? selection1.x : selection2.x;
  auto maxYIndex = selection1.y > selection2.y ? selection1.y : selection2.y;
  auto maxZIndex = selection1.z > selection2.z ? selection1.z : selection2.z;

  selectedIndex = glm::ivec3(minXIndex, minYIndex, minZIndex);
  selectionDim = glm::ivec3(maxXIndex - minXIndex + 1, maxYIndex - minYIndex + 1, maxZIndex - minZIndex + 1);
  editorOrientation = face;

  std::cout << "octree raycast selection1: " << print(selection1) << ", 2 = " << print(selection2) << std::endl;
}

void handleOctreeRaycast(Octree& octree, glm::vec3 fromPos, glm::vec3 toPosDirection, bool secondarySelection, objid id){
  auto octreeRaycast = doRaycast(octree, fromPos, toPosDirection, subdivisionLevel);
  raycastResult = octreeRaycast;

  RaycastResult filteredCells = filterFilledInCells(octree, raycastResult.value());
  if (!secondarySelection){
    closestRaycast = getClosestIntersection(filteredCells);
    if (!closestRaycast.has_value()){
      return;
    }
    selectedIndex = closestRaycast.value().xyzIndex;
    setSelection(closestRaycast.value().xyzIndex, closestRaycast.value().xyzIndex, closestRaycast.value().face);
  }else if (closestRaycast.has_value()) {
    // should change selecteddim here
    auto newRaycast = getClosestIntersection(filteredCells);
    if (!newRaycast.has_value()){
      return;
    }
    setSelection(closestRaycast.value().xyzIndex, newRaycast.value().xyzIndex, closestRaycast.value().face);
  }
}