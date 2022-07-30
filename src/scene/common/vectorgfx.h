#ifndef MOD_VECTORGFX
#define MOD_VECTORGFX

#include "./mesh.h"

void drawCube(float width, float height, float depth);
int drawSphere();  // returns # of verts drawn
void drawGridXY(int numCellsWidth, int numCellsHeight, float cellSize, float offsetX = 0, float offsetY = 0, float offsetZ = 0);
void drawGridXZ(int numCellsWidth, int numCellsHeight, float cellSize, float offsetX = 0, float offsetY = 0, float offsetZ = 0);
void drawGridYZ(int numCellsWidth, int numCellsHeight, float cellSize, float offsetX = 0, float offsetY = 0, float offsetZ = 0);

void drawGrid3D(int numCellsWidth, float cellSize, float offsetX = 0, float offsetY = 0, float offsetZ = 0);
void drawCoordinateSystem(float size);

#endif 
