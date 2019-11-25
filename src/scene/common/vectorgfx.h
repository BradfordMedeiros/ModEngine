#ifndef MOD_VECTORGFX
#define MOD_VECTORGFX

#include "./mesh.h"

void drawCube(float width, float height, float depth);
void drawSphere(float radius);
void drawGrid(int numCellsWidth, int numCellsHeight, int cellWidth, glm::vec3 position);
void drawCoordinateSystem(float size);

#endif 
