#ifndef MOD_LINES
#define MOD_LINES

#include "./common/util.h"
#include "./scene/common/util/types.h"
#include "./scene/common/mesh.h"
#include "./scene/sprites/sprites.h"
#include "./colorselection.h"
#include <glad/glad.h>
#include "glm/ext.hpp"

struct LineDrawingOptions {
  Line line;
  objid lineid;
  objid owner;
  std::optional<unsigned int> textureId;
  bool permaLine;
};

struct TextDrawingOptions  {
  std::string word;
  float left;
  float top;
  unsigned int fontSize;
  std::optional<unsigned int> textureId;
  bool permaText;
  bool ndi;
  glm::vec4 tint;
  std::optional<std::string> fontFamily;
  std::optional<objid> selectionId;
};

struct LineByColor {
  glm::vec4 tint;
  unsigned int linewidth;
  std::vector<LineDrawingOptions> lines;
};
struct LineData {
  std::vector<LineByColor> lineColors;;
  std::vector<TextDrawingOptions> text;
};

LineData createLines();

objid addLineToNextCycleTint(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, std::optional<unsigned int> linewidth);
objid addLineToNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, LineColor color, std::optional<unsigned int> textureId);
void freeLine(LineData& lineData, objid lineId);
void removeLinesByOwner(LineData& lineData, objid owner);

void drawAllLines(LineData& lineData, GLint shaderProgram, std::optional<unsigned int> textureId);

void addTextData(LineData& lineData, TextDrawingOptions text);
void drawTextData(LineData& lineData, unsigned int uiShaderProgram, std::function<FontFamily&(std::string)> fontFamilyByName, std::optional<unsigned int> textureId, unsigned int height, unsigned int width);

void disposeTempBufferedData(LineData& lineData);


#endif
