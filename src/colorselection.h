#ifndef MOD_COLORSELECTION
#define MOD_COLORSELECTION
#include <math.h>     
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include "./scene/common/texture.h"
#include "./translations.h" 

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

struct UVCoord {
  GLfloat x;
  GLfloat y;
};

UVCoord getUVCoord(GLint x, GLint y);
glm::ivec2 ndiToPixelCoord(glm::vec2 ndi, glm::vec2 resolution);
glm::vec3 uvToNDC(UVCoord coord);
glm::ivec2 pixelCoordsRelativeToViewport(int x, int y, unsigned int currentScreenHeight, glm::ivec2 viewportSize, glm::ivec2 viewportoffset, glm::ivec2 resolution);

void saveScreenshot(std::string& filepath);

#endif
