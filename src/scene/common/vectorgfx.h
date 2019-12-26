#ifndef MOD_VECTORGFX
#define MOD_VECTORGFX

#include "./mesh.h"

void drawCube(float width, float height, float depth);
void drawSphere();
void drawGrid(int numCellsWidth, int numCellsHeight, int cellWidth, int offsetX = 0, int offsetY = 0, int offsetZ = 0);
void drawGridHorizontal(int numCellsWidth, int numCellsHeight, int cellWidth, int offsetX = 0, int offsetY = 0, int offsetZ = 0);

void drawGrid3D();
void drawCoordinateSystem(float size);

#endif 
