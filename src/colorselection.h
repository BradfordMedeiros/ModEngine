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
glm::vec4 getColorFromGameobject(objid id);

struct UVCoord {
  GLfloat x;
  GLfloat y;
};

UVCoord getUVCoord(GLint x, GLint y);
glm::vec3 uvToNDC(UVCoord coord);

void saveScreenshot(std::string& filepath);

#endif
