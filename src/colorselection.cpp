#include "./colorselection.h"

Color getPixelColor(GLint x, GLint y, unsigned int currentScreenHeight) {
    Color color;
    glReadPixels(x, currentScreenHeight - y, 1, 1, GL_RGB, GL_FLOAT, &color);
    return color;
}

glm::vec3 getColorFromGameobject(GameObject object, bool useSelectionColor, bool isSelected){
  if (isSelected){
    return glm::vec3(1.0f, 0.0f, 0.0f);
  }
  if (!useSelectionColor){
    return glm::vec3(1.0f, 1.0f, 1.0f);
  }
  float blueChannel = object.id * 0.01;
  return glm::vec3(0.0f, 0.0f, blueChannel);
}

unsigned int getIdFromColor(float r, float g, float b){
  short objectId = round(b / 0.01);
  return objectId;
}
