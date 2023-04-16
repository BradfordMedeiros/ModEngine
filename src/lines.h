#ifndef MOD_LINES
#define MOD_LINES

#include <glad/glad.h>
#include "glm/ext.hpp"
#include "./scene/sprites/sprites.h"
#include "./colorselection.h"

struct LineDrawingOptions {
  Line line;
  objid lineid;
  objid owner;
  std::optional<unsigned int> textureId;
  bool permaLine;
};

struct TextShapeData {
  std::string word;
  std::optional<std::string> fontFamily;
  unsigned int fontSize;
  float left;
  float top;
};

struct RectShapeData {
  float centerX;
  float centerY;
  float width;
  float height;
};

typedef std::variant<TextShapeData, RectShapeData> ShapeDataInfo;

struct ShapeData  {
  ShapeDataInfo shapeData;
  std::optional<unsigned int> textureId;
  bool perma;
  bool ndi;
  glm::vec4 tint;
  std::optional<objid> selectionId;
};

struct LineByColor {
  glm::vec4 tint;
  unsigned int linewidth;
  std::vector<LineDrawingOptions> lines;
};
struct LineData {
  std::vector<LineByColor> lineColors;;
  std::vector<ShapeData> text;
};

LineData createLines();

objid addLineToNextCycleTint(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, std::optional<unsigned int> linewidth);
objid addLineToNextCycle(LineData& lineData, glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, LineColor color, std::optional<unsigned int> textureId);
void freeLine(LineData& lineData, objid lineId);
void removeLinesByOwner(LineData& lineData, objid owner);

void drawAllLines(LineData& lineData, GLint shaderProgram, std::optional<unsigned int> textureId);

void addShapeData(LineData& lineData, ShapeData text);
void drawShapeData(LineData& lineData, unsigned int uiShaderProgram, std::function<FontFamily&(std::string)> fontFamilyByName, std::optional<unsigned int> textureId, unsigned int height, unsigned int width, Mesh& unitXYRect);

void disposeTempBufferedData(LineData& lineData);


#endif
