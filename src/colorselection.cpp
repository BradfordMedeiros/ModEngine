#include "./colorselection.h"

Color getPixelColor(GLint x, GLint y, unsigned int currentScreenHeight) {
    Color color;
    glReadPixels(x, currentScreenHeight - y, 1, 1, GL_RGB, GL_FLOAT, &color);
    return color;
}

glm::vec3 getColorFromGameobject(objid id, bool useSelectionColor, bool isSelected){
  if (isSelected){
    return glm::vec3(1.0f, 0.0f, 0.0f);
  }
  if (!useSelectionColor){
    return glm::vec3(1.0f, 1.0f, 1.0f);
  }

  int redChannel = id % 255;
  int greenChannel = (id / 255) % 255;
  int blueChannel = (id / (255 * 255)) % (255 * 255); // so order of 255^3 ~ 16581375

  int sumValue = redChannel + (greenChannel * 255) + (blueChannel * 255 * 255);
  assert(blueChannel < 255);
  assert(sumValue == id);

  // since the max value of r/g/b should be 255, this needs a resolution of 1/255.f which is 
  // 0.00392156862745098 which should be fine. Still kind of hackey feeling
  return glm::vec3(redChannel / 255.f, greenChannel / 255.f, blueChannel / 255.f);
}

unsigned int getIdFromColor(Color color){
  short objectId = 255 * (color.r + (color.g * 255) + (color.b * 255 * 255));
  return objectId;
}
