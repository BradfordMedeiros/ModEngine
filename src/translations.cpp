#include "./translations.h"

glm::quat setFrontDelta(glm::quat orientation, float deltaYaw, float deltaPitch, float deltaRoll, float delta){ // Might be worth considering the ordering of this. 
  glm::quat pitch  = angleAxis(-1 * deltaPitch * delta, glm::vec3(1.0f, 0.0f, 0.0f)) ;
  glm::quat yaw = glm::angleAxis(-1 * deltaYaw * delta,  glm::vec3(0.0f, 1.0f, 0.0f));
  glm::quat roll = glm::angleAxis(-1 * deltaRoll * delta,  glm::vec3(0.0f, 0.0f, 1.0f));
  return roll * (yaw * (orientation * pitch));                                      
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

// https://gamedev.stackexchange.com/questions/9693/whats-a-good-way-to-check-that-a-player-has-clicked-on-an-object-in-a-3d-game
RotationDirection getCursorInfoWorldNdi(glm::mat4 projection, glm::mat4 view, float screenXPosNdi, float screenYPosNdi, float zDistance){
  glm::vec3 positionFrom = glm::inverse(view) * glm::vec4(0.f, 0.f, 0.f, 1.0);
  auto viewDir = glm::inverse(view) * glm::vec4(0.f, 0.f, -1.f, 0.f);
  glm::vec4 projectionCoordViewSpace = glm::inverse(projection) * glm::vec4(screenXPosNdi, screenYPosNdi, 0.f, 0.f);
  projectionCoordViewSpace.z = 1.f;

  glm::vec4 finalCoordViewSpace = glm::vec4(projectionCoordViewSpace.x * zDistance, projectionCoordViewSpace.y * zDistance, -1 * projectionCoordViewSpace.z * zDistance, 0.f);
  auto finalPosition = glm::inverse(view) * glm::vec4(finalCoordViewSpace.x, finalCoordViewSpace.y, finalCoordViewSpace.z, 1.f);
  auto finalDirection = glm::normalize(glm::vec3(finalPosition.x, finalPosition.y, finalPosition.z) - positionFrom);

  return RotationDirection {
    .position = positionFrom ,
    .direction = finalDirection,
    .viewDir = glm::normalize(glm::vec3(viewDir.x, viewDir.y, viewDir.z)),
    .projectedPosition = finalPosition,
  };
}

RotationDirection getCursorInfoWorld(glm::mat4 projection, glm::mat4 view, float cursorLeft, float cursorBottom, float screenWidth, float screenHeight, float zDistance){
  float screenXPosNdi = convertBase(cursorLeft, 0.f, screenWidth, -1.f, 1.f);
  float screenYPosNdi = convertBase(cursorBottom, 0.f, screenHeight, -1.f, 1.f);
  return getCursorInfoWorldNdi(projection, view, screenXPosNdi, screenYPosNdi, zDistance);
}

float zDepth(float nearPlane, float farPlane, float depth){
  return ((1 / depth) - (1 / nearPlane)) / ((1 / farPlane) - (1 / nearPlane));
}

glm::vec3 projectCursorAtDepth(glm::mat4 projection, glm::mat4 view, float nearPlane, float farPlane, glm::vec2 cursorPos, glm::vec2 screensize, float depth){
  auto zValue = zDepth(nearPlane, farPlane, depth);  
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


float currentMouseDepth();
glm::vec3 projectCursorPositionOntoAxis(glm::mat4 projection, glm::mat4 view, glm::vec2 cursorPos, glm::vec2 screensize, Axis manipulatorAxis, glm::vec3 lockvalues, ProjectCursorDebugInfo* _debugInfo, std::optional<glm::quat> orientation){
  auto positionFrom4 = glm::inverse(view) * glm::vec4(0.f, 0.f, 0.f, 1.0);
  glm::vec3 positionFrom(positionFrom4.x, positionFrom4.y, positionFrom4.z);
  auto selectDir = glm::normalize(getCursorInfoWorld(projection, view, cursorPos.x, cursorPos.y, screensize.x, screensize.y, currentMouseDepth()).direction);
  glm::vec3 target(lockvalues.x, lockvalues.y, lockvalues.z);
  glm::vec3 targetDir(0.f, 0.f, 0.f);
  if (manipulatorAxis == XAXIS){
    targetDir = glm::vec3(1.f, 0.f, 0.f);
  }else if (manipulatorAxis == YAXIS){
    targetDir = glm::vec3(0.f, 1.f, 0.f);
  }else if (manipulatorAxis == ZAXIS){
    targetDir = glm::vec3(0.f, 0.f, -1.f);
  }
  targetDir = glm::normalize(targetDir);

  auto dirToTarget = glm::normalize(target - positionFrom);             // where you click on the manipulator + toward the tard, a triangle
  auto distanceToTarget = glm::distance(positionFrom, target);          // not a right triangle, so use law of sines to solve

  auto dotRadian1 = glm::dot(selectDir, targetDir);
  auto radians = glm::acos(dotRadian1);

  auto dotRadian2 = glm::dot(dirToTarget, selectDir);
  auto radians2 = glm::acos(dotRadian2);

  auto value = glm::sin(radians2) * distanceToTarget / glm::sin(radians); 
  
  glm::vec3 offset(0.f, 0.f, 0.f);

  if (manipulatorAxis == XAXIS){
    if (dirToTarget.x > selectDir.x){
      offset.x = -(value * targetDir.x);
    }else{
      offset.x = (value * targetDir.x);
    }
  }
  if (manipulatorAxis == YAXIS){
    if (dirToTarget.y > selectDir.y){
      offset.y = -(value * targetDir.y);
    }else{
      offset.y = (value * targetDir.y);
    }
  }
  if (manipulatorAxis == ZAXIS){
    if (dirToTarget.z > selectDir.z){
      offset.z = (value * targetDir.z);
    }else{
      offset.z = -(value * targetDir.z);
    }
  }

  auto finalPosition = target + offset;
  if (_debugInfo != NULL){
    _debugInfo -> positionFrom = positionFrom;
    _debugInfo -> projectedTarget = target;
    _debugInfo -> target = target;
    _debugInfo -> intersectionPoint = finalPosition;
    _debugInfo -> selectDir = selectDir;
    _debugInfo -> targetAxis = targetDir;
  }
  return finalPosition;
}

glm::quat quatFromDirection(glm::vec3 direction){ 
  return orientationFromPos(glm::vec3(0.f, 0.f, 0.f), direction);
}
glm::vec3 directionFromQuat(glm::quat direction){
  return direction * glm::vec3(0, 0, -1.f);
}

/*
old implementation should be equivalent but the one i kept measures faster
glm::mat4 matrixFromComponents(glm::mat4 initialModel, glm::vec3 position, glm::vec3 scale, glm::quat rotation){
  glm::mat4 modelMatrix = glm::translate(initialModel, position);
  modelMatrix = modelMatrix * glm::toMat4(rotation);
  glm::mat4 scaledModelMatrix = modelMatrix * glm::scale(glm::mat4(1.f), scale);
  return scaledModelMatrix;
}*/
glm::mat4 matrixFromComponents(glm::mat4 initialModel, glm::vec3 position, glm::vec3 scale, glm::quat rotation) {
  glm::mat4 rotationMatrix = glm::toMat4(rotation);
  rotationMatrix[0] *= scale.x;
  rotationMatrix[1] *= scale.y;
  rotationMatrix[2] *= scale.z;
  glm::mat4 transform = glm::translate(glm::mat4(1.0f), position) * rotationMatrix;
  return initialModel * transform;
}

// This is equivalent to the above effectively, but it seems to be a bit faster when written out. 
// Can just replace with a call to the above if does not help in measurements
// This optimization is mainly done for the call in the animation function
glm::mat4 matrixFromComponents(Transformation& transformation) {
    const glm::quat& q = transformation.rotation;
    const glm::vec3& s = transformation.scale;
    const glm::vec3& t = transformation.position;

    float x2 = q.x + q.x;
    float y2 = q.y + q.y;
    float z2 = q.z + q.z;

    float xx = q.x * x2;
    float yy = q.y * y2;
    float zz = q.z * z2;

    float xy = q.x * y2;
    float xz = q.x * z2;
    float yz = q.y * z2;

    float wx = q.w * x2;
    float wy = q.w * y2;
    float wz = q.w * z2;

    glm::mat4 result(1.0f);

    result[0][0] = (1.0f - (yy + zz)) * s.x;
    result[0][1] = (xy + wz) * s.x;
    result[0][2] = (xz - wy) * s.x;
    result[0][3] = 0.0f;

    result[1][0] = (xy - wz) * s.y;
    result[1][1] = (1.0f - (xx + zz)) * s.y;
    result[1][2] = (yz + wx) * s.y;
    result[1][3] = 0.0f;

    result[2][0] = (xz + wy) * s.z;
    result[2][1] = (yz - wx) * s.z;
    result[2][2] = (1.0f - (xx + yy)) * s.z;
    result[2][3] = 0.0f;

    result[3] = glm::vec4(t, 1.0f);

    return result;
}

void modDecompose(glm::mat4& matrix, glm::vec3& pos, glm::quat& rot, glm::vec3& scale){
  glm::vec3 skew;
  glm::vec4 perspective;
  glm::decompose(matrix, scale, rot, pos, skew, perspective);
}

Transformation getTransformationFromMatrix(glm::mat4 matrix){
  glm::vec3 scale;
  glm::quat rotation;
  glm::vec3 translation;
  modDecompose(matrix, translation, rotation, scale);
  Transformation transform = {
    .position = translation,
    .scale = scale,
    .rotation = rotation,
  };
  return transform;  
}

RotationPosition rotateOverAxis(RotationPosition object, RotationPosition axis, float rotationRadians, std::optional<std::function<glm::quat(glm::quat)>> snapRotate){
  auto fromAxis = object.position - axis.position;
  auto objectTranslation = glm::translate(glm::mat4(1.f), fromAxis);
  auto axisRotation = glm::rotate(glm::mat4(1.f), -1 * rotationRadians, directionFromQuat(axis.rotation));

  auto quatRotation = getTransformationFromMatrix(axisRotation).rotation;
  if (snapRotate.has_value()){
    quatRotation = snapRotate.value()(quatRotation);
  }
  axisRotation = glm::toMat4(quatRotation);

  auto axisTransform = glm::translate(glm::mat4(1.f), axis.position);
  auto transform = getTransformationFromMatrix(axisTransform * axisRotation * objectTranslation * glm::toMat4(object.rotation));

  return RotationPosition {
    .position = transform.position,
    .rotation = transform.rotation,
  }; 
}

// todo add diagram/explanation of this math (parametric representation of line, plane equation w/ substitutions)
std::optional<glm::vec3> findPlaneIntersection(glm::vec3 anypointOnPlane, glm::vec3 planeNormal, glm::vec3 rayPosition, glm::vec3 rayDirection){
  auto normalConstant = planeNormal.x * anypointOnPlane.x + planeNormal.y * anypointOnPlane.y + planeNormal.z * anypointOnPlane.z;
  auto tSum = planeNormal.x * rayDirection.x + planeNormal.y * rayDirection.y + planeNormal.z * rayDirection.z;
  auto constantSum = (planeNormal.x * rayPosition.x)  + (planeNormal.y * rayPosition.y) + (planeNormal.z * rayPosition.z);
  auto tValue = (normalConstant - constantSum) / tSum;
  auto xFuncT = rayDirection.x  * tValue + rayPosition.x;
  auto yFuncT = rayDirection.y * tValue + rayPosition.y;
  auto zFuncT = rayDirection.z * tValue + rayPosition.z;

  auto tValuesNormal = (planeNormal.x * xFuncT) + (planeNormal.y * yFuncT) + (planeNormal.z * zFuncT) - normalConstant;
  if (!aboutEqual(tValuesNormal, 0.f)){
    std::cout << "tvaluesnormal: " << tValuesNormal << std::endl;
    return std::nullopt;
  }
  return glm::vec3(xFuncT, yFuncT, zFuncT);
}

glm::quat axisToOrientation(Axis axis){
  if (axis == XAXIS){
    return MOD_ORIENTATION_RIGHT;
  }
  if (axis == YAXIS){
    return MOD_ORIENTATION_UP;
  }
  if (axis == ZAXIS){
    return MOD_ORIENTATION_FORWARD;
  }
  modassert(false, "axis to orientation invalid orientation");
  return glm::identity<glm::quat>();
}

float atanRadians360(float x, float y){
  auto angle = glm::atan(y / x);
  if (x < 0){
    return angle + MODPI;
  }
  return angle;
}

glm::vec3 calcOffsetFromRotation(glm::vec3 position, std::optional<glm::vec3> offset, glm::quat rotation){
  if (!offset.has_value()){
    return position;
  }
  return position + rotation * offset.value();
}

glm::vec3 calcOffsetFromRotationReverse(glm::vec3 position, std::optional<glm::vec3> offset, glm::quat rotation){
  if (!offset.has_value()){
    return position;
  }
  return position - rotation * offset.value();
}

std::string print(Transformation& transform){
  return std::string(" [pos = " + print(transform.position) + ", scale = " + print(transform.scale) + ", rot = " + serializeQuat(transform.rotation) +  "]");
}

bool testOnlyAboutEqual(Transformation& transform1, Transformation& transform2){
  return aboutEqual(transform1.position, transform2.position) && aboutEqual(transform1.scale, transform2.scale);
}

glm::vec2 uvToNdi(glm::vec2 coord){
  float xCoord = convertBase(coord.x, 0, 1, -1, 1);
  float yCoord = convertBase(coord.y, 0, 1, -1, 1);
  return glm::vec2(xCoord, yCoord);
}

FovAngles calcFovAngles(LayerInfo& layer, glm::ivec2 viewportSize){
  auto fieldOfView = 0.5f * (layer.fov);

  auto aspect = ((float)viewportSize.x) / ((float)viewportSize.y);
  return FovAngles {
    .x = glm::radians(fieldOfView * aspect),
    .y = glm::radians(fieldOfView),
    .zNear = layer.nearplane,
    .zFar = layer.farplane,
  };
}

ViewFrustum cameraToViewFrustum(LayerInfo& layer, glm::ivec2 viewportSize){
  // All planes have the camera locations as a point if you extend them 
  // except for the near and far plane
  auto angles = calcFovAngles(layer, viewportSize);

  float widthXNear = angles.zNear * glm::tan(angles.x);
  float widthYNear = angles.zNear * glm::tan(angles.y);

  auto topLeft = glm::normalize(glm::vec3(-widthXNear, widthYNear, -angles.zNear));
  auto topRight = glm::normalize(glm::vec3(widthXNear, widthYNear, -angles.zNear));
  auto bottomLeft = glm::normalize(glm::vec3(-widthXNear, -widthYNear, -angles.zNear));
  auto bottomRight = glm::normalize(glm::vec3(widthXNear, -widthYNear, -angles.zNear));

  // think right hand rule for direction, should be pointing inwards
  auto topNormal = glm::cross(topLeft, topRight);
  auto bottomNormal = glm::cross(bottomRight, bottomLeft);
  auto leftNormal = glm::cross(bottomLeft, topLeft);
  auto rightNormal = glm::cross(topRight, bottomRight);

  return ViewFrustum {
    .left = FrustumPlane {
      .point = glm::vec3(0.f, 0.f, 0.f),
      .normal = leftNormal,
    },
    .right = FrustumPlane {
      .point = glm::vec3(0.f, 0.f, 0.f),
      .normal = rightNormal,
    },
    .top = FrustumPlane {
      .point = glm::vec3(0.f, 0.f, 0.f),
      .normal = topNormal,
    },
    .bottom = FrustumPlane {
      .point = glm::vec3(0.f, 0.f, 0.f),
      .normal = bottomNormal,
    },
    .near = FrustumPlane {
      .point = glm::vec3(0.f, 0.f, -1 * angles.zNear),
      .normal = glm::vec3(0.f, 0.f, -1.f),
    },
    .far = FrustumPlane {
      .point = glm::vec3(0.f, 0.f, -1 * angles.zFar),
      .normal = glm::vec3(0.f, 0.f, 1.f),
    },
  };
}

// the point here should be in camera / frustum) space
bool isInFrontOfPlane(FrustumPlane& plane, glm::vec3 point){
  auto vecToPoint = point - plane.point;
  return glm::dot(vecToPoint, plane.normal) >= 0;
}

bool passesFrustumCulling(ViewFrustum& viewFrustum, Transformation& camera, ModAABB2& aabb){
  auto bounds = toBounds(aabb);

  std::vector<glm::vec3> points {
    bounds.topLeftFront,
    bounds.topRightFront,
    bounds.bottomLeftFront,
    bounds.bottomRightFront,
    bounds.topLeftBack,
    bounds.topRightBack,
    bounds.bottomLeftBack,
    bounds.bottomRightBack,
  };


  int numPointsOnLeft = 0;
  int numPointsOnRight = 0;
  int numPointsOnFar = 0;
  int numPointsOnNear = 0;
  int numPointsOnTop = 0;
  int numPointsOnBottom = 0;

  for (auto &boundingPoint : points){
    // this needs to check the AABB and if all the points are not in is then can cull
    auto point = glm::inverse(camera.rotation) * (boundingPoint - camera.position) ; // this gives us the world space point of 0,0,-1 in camera space
    auto inFrontOfNearPlane = isInFrontOfPlane(viewFrustum.near, point);
    if (!inFrontOfNearPlane){
      numPointsOnNear++;
    }
    auto inFrontOfFarPlane = isInFrontOfPlane(viewFrustum.far, point);
    if (!inFrontOfFarPlane){
      numPointsOnFar++;
    }
    auto inFrontOfLeftPlane = isInFrontOfPlane(viewFrustum.left, point);
    if (!inFrontOfLeftPlane){
      numPointsOnLeft++;
    }
    auto inFrontOfRightPlane = isInFrontOfPlane(viewFrustum.right, point);
    if (!inFrontOfRightPlane){
      numPointsOnRight++;
    }
    auto inFrontOfTopPlane = isInFrontOfPlane(viewFrustum.top, point);
    if (!inFrontOfTopPlane){
      numPointsOnTop++;
    }
    auto inFrontOfBottomPlane = isInFrontOfPlane(viewFrustum.bottom, point);
    if (!inFrontOfBottomPlane){
      numPointsOnBottom++;
    }
  
    auto passesCulling = inFrontOfNearPlane && inFrontOfFarPlane && inFrontOfLeftPlane && inFrontOfRightPlane && inFrontOfTopPlane && inFrontOfBottomPlane;
    if (passesCulling){
      return true;
    } 
  }

  if(numPointsOnLeft == 8){
    return false;
  }
  if(numPointsOnRight == 8){
    return false;

  }
  if(numPointsOnFar == 8){
    return false;

  }
  if(numPointsOnNear == 8){
    return false;

  }
  if(numPointsOnTop == 8){
    return false;

  }
  if(numPointsOnBottom == 8){
    return false;
  }
  return true;
}