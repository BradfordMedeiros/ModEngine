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
  std::cout << "direction is: " << print(glm::vec3(direction.x, direction.y, direction.z)) << std::endl;
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

// Solves the value of the parametrized u for the ray2 parametrize equation
float calculateIntersectionU(
  float ray1FromCoord1, float ray1DirCoord1, float ray2FromCoord1, float ray2DirCoord1,
  float ray1FromCoord2, float ray1DirCoord2, float ray2FromCoord2, float ray2DirCoord2,
  bool *valid
){
  float num = (ray1DirCoord2 * (ray1FromCoord1 - ray2FromCoord1)) + (ray1DirCoord1 * (ray2FromCoord2 - ray1FromCoord2));
  if (aboutEqual(num, 0)){   // this means the vectors are parallel
    std::cout << "num is zero!" << std::endl;
  }
  float denom = (ray1DirCoord2 * ray2DirCoord1) - (ray1DirCoord1 * ray2DirCoord2); 
  if (aboutEqual(denom, 0)){   // this means the vectors are parallel
    *valid = false;
    std::cout << "denom is zero!" << std::endl;
    return 0.f;
  }
  *valid = true;
  return num / denom;
}

bool linesParallel(glm::vec3 ray1Dir, glm::vec3 ray2Dir){
  return aboutEqual(glm::length(glm::cross(ray1Dir, ray2Dir)), 0);
}

// TODO - check if there is a nice glm function i can use instead of this code
bool calcLineIntersection(glm::vec3 ray1From, glm::vec3 ray1Dir, glm::vec3 ray2From, glm::vec3 ray2Dir, glm::vec3* _intersectPoint){
  // math explanation here to derive the below equation:
  // line is represented parametrically eg
  // (1 + t, 3 + 2, 0 + t)
  // then it's like line1) x = ray1from_x + ray1Dir.x * t,  line2) x = ray2from_x + ray1Dir.x * u
  // then solve the system of equations using x and y (two equations) to get value of variable, say u, solve for t.
  // notice that x in line 1, is equal to x in line 2 when they intersect.  CalculuateIntersectionU above refers to this u value.

  // then plug those values back in to find x y and z points
  // repeat using second system of equations using x and z. 
  // verify that the point is the same

  // effectively this solves intersection by considering 2 dimensions at a time, and then verifying samenamess to get back into 3d
  // to solve system of equations, find a common multiple and then multiplying one by -1 
  // if solved you get the equation coded below
  if (aboutEqual(ray1From, ray2From)){
    *_intersectPoint = ray1From;
    return true;
  }
  if (linesParallel(ray1Dir, ray2Dir)){
    if (linesParallel(ray1Dir, ray1From - ray2From)){
     *_intersectPoint = ray1From; 
     return true;
    }
    // parellel vectors don't touch unless they both lie along the same line
    return false;
  }
  bool xyValid = false;
  auto uValueXY= calculateIntersectionU(
    ray1From.x, ray1Dir.x, ray2From.x, ray2Dir.x,
    ray1From.y, ray1Dir.y, ray2From.y, ray2Dir.y,
    &xyValid
  );
  bool xzValid = false;
  auto uValueXZ = calculateIntersectionU(
    ray1From.x, ray1Dir.x, ray2From.x, ray2Dir.x,
    ray1From.z, ray1Dir.z, ray2From.z, ray2Dir.z,
    &xzValid
  );

  if (!xyValid || !xzValid || !aboutEqual(uValueXY, uValueXZ)){
    *_intersectPoint = glm::vec3(0.f, 0.f, 0.f);
    return false;
  }
  auto xCoord = ray2From.x + (ray2Dir.x * uValueXY);
  auto yCoord = ray2From.y + (ray2Dir.y * uValueXY);
  auto zCoord = ray2From.z + (ray2Dir.z * uValueXY);
  *_intersectPoint = glm::vec3(xCoord, yCoord, zCoord);
  return true;
}

glm::vec3 projectCursorPositionOntoAxis(glm::mat4 projection, glm::mat4 view, glm::vec2 cursorPos, glm::vec2 screensize, Axis manipulatorAxis, glm::vec3 lockvalues){
  

  /*auto viewTarget = view * glm::vec4(lockvalues.x, lockvalues.y, lockvalues.z, 1.f);
  auto zValue = zDepth(0.1f, 1000.f, viewTarget.z);


  dfsdafo worldPosWrongLength = glm::unProjectNO (glm::vec3((screensize.x - cursorPos.x), cursorPos.y, zValue), view, projection, glm::vec4(0, 0, screensize.x, screensize.y));


  float screenXPosNdi = convertBase(cursorPos.x, 0.f, screensize.x, -1.f, 1.f);
  float screenYPosNdi = convertBase(cursorPos.y, 0.f, screensize.y, -1.f, 1.f);
  glm::vec2 cursorClip =  glm::vec2(screenXPosNdi, -screenYPosNdi);
  glm::vec4 actualCursor = glm::inverse(view) * glm::vec4(cursorClip.x, cursorClip.y, 0.f, 1.0f);
  glm::vec3 cursor = glm::vec3(actualCursor.x, actualCursor.y, actualCursor.z);
  auto cursorToTarget = lockvalues - cursor;
  auto cursorToTargetLength = glm::length(cursorToTarget);
  auto projLineDirection = glm::normalize(worldPosWrongLength - cursor);
  auto projectionLength = cursorToTargetLength / glm::dot(projLineDirection, glm::normalize(cursorToTarget));
  
  auto newZValue = zDepth(0.1f, 1000.f, projectionLength);
  auto worldPos = glm::unProjectNO (glm::vec3((screensize.x - cursorPos.x), cursorPos.y, newZValue), view, projection, glm::vec4(0, 0, screensize.x, screensize.y));

  if (manipulatorAxis == XAXIS){
    lockvalues.x =  worldPos.x;
  }else if (manipulatorAxis == YAXIS){
    lockvalues.y =  worldPos.y;
  }
  std::cout << std::endl;
  std::cout << "projectionLength: " << projectionLength << std::endl;
  std::cout << "cursor: " << print(cursorPos) << std::endl;
  std::cout << "screen: " << print(screensize) << std::endl;
  std::cout << "worldpos: " << print(worldPosWrongLength) << std::endl;
  */

  return lockvalues;
  //return glm::vec3(perspectiveCursorTarget.x, perspectiveCursorTarget.y, perspectiveCursorTarget.z);
}

glm::quat quatFromDirection(glm::vec3 direction){ 
  return orientationFromPos(glm::vec3(0.f, 0.f, 0.f), direction);
}
glm::vec3 directionFromQuat(glm::quat direction){
  return direction * glm::vec3(0, 1, 0);
}

