#ifndef MOD_COLORSELECTION
#define MOD_COLORSELECTION
#include <math.h>     
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include "./common/util.h"
#include "./scene/common/texture.h"

struct Color {
  GLfloat r;
  GLfloat g;
  GLfloat b;
  GLfloat a;
};

Color getPixelColor(GLint x, GLint y, unsigned int currentScreenHeight);
objid getIdFromColor(Color color);
glm::vec4 getColorFromGameobject(objid id);

struct UVCoord {
  GLfloat x;
  GLfloat y;
};

UVCoord getUVCoord(GLint x, GLint y, unsigned int currentScreenHeight);

void saveScreenshot(std::string filepath);

#endif
