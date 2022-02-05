#include "./translations.h"

glm::quat setFrontDelta(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta){ // Might be worth considering the ordering of this. 
  glm::quat pitch  = angleAxis(-1 * deltaPitch * delta, glm::vec3(1.0f, 0.0f, 0.0f)) ;
  glm::quat yaw = glm::angleAxis(-1 * deltaYaw * delta,  glm::vec3(0.0f, 1.0f, 0.0f));
  glm::quat roll = glm::angleAxis(-1 * deltaRoll * delta,  glm::vec3(0.0f, 0.0f, 1.0f));
  return roll * yaw * orientation * pitch;                                      
}

glm::vec3 calculateRelativeOffset(glm::quat orientation, glm::vec3 offset, bool xzPlaneOnly){
  glm::vec3 delta = orientation * offset;
  return (xzPlaneOnly ? glm::vec3(delta.x, 0, delta.z) : delta);
}

glm::vec3 moveRelative(glm::vec3 position, glm::quat orientation, glm::vec3 offset, bool xzPlaneOnly){
  return position + calculateRelativeOffset(orientation, offset, xzPlaneOnly);   // @TODO This loses magnitude if you do have upwards velocity.  This should really be just projecting offset onto xz plane
}

glm::mat4 renderView(glm::vec3 position, glm::quat orientation){
  glm::mat4 cameraModelMatrix =  glm::translate(glm::mat4(1.f), glm::vec3(position.x, position.y, position.z)) * glm::toMat4(orientation);
  return glm::inverse(cameraModelMatrix);
}

glm::vec3 getVecAxis(Axis axis){
  if (axis == XAXIS){
    return glm::vec3(1.0f, 0.f, 0.f);
  }else if (axis == YAXIS){
    return glm::vec3(0.f, 1.0f, 0.f);
  }else if (axis == ZAXIS){
    return glm::vec3(0.f, 0.f, 1.0f);
  }
  return glm::vec3(1.0f, 0.0f, 1.0f);
}
glm::vec3 getVecTranslate(float offsetX, float offsetY){
  return glm::vec3(-offsetX, offsetY, offsetY);
}
glm::vec3 applyTranslation(glm::vec3 position, float offsetX, float offsetY, Axis manipulatorAxis){
  glm::vec3 axis = getVecAxis(manipulatorAxis);
  glm::vec3 translate = getVecTranslate(offsetX, offsetY);
  return position + axis * translate;
}
glm::vec3 applyScaling(glm::vec3 position, glm::vec3 initialScale, float lastX, float lastY, float offsetX, float offsetY, Axis manipulatorAxis){
  // The scaling operation needs to figure out if you should make it bigger by making it 
  // more positive, or by making it more negative.  Therefore, need to use the objects position 
  float distanceOld = glm::distance(position, glm::vec3(lastX, lastY, 0));
  float distanceNew = glm::distance(position, glm::vec3(lastX + offsetX, lastY + offsetY, 0));

  if (distanceNew < distanceOld){
    return initialScale - getVecAxis(manipulatorAxis) * 0.1f;
  }else if (distanceNew > distanceOld){
    return initialScale + getVecAxis(manipulatorAxis) * 0.1f;
  }else{ 
    return initialScale;
  }
}
glm::quat applyRotation(glm::quat currentOrientation, float offsetX, float offsetY, Axis manipulatorAxis){
  float deltaPitch = manipulatorAxis == XAXIS ? offsetY : 0;
  float deltaYaw = manipulatorAxis == YAXIS ? offsetX : 0;
  float deltaRoll = manipulatorAxis == ZAXIS ? offsetX : 0;
  return setFrontDelta(currentOrientation, deltaYaw, deltaPitch, deltaRoll, 0.1);
}

float convertBase(float value, float fromBaseLow, float fromBaseHigh, float toBaseLow, float toBaseHigh){
  return ((value - fromBaseLow) * ((toBaseHigh - toBaseLow) / (fromBaseHigh - fromBaseLow))) + toBaseLow;
}

glm::vec3 getCursorRayDirection(glm::mat4 projection, glm::mat4 view, float cursorLeft, float cursorTop, float screenWidth, float screenHeight){
  glm::mat4 inversionMatrix = glm::inverse(projection * view);
  float screenXPosNdi = convertBase(cursorLeft, 0.f, screenWidth, -1.f, 1.f);
  float screenYPosNdi = convertBase(cursorTop, 0.f, screenHeight, -1.f, 1.f);
  glm::vec4 direction = inversionMatrix * glm::vec4(screenXPosNdi, -screenYPosNdi, 1.0f, 1.0f);
  //std::cout << "direction is: " << print(glm::vec3(direction.x, direction.y, direction.z)) << std::endl;
  return glm::normalize(glm::vec3(direction.x, direction.y, direction.z));
}

float zDepth(float nearPlane, float farPlane, float depth){
  return ((1 / depth) - (1 / nearPlane)) / ((1 / farPlane) - (1 / nearPlane));
}

glm::vec3 projectCursorAtDepth(glm::mat4 projection, glm::mat4 view, float nearPlane, float farPlane, glm::vec2 cursorPos, glm::vec2 screensize, float depth){
  auto zValue = zDepth(nearPlane, farPlane, depth);  // don't use hardcoded clip planes
  auto worldpos = glm::unProjectNO (glm::vec3((screensize.x - cursorPos.x), cursorPos.y, zValue), view, projection, glm::vec4(0, 0, screensize.x, screensize.y));
  return worldpos;
}

bool linesParallel(glm::vec3 ray1Dir, glm::vec3 ray2Dir){
  return aboutEqual(glm::length(glm::cross(ray1Dir, ray2Dir)), 0);
}

glm::vec3 calcIntersectionFromT(glm::vec3 from, glm::vec3 dir, float t){
  auto x = from.x + (dir.x * t);
  auto y = from.y + (dir.y * t);
  auto z = from.z + (dir.z * t);
  return glm::vec3(x, y, z);
}

// TODO - check if there is a nice glm function i can use instead of this code
bool calcLineIntersection(glm::vec3 ray1From, glm::vec3 ray1Dir, glm::vec3 ray2From, glm::vec3 ray2Dir, glm::vec3* _intersectPoint){
  // math explanation here to derive the below equation:
  glm::mat2 matrix1XY(1.f);
  matrix1XY[0][0] = ray1Dir.x;
  matrix1XY[1][0] = -1 * ray2Dir.x;
  matrix1XY[0][1] = ray1Dir.y;
  matrix1XY[1][1] = -1 * ray2Dir.y;

  glm::mat2 matrix1XZ(1.f);
  matrix1XZ[0][0] = ray1Dir.x;
  matrix1XZ[1][0] = -1 * ray2Dir.x;
  matrix1XZ[0][1] = ray1Dir.z;
  matrix1XZ[1][1] = -1 * ray2Dir.z;

  glm::vec2 initialPointsXY(1.f, 1.f);
  initialPointsXY.x = ray2From.x - ray1From.x;
  initialPointsXY.y = ray2From.y - ray1From.y;

  glm::vec2 initialPointsXZ(1.f, 1.f);
  initialPointsXZ[0] = ray2From.x - ray1From.x;
  initialPointsXZ[1] = ray2From.z - ray1From.z;

  auto matrixXYInverse = glm::inverse(matrix1XY);
  auto XYisInvertible = (matrixXYInverse * matrix1XY) == glm::mat2(1.f);

  auto matrixXZInverse = glm::inverse(matrix1XZ);
  auto XZIsInvertible = (matrixXZInverse * matrix1XZ) == glm::mat2(1.f);


  std::vector<std::pair<float, float>> solutions;
   // then instead here, check if either was invertible, and then use those points and as the solution
  // if both were, verify that the values are the same 
  if (XYisInvertible){
    glm::vec2 solution = matrixXYInverse * initialPointsXY;
    float t = solution.x;
    float u = solution.y;
    solutions.push_back({t, u});
  }
  if (XZIsInvertible){
    glm::vec2 solution2 = matrixXZInverse * initialPointsXZ;
    float t = solution2.x;
    float u = solution2.y;
    solutions.push_back({t, u});
  }

  if (solutions.size() == 1){
    // this should check if the t value works for the other point here i think
 //   std::cout << "todo check secondary solution" << std::endl;
    auto rayOneIntersectT = calcIntersectionFromT(ray1From, ray1Dir, solutions.at(0).first);
    auto rayTwoIntersectU = calcIntersectionFromT(ray2From, ray2Dir, solutions.at(0).second);
    if (!aboutEqual(rayOneIntersectT, rayTwoIntersectU)){
      std::cout << "0: t and u are providing different solutions" << std::endl;
      std::cout << "ray1from, ray1dir: " << print(ray1From) << ", " << print(ray1Dir) << std::endl;
      std::cout << "ray2from, ray2dir: " << print(ray2From) << ", " << print(ray2Dir) << std::endl;
      std::cout << "(t, u) - " << solutions.at(0).first << " - " << solutions.at(0).second << std::endl;
      std::cout << "from t: " << print(rayOneIntersectT) << std::endl;
      std::cout << "from u: " << print(rayTwoIntersectU) << std::endl;
      return false;
    }
    *_intersectPoint = rayOneIntersectT;
    return true;
  }else if (solutions.size() == 2){
    if (!aboutEqual(solutions.at(0).first, solutions.at(1).first) || !aboutEqual(solutions.at(0).second, solutions.at(1).second)){
      return false;
    }
    auto rayOneIntersectT = calcIntersectionFromT(ray1From, ray1Dir, solutions.at(0).first);
    auto rayTwoIntersectU = calcIntersectionFromT(ray2From, ray2Dir, solutions.at(0).second);
    if (!aboutEqual(rayOneIntersectT, rayTwoIntersectU)){
      std::cout << "1: t and u are providing different solutions" << std::endl;
      std::cout << "ray1from, ray1dir: " << print(ray1From) << ", " << print(ray1Dir) << std::endl;
      std::cout << "ray2from, ray2dir: " << print(ray2From) << ", " << print(ray2Dir) << std::endl;
      std::cout << "(t, u) - " << solutions.at(0).first << " - " << solutions.at(0).second << std::endl;
      std::cout << "from t: " << print(rayOneIntersectT) << std::endl;
      std::cout << "from u: " << print(rayTwoIntersectU) << std::endl;
      return false;
    }
    *_intersectPoint = rayOneIntersectT;
    return true;
  }

  if (aboutEqual(ray1From, ray2From)){
    *_intersectPoint = ray1From;
    return true;
  }
  auto ray1ToRay2Dir = ray2From - ray1From;
  if (linesParallel(ray1ToRay2Dir, ray1Dir)){
    *_intersectPoint = ray1From;
    return true;
  }
  if (linesParallel(ray1ToRay2Dir, ray2Dir)){
    *_intersectPoint = ray1From;
    return true;
  }
  return false;
}

glm::vec3 projectCursorPositionOntoAxis(glm::mat4 projection, glm::mat4 view, glm::vec2 cursorPos, glm::vec2 screensize, Axis manipulatorAxis, glm::vec3 lockvalues, ProjectCursorDebugInfo* _debugInfo){
  auto positionFrom4 = glm::inverse(view) * glm::vec4(0.f, 0.f, 0.f, 1.0);
  glm::vec3 positionFrom(positionFrom4.x, positionFrom4.y, positionFrom4.z);
  auto selectDir = getCursorRayDirection(projection, view, cursorPos.x, cursorPos.y, screensize.x, screensize.y);
  glm::vec3 target(lockvalues.x, lockvalues.y, lockvalues.z);

  if (manipulatorAxis == XAXIS){
    target.y = positionFrom.y;
    selectDir.y = 0.f;
  }
  if (manipulatorAxis == YAXIS){
   // target.x = positionFrom.x;
   // selectDir.x = 0.f;
  }
  if (manipulatorAxis == ZAXIS){
    target.x = positionFrom.x;
    selectDir.x = 0.f;
  }

  glm::vec3 targetDir(0.f, 0.f, 0.f);
  if (manipulatorAxis == XAXIS){
    targetDir = glm::vec3(1.f, 0.f, 0.f);
  }else if (manipulatorAxis == YAXIS){
    std::cout << "setting to y axis!" << std::endl;
    targetDir = glm::vec3(0.f, 1.f, 0.f);
  }else if (manipulatorAxis == ZAXIS){
    targetDir = glm::vec3(0.f, 0.f, -1.f);
  }

  glm::vec3 projectedPoint(0.f, 0.f, 0.f);

  bool lineIntersects = calcLineIntersection(positionFrom, selectDir, target, targetDir, &projectedPoint);
  if (!lineIntersects){
    std::cout << "does not intersect!" << std::endl;
    return lockvalues;
  }
  if (manipulatorAxis == XAXIS){
    lockvalues.x = projectedPoint.x;
  }
  if (manipulatorAxis == YAXIS){
    lockvalues.y = projectedPoint.y;
  }
  if (manipulatorAxis == ZAXIS){
    lockvalues.z = projectedPoint.z;
  }

  if (_debugInfo != NULL){
    _debugInfo -> ray1From = positionFrom;
    _debugInfo -> ray1Dir = selectDir;
    _debugInfo -> ray2From = target;
    _debugInfo -> ray2Dir = targetDir;
    _debugInfo -> intersectionPoint = projectedPoint;
    _debugInfo -> intersects = lineIntersects;
  }
  return lockvalues;
}

glm::quat quatFromDirection(glm::vec3 direction){ 
  return orientationFromPos(glm::vec3(0.f, 0.f, 0.f), direction);
}
glm::vec3 directionFromQuat(glm::quat direction){
  return direction * glm::vec3(0, 1, 0);
}

