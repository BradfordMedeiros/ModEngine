#ifndef MOD_COLORSELECTION
#define MOD_COLORSELECTION
#include <math.h>     
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>
#include "./common/util.h"

struct Color {
  GLfloat r;
  GLfloat g;
  GLfloat b;
  GLfloat a;
};

Color getPixelColor(GLint x, GLint y, unsigned int currentScreenHeight);
objid getIdFromColor(Color color);
glm::vec4 getColorFromGameobject(objid id);

#endif
