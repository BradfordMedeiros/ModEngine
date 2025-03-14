#ifndef MOD_VECTORGFX
#define MOD_VECTORGFX

#include "./util/types.h"

std::vector<Line> drawGrid3D(int numCellsWidth, float cellSize, float offsetX, float offsetY, float offsetZ);
std::vector<Line> drawCoordinateSystem(float size);
std::vector<Line> drawSphere();

#endif 
