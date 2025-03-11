#ifndef MOD_VECTORGFX
#define MOD_VECTORGFX

#include "./mesh.h"

void drawCube(GLint shaderProgram, float width, float height, float depth);
void drawGridXY(GLint shaderProgram, int numCellsWidth, int numCellsHeight, float cellSize, float offsetX = 0, float offsetY = 0, float offsetZ = 0, std::optional<glm::quat> orientation = std::nullopt);
void drawGridXZ(GLint shaderProgram, int numCellsWidth, int numCellsHeight, float cellSize, float offsetX = 0, float offsetY = 0, float offsetZ = 0, std::optional<glm::quat> orientation = std::nullopt);
void drawGridYZ(GLint shaderProgram, int numCellsWidth, int numCellsHeight, float cellSize, float offsetX = 0, float offsetY = 0, float offsetZ = 0, std::optional<glm::quat> orientation = std::nullopt);

void drawGrid3D(GLint shaderProgram, int numCellsWidth, float cellSize, float offsetX = 0, float offsetY = 0, float offsetZ = 0);
void drawCoordinateSystem(GLint shaderProgram, float size);

std::vector<Line> drawSphere();

#endif 
