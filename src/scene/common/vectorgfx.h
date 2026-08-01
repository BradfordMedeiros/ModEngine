#ifndef MOD_VECTORGFX
#define MOD_VECTORGFX

#include "./util/types.h"

std::vector<Line> drawGridXZ(int numCellsWidth, int numCellsHeight, float cellSize, float offsetX, float offsetY, float offsetZ, std::optional<glm::quat> orientation);
std::vector<Line> drawGridXY(int numCellsWidth, int numCellsHeight, float cellSize, float offsetX, float offsetY, float offsetZ, std::optional<glm::quat> orientation);
std::vector<Line> drawGrid3D(int numCellsWidth, float cellSize, float offsetX, float offsetY, float offsetZ);
std::vector<Line> drawCoordinateSystem(float size);
std::vector<Line> drawSphere();

#endif 
