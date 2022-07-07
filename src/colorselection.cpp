#include "./colorselection.h"

Color getPixelColor(GLint x, GLint y) {
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  Color color;
  glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, &color); 
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

objid getIdFromPixelCoord(GLint x, GLint y){
  Color hoveredItemColor = getPixelColor(x, y);
  auto hoveredId = getIdFromColor(hoveredItemColor);
  return hoveredId;
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

// Emphasis:  Color attachment 1 needs to save the uvCoordData 
// eg: layout(location = 1) out vec2 UVCoords;
// UVCoords = vec2(1, 2)
UVCoord getUVCoord(GLint x, GLint y){
  glReadBuffer(GL_COLOR_ATTACHMENT1);
  UVCoord uvdata; 
  glReadPixels(x, y, 1, 1, GL_RG, GL_FLOAT, &uvdata); 
  return uvdata;
}

glm::ivec2 ndiToPixelCoord(glm::vec2 ndi, glm::vec2 resolution){
  auto xCoord = convertBase(ndi.x, -1, 1, 0, resolution.x);
  auto yCoord = convertBase(ndi.y, -1, 1, 0, resolution.y);
  return glm::ivec2(xCoord, yCoord);
}

glm::vec3 uvToNDC(UVCoord coord){
  float xCoord = convertBase(coord.x, 0, 1, -1, 1);
  float yCoord = convertBase(coord.y, 0, 1, -1, 1);
  return glm::vec3(xCoord, yCoord, 0.f);
}

void saveScreenshot(std::string& filepath){
  int w, h;
  int miplevel = 0;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);
  char* data = new char[w * h * 3];
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, data);
  saveTextureData(filepath, data, w, h);
  delete[] data;
}