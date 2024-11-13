#ifndef MOD_VECTORGFX
#define MOD_VECTORGFX

#include "./mesh.h"

void drawCube(float width, float height, float depth);
void drawGridXY(int numCellsWidth, int numCellsHeight, float cellSize, float offsetX = 0, float offsetY = 0, float offsetZ = 0, std::optional<glm::quat> orientation = std::nullopt);
void drawGridXZ(int numCellsWidth, int numCellsHeight, float cellSize, float offsetX = 0, float offsetY = 0, float offsetZ = 0, std::optional<glm::quat> orientation = std::nullopt);
void drawGridYZ(int numCellsWidth, int numCellsHeight, float cellSize, float offsetX = 0, float offsetY = 0, float offsetZ = 0, std::optional<glm::quat> orientation = std::nullopt);

void drawGrid3D(int numCellsWidth, float cellSize, float offsetX = 0, float offsetY = 0, float offsetZ = 0);
void drawCoordinateSystem(float size);

std::vector<Line> drawSphere();

#endif 
