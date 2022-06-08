#ifndef MOD_LINES
#define MOD_LINES

#include "./common/util.h"
#include "./scene/common/util/types.h"
#include "./scene/common/mesh.h"
#include "./scene/sprites/sprites.h"
#include <glad/glad.h>
#include "glm/ext.hpp"

struct LineDrawingOptions {
  Line line;
  objid lineid;
  objid owner;
  LineColor color;
  std::optional<unsigned int> textureId;
  bool permaLine;
};

struct TextDrawingOptions  {
  std::string word;
  float left;
  float top;
  unsigned int fontSize;
  std::optional<unsigned int> textureId;
};

struct LineData {
  std::vector<LineDrawingOptions> permaLines;
  std::vector<TextDrawingOptions> text;
};

LineData createLines();

objid addLineNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, LineColor color, std::optional<unsigned int> textureId);
objid addLineNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, std::optional<unsigned int> textureId);
void freeLine(LineData& lineData, objid lineId);
void removeLinesByOwner(LineData& lineData, objid owner);

void drawAllLines(LineData& lineData, GLint shaderProgram, std::optional<unsigned int> textureId);

void addTextData(LineData& lineData, TextDrawingOptions text);
void drawTextData(LineData& lineData, unsigned int uiShaderProgram, std::map<unsigned int, Mesh>& fontMeshes, std::optional<unsigned int> textureId);

void disposeTempBufferedData(LineData& lineData);

std::vector<unsigned int> textureIdsToRender(LineData& lineData);

#endif
