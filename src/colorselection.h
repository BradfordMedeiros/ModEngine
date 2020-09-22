#ifndef MOD_COLORSELECTION
#define MOD_COLORSELECTION
#include <math.h>     
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "./common/util.h"

struct Color {
  GLfloat r;
  GLfloat g;
  GLfloat b;
};

Color getPixelColor(GLint x, GLint y, unsigned int currentScreenHeight);
glm::vec3 getColorFromGameobject(objid id);
unsigned int getIdFromColor(Color color);

#endif