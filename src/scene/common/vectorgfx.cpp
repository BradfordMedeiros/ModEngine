#include "./vectorgfx.h"


static glm::mat4 modelMatrix(1.f);
void drawCube(GLint shaderProgram, float width, float height, float depth){
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
  drawLines(shaderProgram, allLines, 5, modelMatrix);
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

void drawGridXY(GLint shaderProgram, int numCellsWidth, int numCellsHeight, float cellSize, float offsetX, float offsetY, float offsetZ, std::optional<glm::quat> orientation){   
  float centeringOffsetX = -1.f * (cellSize * numCellsWidth) / 2.f;
  float centeringOffsetY = -1.f * (cellSize * numCellsHeight) / 2.f;

  std::vector<Line> allLines;
  for (unsigned int i = 0 ; i < (numCellsHeight + 1); i++){
    allLines.push_back(Line {                                                   
      .fromPos = glm::vec3(centeringOffsetX, centeringOffsetY + (i * cellSize), 0), 
      .toPos = glm::vec3(centeringOffsetX + (numCellsWidth * cellSize), centeringOffsetY + (i * cellSize), 0),
    });
  }
  for (unsigned int i = 0 ; i < (numCellsWidth + 1); i++){
    allLines.push_back(Line { 
      .fromPos = glm::vec3(centeringOffsetX + (i * cellSize), centeringOffsetY, 0), 
      .toPos = glm::vec3(centeringOffsetX + (i * cellSize), centeringOffsetY + (numCellsHeight * cellSize), 0),
    });
  }
  applyOrientationToLines(allLines, orientation);
  for (auto &line : allLines){
    line.fromPos.x += offsetX;
    line.fromPos.y += offsetY;
    line.fromPos.z += offsetZ;
    line.toPos.x += offsetX;
    line.toPos.y += offsetY;
    line.toPos.z += offsetZ;
  }
  drawLines(shaderProgram, allLines, 5, modelMatrix);
}

void drawGridXZ(GLint shaderProgram, int numCellsWidth, int numCellsHeight, float cellSize, float offsetX, float offsetY, float offsetZ, std::optional<glm::quat> orientation){
  float centeringOffsetX = -1.f * (cellSize * numCellsWidth) / 2.f;
  float centeringOffsetZ = -1.f * (cellSize * numCellsHeight) / 2.f;

  std::vector<Line> allLines;
  for (unsigned int i = 0 ; i < (numCellsHeight + 1); i++){
    allLines.push_back(Line {                                                   
      .fromPos = glm::vec3(centeringOffsetX, 0.f, centeringOffsetZ + (i * cellSize) + 0.f), 
      .toPos = glm::vec3(centeringOffsetX + (numCellsWidth * cellSize), 0.f, centeringOffsetZ + (i * cellSize)),
    });
  }
  for (unsigned int i = 0 ; i < (numCellsWidth + 1); i++){
    allLines.push_back(Line { 
      .fromPos = glm::vec3(centeringOffsetX + (i * cellSize), 0 , centeringOffsetZ), 
      .toPos = glm::vec3(centeringOffsetX + (i * cellSize), 0.f, centeringOffsetZ + (numCellsHeight * cellSize)),
    });
  }

  applyOrientationToLines(allLines, orientation);
  for (auto &line : allLines){
    line.fromPos.x += offsetX;
    line.fromPos.y += offsetY;
    line.fromPos.z += offsetZ;
    line.toPos.x += offsetX;
    line.toPos.y += offsetY;
    line.toPos.z += offsetZ;
  }
  drawLines(shaderProgram, allLines, 5, modelMatrix);
}

void drawGridYZ(GLint shaderProgram, int numCellsWidth, int numCellsHeight, float cellSize, float offsetX, float offsetY, float offsetZ, std::optional<glm::quat> orientation){
  float centeringOffsetY = -1.f * (cellSize * numCellsWidth) / 2.f;
  float centeringOffsetZ = -1.f * (cellSize * numCellsHeight) / 2.f;

  std::vector<Line> allLines;
  for (unsigned int i = 0 ; i < (numCellsHeight + 1); i++){
    allLines.push_back(Line {                                                   
      .fromPos = glm::vec3(0.f, centeringOffsetY, centeringOffsetZ + (i * cellSize)), 
      .toPos = glm::vec3(0.f, (numCellsWidth * cellSize) + centeringOffsetY, centeringOffsetZ + (i * cellSize)),
    });
  }
  for (unsigned int i = 0 ; i < (numCellsWidth + 1); i++){
    allLines.push_back(Line { 
      .fromPos = glm::vec3(0.f, (i * cellSize) + centeringOffsetY, centeringOffsetZ), 
      .toPos = glm::vec3(0.f, (i * cellSize) + centeringOffsetY, centeringOffsetZ + (numCellsHeight * cellSize)),
    });
  }
  applyOrientationToLines(allLines, orientation);
  for (auto &line : allLines){
    line.fromPos.x += offsetX;
    line.fromPos.y += offsetY;
    line.fromPos.z += offsetZ;
    line.toPos.x += offsetX;
    line.toPos.y += offsetY;
    line.toPos.z += offsetZ;
  }
  drawLines(shaderProgram, allLines, 5, modelMatrix);
}

void drawGrid3D(GLint shaderProgram, int numCellsWidth, float cellSize, float offsetX, float offsetY, float offsetZ){
  for (int i = 0; i <= numCellsWidth; i++){
    float centeredOffsetY = -1.f * (numCellsWidth * cellSize) / 2.f;
    drawGridXZ(shaderProgram, numCellsWidth, numCellsWidth, cellSize, offsetX, centeredOffsetY + i * cellSize + offsetY, offsetZ, std::nullopt);
  }
  for (int i = 0; i <= numCellsWidth; i++){
    float centeredOffsetZ = -1.f * (numCellsWidth * cellSize) / 2.f;
    drawGridXY(shaderProgram, numCellsWidth, numCellsWidth, cellSize, offsetX, offsetY, centeredOffsetZ + i * cellSize + offsetZ, std::nullopt);
  }
}

void drawGrid3DCentered(GLint shaderProgram, int numCellsWidth, float cellSize, float offsetX, float offsetY, float offsetZ){
  float offset = numCellsWidth * cellSize / 2.f;
  drawGrid3D(shaderProgram, numCellsWidth, cellSize, -offset + offsetX, -offset + offsetY, -offset + offsetZ);
}

void drawCoordinateSystem(GLint shaderProgram, float size){
  std::vector<Line> allLines;
  allLines.push_back(Line { .fromPos = glm::vec3(-1.f * size, 0.f, 0.f), .toPos = glm::vec3(1.f * size, 0.f, 0.f) });
  allLines.push_back(Line { .fromPos = glm::vec3(0.f, -1.f * size, 0.f), .toPos = glm::vec3(0.f, 1.f * size, 0.f) });
  allLines.push_back(Line { .fromPos = glm::vec3(0.f, 0.f, -1.f * size), .toPos = glm::vec3(0.f, 0.f, 1.f * size) });
  drawLines(shaderProgram, allLines, 5, modelMatrix);
}

std::vector<Line> drawSphere(){
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

  return allLines;
}