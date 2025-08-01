#include "./octree_vector.h"

extern std::optional<glm::ivec3> selectedIndex;
extern std::optional<glm::ivec3> selectionDim;
extern int subdivisionLevel;
extern std::optional<Line> line;
extern std::optional<RaycastResult> raycastResult;
extern std::optional<ClosestIntersection> closestRaycast;

void drawPhysicsBlock(PositionAndScale& physicShape, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
  auto leftX = physicShape.position.x;
  auto rightX = physicShape.position.x + physicShape.size.x;
  auto topY = physicShape.position.y + physicShape.size.y;
  auto bottomY = physicShape.position.y;
  auto farZ = physicShape.position.z - physicShape.size.z;
  auto nearZ = physicShape.position.z;

  drawLine(glm::vec3(leftX, bottomY, nearZ), glm::vec3(rightX, bottomY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(glm::vec3(leftX, topY, nearZ), glm::vec3(rightX, topY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));

  drawLine(glm::vec3(leftX, bottomY, farZ), glm::vec3(rightX, bottomY, farZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(glm::vec3(leftX, topY, farZ), glm::vec3(rightX, topY, farZ), glm::vec4(1.f, 0.f, 0.f, 1.f));

  drawLine(glm::vec3(leftX, topY, farZ), glm::vec3(leftX, topY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(glm::vec3(rightX, topY, farZ), glm::vec3(rightX, topY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));

  drawLine(glm::vec3(leftX, bottomY, farZ), glm::vec3(leftX, bottomY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(glm::vec3(rightX, bottomY, farZ), glm::vec3(rightX, bottomY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));

  drawLine(glm::vec3(rightX, bottomY, farZ), glm::vec3(rightX, topY, farZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(glm::vec3(leftX, bottomY, farZ), glm::vec3(leftX, topY, farZ), glm::vec4(1.f, 0.f, 0.f, 1.f));

  drawLine(glm::vec3(rightX, bottomY, nearZ), glm::vec3(rightX, topY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(glm::vec3(leftX, bottomY, nearZ), glm::vec3(leftX, topY, nearZ), glm::vec4(1.f, 0.f, 0.f, 1.f));
}

void drawPhysicsShape(std::vector<glm::vec3>& verts, glm::vec3& centeringOffset, Transformation& transform, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
  modassert(verts.size() % 3 == 0, "expected verts to be a multiple of 3");
  auto scale = transform.scale * 2.f;
  for (int i = 0; i < verts.size() - 1; i+=3){
    auto pos1 = verts.at(i);
    auto pos2 = verts.at(i + 1);
    auto pos3 = verts.at(i + 2);

    pos1 = pos1 + centeringOffset;
    pos2 = pos2 + centeringOffset;
    pos3 = pos3 + centeringOffset;

    pos1 *= scale;  // * 2 since communicated to physics in half extents
    pos2 *= scale;  // * 2 since communicated to physics in half extents
    pos3 *= scale;  // * 2 since communicated to physics in half extents

    pos1 = transform.rotation * pos1 + transform.position;
    pos2 = transform.rotation * pos2 + transform.position;
    pos3 = transform.rotation * pos3 + transform.position;

    auto rotatedScaledCentering = transform.rotation * (centeringOffset * scale);

    if (rotatedScaledCentering.x < 0){
      rotatedScaledCentering.x *= -1;
    }
    if (rotatedScaledCentering.y < 0){
      rotatedScaledCentering.y *= -1;
    }
    if (rotatedScaledCentering.z < 0){
      rotatedScaledCentering.z *= -1;
    }
    rotatedScaledCentering.z *= -1; 

    //std::cout << "values: c = " << print(centeringOffset) << ", s = " << print(scale) << ", r  = " << print(rotatedScale) << ", rc = " << print(rotatedCentering) << ", rsc = " << print(rotatedScaledCentering) << std::endl;

    pos1 += rotatedScaledCentering;  // why doesn't this need the rotation added back in ? 
    pos2 += rotatedScaledCentering;
    pos3 += rotatedScaledCentering;

    drawLine(pos1, pos2, glm::vec4(1.f, 0.f, 0.f, 1.f));
    drawLine(pos1, pos3, glm::vec4(1.f, 0.f, 0.f, 1.f));
    drawLine(pos2, pos3, glm::vec4(1.f, 0.f, 0.f, 1.f));
  }
}

void drawPhysicsShapes(PhysicsShapes& physicsShapes, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
  for (auto &block : physicsShapes.blocks){
    drawPhysicsBlock(block, drawLine);
  }
  for (auto &shape : physicsShapes.shapes){
    for (auto &transform : shape.specialBlocks){
      drawPhysicsShape(shape.verts, shape.centeringOffset, transform, drawLine);
    }
  }
}

void drawGridSelectionXY(int x, int y, int z, int numCellsWidth, int numCellsHeight, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, std::optional<OctreeSelectionFace> face){
  float cellSize = size * glm::pow(0.5f, subdivision);

  float offsetX = x * cellSize;
  float offsetY = y * cellSize;
  float offsetZ = -1 * z * cellSize;
  glm::vec3 offset(offsetX, offsetY, offsetZ);

  glm::vec4 color(0.f, 0.f, 1.f, 1.f);
  drawLine(offset + glm::vec3(0.f, 0.f, 0.f), offset + glm::vec3(numCellsWidth * cellSize, 0.f, 0.f), color);
  drawLine(offset + glm::vec3(0.f, 0.f, 0.f), offset + glm::vec3(0.f, numCellsHeight * cellSize, 0.f), color);
  drawLine(offset + glm::vec3(0.f, numCellsHeight * cellSize, 0.f), offset + glm::vec3(numCellsWidth * cellSize, numCellsHeight * cellSize, 0.f), color);
  drawLine(offset + glm::vec3(numCellsWidth * cellSize, 0.f, 0.f), offset + glm::vec3(numCellsWidth * cellSize, numCellsHeight * cellSize, 0.f), color);
}

void drawGridSelectionYZ(int x, int y, int z, int numCellsHeight, int numCellsDepth, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, std::optional<OctreeSelectionFace> face){
  float cellSize = size * glm::pow(0.5f, subdivision);

  float offsetX = x * cellSize;
  float offsetY = y * cellSize;
  float offsetZ = -1 * z * cellSize;
  glm::vec3 offset(offsetX, offsetY, offsetZ);

  glm::vec4 color(0.f, 0.f, 1.f, 1.f);
  drawLine(offset + glm::vec3(0.f, 0.f, 0.f), offset + glm::vec3(0.f, 0.f, -1 * numCellsDepth * cellSize), color);
  drawLine(offset + glm::vec3(0.f, 0.f, 0.f), offset + glm::vec3(0.f, numCellsHeight * cellSize, 0.f), color);
  drawLine(offset + glm::vec3(0.f, numCellsHeight * cellSize, 0.f), offset + glm::vec3(0.f, numCellsHeight * cellSize, -1 * numCellsDepth * cellSize), color);
  drawLine(offset + glm::vec3(0.f, 0.f, -1 * numCellsDepth * cellSize), offset + glm::vec3(0.f, numCellsHeight * cellSize, -1 * numCellsDepth * cellSize), color);
}

void drawGridSelectionCube(int x, int y, int z, int numCellsWidth, int numCellsHeight, int numCellDepth, int subdivision, float size, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine, std::optional<OctreeSelectionFace> face){
  drawGridSelectionXY(x, y, z,     1, 1, subdivision, size, drawLine, face);
  drawGridSelectionXY(x, y, z + 1, 1, 1, subdivision, size, drawLine, face);
  drawGridSelectionYZ(x, y, z, 1, 1, subdivision, size, drawLine, face);
  drawGridSelectionYZ(x + 1, y, z, 1, 1, subdivision, size, drawLine, face);
}

void visualizeFaces(Faces& faces, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine){
  drawLine(faces.center, faces.YZLeftPoint, glm::vec4(1.f, 0.f, 0.f, 1.f));
  drawLine(faces.center, faces.YZRightPoint, glm::vec4(0.f, 1.f, 0.f, 1.f));

  drawLine(faces.center, faces.XZTopPoint, glm::vec4(0.f, 1.f, 0.f, 1.f));
  drawLine(faces.center, faces.XZBottomPoint, glm::vec4(0.f, 0.f, 1.f, 1.f));

  drawLine(faces.center, faces.XYClosePoint, glm::vec4(1.f, 1.f, 0.f, 1.f));
  drawLine(faces.center, faces.XYFarPoint, glm::vec4(0.f, 1.f, 1.f, 1.f));
}

bool drawRaycastLine = false;
bool drawAllSelectedBlocks = false;
void drawOctreeSelectionGrid(Octree& octree, std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLine2, glm::mat4 modelMatrix){
  std::function<void(glm::vec3, glm::vec3, glm::vec4)> drawLineModel = [drawLine2, &modelMatrix](glm::vec3 fromPos, glm::vec3 toPos, glm::vec4 color) -> void {
    glm::vec4 fromPosVec4(fromPos.x, fromPos.y, fromPos.z, 1.f);
    glm::vec4 toPosVec4(toPos.x, toPos.y, toPos.z, 1.f);
    auto transformedFrom = modelMatrix * fromPosVec4;
    auto transformedTo = modelMatrix * toPosVec4;

    drawLine2(
      glm::vec3(transformedFrom.x, transformedFrom.y, transformedFrom.z), 
      glm::vec3(transformedTo.x, transformedTo.y, transformedTo.z), 
      color
    );
  };

  if (selectedIndex.has_value()){
    drawLineModel(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 5.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 1.f));
    drawGridSelectionXY(selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, selectionDim.value().x, selectionDim.value().y, subdivisionLevel, 1.f,  drawLineModel, std::nullopt);
    if (selectionDim.value().z > 0){
      drawGridSelectionXY(selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z + selectionDim.value().z, selectionDim.value().x, selectionDim.value().y, subdivisionLevel, 1.f,  drawLineModel, std::nullopt);
    }
    if (line.has_value()){
      drawLineModel(line.value().fromPos, line.value().toPos, glm::vec4(1.f, 0.f, 0.f, 1.f));
    }
  }

  //auto faces = getFaces(selectedIndex.value().x, selectedIndex.value().y, selectedIndex.value().z, testOctree.size, subdivisionLevel);
  //visualizeFaces(faces, drawLine);

  if (raycastResult.has_value()){
    auto dirOffset = glm::normalize(raycastResult.value().toPosDirection);
    dirOffset.x *= 20;
    dirOffset.y *= 20;
    dirOffset.z *= 20;
    drawLineModel(raycastResult.value().fromPos, raycastResult.value().fromPos + dirOffset, glm::vec4(1.f, 1.f, 1.f, 1.f));

    if (drawAllSelectedBlocks){
      for (auto intersection : raycastResult.value().intersections){
        auto xyzIndex = flatIndexToXYZ(intersection.index);
        drawGridSelectionCube(xyzIndex.x + intersection.blockOffset.x, xyzIndex.y + intersection.blockOffset.y, xyzIndex.z + intersection.blockOffset.z, 1, 1, 1, raycastResult.value().subdivisionDepth, 1.f, drawLineModel, std::nullopt);    
        // draw hit marker on the point
        for (auto &face : intersection.faceIntersections){
          drawLineModel(face.position, face.position + glm::vec3(0.f, 0.2f, 0.f), glm::vec4(0.f, 1.f, 0.f, 1.f));
        }
      }
    }
  }

  if (false && closestRaycast.has_value()){
    drawGridSelectionCube(closestRaycast.value().xyzIndex.x, closestRaycast.value().xyzIndex.y, closestRaycast.value().xyzIndex.z, 1, 1, 1, closestRaycast.value().subdivisionDepth, 1.f, drawLineModel, std::nullopt);    
    drawLineModel(closestRaycast.value().position, closestRaycast.value().position + glm::vec3(0.f, 0.2f, 0.f), glm::vec4(0.f, 0.f, 1.f, 1.f));
  }

  ////// visualize the physics objects
  //auto physicsShapes = getPhysicsShapes(octree);
  //drawPhysicsShapes(physicsShapes, drawLine2);
}
