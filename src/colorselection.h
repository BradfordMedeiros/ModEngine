#ifndef MOD_COLORSELECTION
#define MOD_COLORSELECTION
#include <math.h>     
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "./scene/scene.h"

struct Color {
  GLfloat r;
  GLfloat g;
  GLfloat b;
};

Color getPixelColor(GLint x, GLint y, unsigned int currentScreenHeight);
glm::vec3 getColorFromGameobject(GameObject object, bool useSelectionColor, bool isSelected);
unsigned int getIdFromColor(float r, float g, float b);

#endif