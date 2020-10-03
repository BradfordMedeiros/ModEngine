#include "./colorselection.h"

Color getPixelColor(GLint x, GLint y, unsigned int currentScreenHeight) {
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  Color color;
  glReadPixels(x, currentScreenHeight - y, 1, 1, GL_RGBA, GL_FLOAT, &color); 
  return color;
}

objid getIdFromColor(Color color){
  int redBack =   ((int)(color.r * 255.f));
  int greenBack = ((int)(color.g * 255.f)) << 8;
  int blueBack =  ((int)(color.b * 255.f)) << 16;
  int alphaBack = ((int)(color.a * 255.f)) << 24;
  int objectId = redBack + greenBack + blueBack + alphaBack ;
  return objectId;
}

glm::vec4 getColorFromGameobject(objid id){
  int redChannel =   (id & 0x000000FF);
  int greenChannel = (id & 0x0000FF00) >> 8;
  int blueChannel =  (id & 0x00FF0000) >> 16;
  int alphaChannel = (id & 0xFF000000) >> 24;
  glm::vec4 idColor = glm::vec4(redChannel / 255.f, greenChannel / 255.f, blueChannel / 255.f, alphaChannel / 255.f);
  Color color {  
    .r = idColor.r, 
    .g = idColor.g, 
    .b = idColor.b,
    .a = idColor.a, 
  };
  assert(getIdFromColor(color) == id);
  return idColor;
}

UVData getUVCoord(GLint x, GLint y, unsigned int currentScreenHeight){
  glReadBuffer(GL_COLOR_ATTACHMENT1);
  UVData uvdata; 
  glReadPixels(x, currentScreenHeight - y, 1, 1, GL_RG, GL_FLOAT, &uvdata); 
  return uvdata;
}
