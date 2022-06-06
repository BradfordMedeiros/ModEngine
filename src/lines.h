#ifndef MOD_LINES
#define MOD_LINES

#include "./common/util.h"
#include "./scene/common/util/types.h"
#include "./scene/common/mesh.h"
#include <glad/glad.h>
#include "glm/ext.hpp"

struct PermaLine {
  Line line;
  objid lineid;
  objid owner;
  LineColor color;
};

struct LineData {
  std::vector<Line> lines;
  std::vector<Line> bluelines;
  std::vector<PermaLine> permaLines;
  std::vector<Line> screenspaceLines;
};

LineData createLines();
objid addLineNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, LineColor color);
objid addLineNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, std::optional<unsigned int> textureId);
void freeLine(LineData& lineData, objid lineId);
void removeLinesByOwner(LineData& lineData, objid owner);
void drawPermaLines(LineData& lineData, GLint shaderProgram, LineColor color, glm::vec4 tint);
void drawScreenspaceLines(LineData& lineData, GLint shaderProgram);

#endif
