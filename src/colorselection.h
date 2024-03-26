#ifndef MOD_COLORSELECTION
#define MOD_COLORSELECTION
#include <math.h>     
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "./scene/common/texture.h"

struct Color {
  GLfloat r;
  GLfloat g;
  GLfloat b;
  GLfloat a;
};

Color getPixelColor(GLint x, GLint y);
objid getIdFromColor(Color color);
objid getIdFromPixelCoord(GLint x, GLint y);
glm::vec4 getColorFromGameobject(objid id);
std::string printColor(Color color);

struct UVCoordAndTextureId {
  GLfloat x;
  GLfloat y;
  GLfloat z;
};
struct UVCoord {
  GLfloat x;
  GLfloat y;
};

UVCoord getUVCoord(GLint x, GLint y);
UVCoord toUvCoord(UVCoordAndTextureId uvCoordWithTex);
UVCoordAndTextureId getUVCoordAndTextureId(GLint x, GLint y);

glm::ivec2 ndiToPixelCoord(glm::vec2 ndi, glm::vec2 resolution);
glm::vec2 pixelCoordToNdi(glm::ivec2 pixelCoord, glm::vec2 resolution);

glm::vec3 uvToNDC(UVCoord coord);
glm::ivec2 uvToPixelCoord(UVCoord coord, glm::vec2 resolution);
glm::ivec2 pixelCoordsRelativeToViewport(int x, int y, unsigned int currentScreenHeight, glm::ivec2 viewportSize, glm::ivec2 viewportoffset, glm::ivec2 resolution);

std::string print(Color& color);

void saveScreenshot(std::string& filepath);

glm::vec3 getTintIfSelected(bool isSelected);

#endif
