#include "./vectorgfx.h"

void drawCube(float width, float height, float depth){
  std::vector<Line> allLines;                    
  allLines.push_back(Line { .fromPos = glm::vec3(0, 0, 0),           .toPos = glm::vec3(width, 0, 0)          });
  allLines.push_back(Line { .fromPos = glm::vec3(0,  height, 0),     .toPos = glm::vec3(width, height, 0)     });
  allLines.push_back(Line { .fromPos = glm::vec3(0,  0, depth),      .toPos = glm::vec3(width, 0, depth)      });
  allLines.push_back(Line { .fromPos = glm::vec3(0,  height, depth), .toPos = glm::vec3(width, height, depth) });
  allLines.push_back(Line { .fromPos = glm::vec3(0, 0, 0),           .toPos = glm::vec3(0, height, 0)         });
  allLines.push_back(Line { .fromPos = glm::vec3(width, 0, 0),       .toPos = glm::vec3(width, height, 0)     });
  allLines.push_back(Line { .fromPos = glm::vec3(0, 0, depth),       .toPos = glm::vec3(0, height, depth)     });
  allLines.push_back(Line { .fromPos = glm::vec3(width, 0, depth),   .toPos = glm::vec3(width, height, depth) });
  allLines.push_back(Line { .fromPos = glm::vec3(0, 0, 0),           .toPos = glm::vec3(0, 0, depth)          });
  allLines.push_back(Line { .fromPos = glm::vec3(0, height, 0),      .toPos = glm::vec3(0, height, depth)     });
  allLines.push_back(Line { .fromPos = glm::vec3(width, 0, 0),       .toPos = glm::vec3(width, 0, depth)      });
  allLines.push_back(Line { .fromPos = glm::vec3(width, height, 0),  .toPos = glm::vec3(width, height, depth) });
  drawLines(allLines);
}

int drawSphere(){                  // lots of repeat code here, should generalize
  static unsigned int resolution = 30;
  std::vector<Line> allLines;

  float lastX  = 1;
  float lastY = 0;
  for (unsigned int i = 1; i <= resolution;  i++){
    float radianAngle = i * ((2 * M_PI) / resolution);
    float x = cos(radianAngle);
    float y = sin(radianAngle);
    allLines.push_back(Line{ .fromPos = glm::vec3(lastX, lastY, 0), .toPos = glm::vec3(x, y, 0) });
    lastX = x;
    lastY = y;
  }
  
  lastX  = 1;
  lastY = 0;
  for (unsigned int i = 1; i <= resolution;  i++){
    float radianAngle = i * ((2 * M_PI) / resolution);
    float x = cos(radianAngle);
    float y = sin(radianAngle);
    allLines.push_back(Line{ .fromPos = glm::vec3(lastX, 0, lastY), .toPos = glm::vec3(x, 0, y) });
    lastX = x;
    lastY = y;
  }

  lastX  = 1;
  lastY = 0;
  for (unsigned int i = 1; i <= resolution;  i++){
    float radianAngle = i * ((2 * M_PI) / resolution);
    float x = cos(radianAngle);
    float y = sin(radianAngle);
    allLines.push_back(Line{ .fromPos = glm::vec3(0, lastX, lastY), .toPos = glm::vec3(0, x, y) });
    lastX = x;
    lastY = y;
  }

  return drawLines(allLines);
}

void applyOrientationToLines(std::vector<Line>& allLines, std::optional<glm::quat> orientation){
  if (!orientation.has_value()){
    return;
  }
  for (auto &line : allLines){
    line.fromPos = orientation.value() * line.fromPos;
    line.toPos = orientation.value() * line.toPos;
  }
}

void drawGridXY(int numCellsWidth, int numCellsHeight, float cellSize, float offsetX, float offsetY, float offsetZ, std::optional<glm::quat> orientation){   
  float centeringOffsetX = -1.f * (cellSize * numCellsWidth) / 2.f;
  float centeringOffsetY = -1.f * (cellSize * numCellsHeight) / 2.f;

  std::vector<Line> allLines;
  for (unsigned int i = 0 ; i < (numCellsHeight + 1); i++){
    allLines.push_back(Line {                                                   
      .fromPos = glm::vec3(centeringOffsetX + offsetX, centeringOffsetY + (i * cellSize) + offsetY, 0 + offsetZ), 
      .toPos = glm::vec3(centeringOffsetX + (numCellsWidth * cellSize) + offsetX, centeringOffsetY + (i * cellSize) + offsetY, 0 + offsetZ),
    });
  }
  for (unsigned int i = 0 ; i < (numCellsWidth + 1); i++){
    allLines.push_back(Line { 
      .fromPos = glm::vec3(centeringOffsetX + (i * cellSize) + offsetX, centeringOffsetY + offsetY, 0 + offsetZ), 
      .toPos = glm::vec3(centeringOffsetX + (i * cellSize) + offsetX, centeringOffsetY + (numCellsHeight * cellSize) + offsetY, 0 + offsetZ),
    });
  }
  applyOrientationToLines(allLines, orientation);
  drawLines(allLines);
}

void drawGridXZ(int numCellsWidth, int numCellsHeight, float cellSize, float offsetX, float offsetY, float offsetZ, std::optional<glm::quat> orientation){
  float centeringOffsetX = -1.f * (cellSize * numCellsWidth) / 2.f;
  float centeringOffsetZ = -1.f * (cellSize * numCellsHeight) / 2.f;

  std::vector<Line> allLines;
  for (unsigned int i = 0 ; i < (numCellsHeight + 1); i++){
    allLines.push_back(Line {                                                   
      .fromPos = glm::vec3(centeringOffsetX + offsetX, offsetY, centeringOffsetZ + (i * cellSize) + offsetZ), 
      .toPos = glm::vec3(centeringOffsetX + (numCellsWidth * cellSize) + offsetX, offsetY, centeringOffsetZ + (i * cellSize) + offsetZ),
    });
  }
  for (unsigned int i = 0 ; i < (numCellsWidth + 1); i++){
    allLines.push_back(Line { 
      .fromPos = glm::vec3(centeringOffsetX + (i * cellSize) + offsetX, 0 + offsetY, centeringOffsetZ + offsetZ), 
      .toPos = glm::vec3(centeringOffsetX + (i * cellSize) + offsetX, offsetY, centeringOffsetZ + (numCellsHeight * cellSize) +  offsetZ),
    });
  }
  applyOrientationToLines(allLines, orientation);
  drawLines(allLines);
}

void drawGridYZ(int numCellsWidth, int numCellsHeight, float cellSize, float offsetX, float offsetY, float offsetZ, std::optional<glm::quat> orientation){
  float centeringOffsetY = -1.f * (cellSize * numCellsWidth) / 2.f;
  float centeringOffsetZ = -1.f * (cellSize * numCellsHeight) / 2.f;

  std::vector<Line> allLines;
  for (unsigned int i = 0 ; i < (numCellsHeight + 1); i++){
    allLines.push_back(Line {                                                   
      .fromPos = glm::vec3(offsetX, centeringOffsetY + offsetY, centeringOffsetZ + (i * cellSize) + offsetZ), 
      .toPos = glm::vec3(offsetX, (numCellsWidth * cellSize) + centeringOffsetY + offsetY, centeringOffsetZ + (i * cellSize) + offsetZ),
    });
  }
  for (unsigned int i = 0 ; i < (numCellsWidth + 1); i++){
    allLines.push_back(Line { 
      .fromPos = glm::vec3(offsetX, (i * cellSize) + centeringOffsetY + offsetY, centeringOffsetZ + offsetZ), 
      .toPos = glm::vec3(offsetX, (i * cellSize) + centeringOffsetY + offsetY, centeringOffsetZ + (numCellsHeight * cellSize) +  offsetZ),
    });
  }
  applyOrientationToLines(allLines, orientation);
  drawLines(allLines);
}

void drawGrid3D(int numCellsWidth, float cellSize, float offsetX, float offsetY, float offsetZ){
  for (int i = 0; i <= numCellsWidth; i++){
    float centeredOffsetY = -1.f * (numCellsWidth * cellSize) / 2.f;
    drawGridXZ(numCellsWidth, numCellsWidth, cellSize, offsetX, centeredOffsetY + i * cellSize + offsetY, offsetZ, std::nullopt);
  }
  for (int i = 0; i <= numCellsWidth; i++){
    float centeredOffsetZ = -1.f * (numCellsWidth * cellSize) / 2.f;
    drawGridXY(numCellsWidth, numCellsWidth, cellSize, offsetX, offsetY, centeredOffsetZ + i * cellSize + offsetZ, std::nullopt);
  }
}

void drawGrid3DCentered(int numCellsWidth, float cellSize, float offsetX, float offsetY, float offsetZ){
  float offset = numCellsWidth * cellSize / 2.f;
  drawGrid3D(numCellsWidth, cellSize, -offset + offsetX, -offset + offsetY, -offset + offsetZ);
}

void drawCoordinateSystem(float size){
  std::vector<Line> allLines;
  allLines.push_back(Line { .fromPos = glm::vec3(-1.f * size, 0.f, 0.f), .toPos = glm::vec3(1.f * size, 0.f, 0.f) });
  allLines.push_back(Line { .fromPos = glm::vec3(0.f, -1.f * size, 0.f), .toPos = glm::vec3(0.f, 1.f * size, 0.f) });
  allLines.push_back(Line { .fromPos = glm::vec3(0.f, 0.f, -1.f * size), .toPos = glm::vec3(0.f, 0.f, 1.f * size) });
  drawLines(allLines);
}