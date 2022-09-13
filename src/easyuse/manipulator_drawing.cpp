#include "./manipulator_drawing.h"

void drawDirectionalLine(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, glm::vec3 fromPos, glm::vec3 direction, LineColor color){
  glm::vec3 normalizedDirection = glm::normalize(direction);
  auto rotation = quatFromDirection(normalizedDirection);
  for (int i = 0; i < 10; i++){
    auto newPos = fromPos + glm::vec3(normalizedDirection.x * i, normalizedDirection.y * i, normalizedDirection.z * i);
    auto leftDash = newPos + rotation * glm::vec3(-0.1f, -0.01f, 0.5f);
    auto rightDash = newPos + rotation * glm::vec3(0.1f, -0.01f, 0.5f);
    //std::cout << "drawLine from: " << print(leftDash) << " to " << print(rightDash) << std::endl;
    drawLine(leftDash, newPos, color);
    drawLine(rightDash, newPos, color);
  }
}

void drawHitMarker(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, glm::vec3 position){
  float length = 5.f;
  drawLine(position + glm::vec3(0.f, length * 2.f, 0.f), position + glm::vec3(0.f, length * -2.f, 0.f), RED);
  drawLine(position + glm::vec3(length * 2.f, 0.f, 0.f), position + glm::vec3(length *  -2.f, 0.f, 0.f), RED);
  drawLine(position + glm::vec3(0.f, 0.f, length *  -2.f), position + glm::vec3(0.f, 0.f, length *  2.f), RED);
}

const int sphereNumPoints = 30;
void drawRotationCircle(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, glm::vec3 position, glm::quat orientation, float radius){
  float radianPerPoint = 2 * MODPI / sphereNumPoints;

  glm::vec3 lastPos(0.f, 0.f, 0.f);
  glm::vec3 firstPos(0.f, 0.f, 0.f);
  for (int i = 0; i < sphereNumPoints; i++){
    auto xPos = glm::cos(radianPerPoint * i) * radius;
    auto yPos = glm::sin(radianPerPoint * i) * radius;
    auto deltaCirclePos = orientation * glm::vec3(xPos, yPos, 0.f);
    auto newPos = position + glm::vec3(deltaCirclePos.x, deltaCirclePos.y, deltaCirclePos.z);
    if (i > 0){
      drawLine(lastPos, newPos, RED);
    }else{
      firstPos = newPos;
    }
    lastPos = newPos;
  }
  drawLine(lastPos, firstPos, RED);
}

void drawRotation(
	std::vector<IdVec3Pair> positions, 
	glm::vec3 meanPosition, 
	glm::quat rotationOrientation, 
	glm::vec3 position, 
	glm::vec3 selectDir,
	glm::vec3 intersection, float angle,
	std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine
){

  float maxDistance = 0.f;
  if (positions.size() > 1){
    for (auto &target : positions){
    	auto targetId = target.id;
      auto targetPosition = target.value;
      auto distanceToTarget = glm::distance(targetPosition, meanPosition);
      if (distanceToTarget > maxDistance){
        maxDistance = distanceToTarget;
      }
      drawRotationCircle(drawLine, meanPosition, rotationOrientation, distanceToTarget);
    }
  }

  auto lineAmount = rotationOrientation * glm::vec3(0, 0.f, -5.f);
  drawLine(meanPosition + lineAmount, meanPosition - lineAmount, GREEN);

  ////////////////////////////////////
  // visualization for the selection dir
  glm::vec3 bias(-0.01f, -0.01f, 0.f);
  drawLine(position + bias, position + glm::vec3(selectDir.x, selectDir.y, selectDir.z), RED);
  bias = glm::vec3(0.01f, 0.01f, 0.f);
  drawLine(position + bias, position + glm::vec3(selectDir.x , selectDir.y, selectDir.z ), RED);

  bias = glm::vec3(0.01f, -0.01f, 0.f);
  drawLine(position + bias, position +  glm::vec3(selectDir.x, selectDir.y, selectDir.z ), RED);
  bias = glm::vec3(-0.01f, 0.01f, 0.f);
  drawLine(position + bias, position + glm::vec3(selectDir.x , selectDir.y , selectDir.z), RED);
  drawLine(intersection, intersection + rotationOrientation * glm::vec3(0.1f, 0.1f, 0.f), RED);
  drawLine(intersection, intersection + rotationOrientation * glm::vec3(-0.1f, -0.1f, 0.f), RED);
  drawLine(intersection, intersection + rotationOrientation * glm::vec3(-0.1f, 0.1f, 0.f), RED);
  drawLine(intersection, intersection + rotationOrientation * glm::vec3(0.1f, -0.1f, 0.f), RED);

  auto angleIndicator = rotationOrientation * (glm::normalize(glm::vec3(glm::cos(angle), glm::sin(angle), 0.f)) * maxDistance);
  drawLine(meanPosition, meanPosition + angleIndicator, GREEN);
}

bool drawDebugLines = true;
void drawProjectionVisualization(std::function<void(glm::vec3, glm::vec3, LineColor)> drawLine, ProjectCursorDebugInfo& projectCursorInfo){
  drawHitMarker(drawLine, projectCursorInfo.intersectionPoint);
  if (drawDebugLines){
    drawLine(projectCursorInfo.positionFrom, projectCursorInfo.intersectionPoint, RED);
    drawLine(projectCursorInfo.positionFrom, projectCursorInfo.projectedTarget, GREEN);
    drawLine(projectCursorInfo.positionFrom, projectCursorInfo.target, BLUE);
  
    // directions
    drawDirectionalLine(drawLine, projectCursorInfo.positionFrom, projectCursorInfo.selectDir, BLUE);
    drawDirectionalLine(drawLine, projectCursorInfo.positionFrom, projectCursorInfo.targetAxis, RED);
  }
}