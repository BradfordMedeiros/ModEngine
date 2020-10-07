#ifndef MOD_DRAWING
#define MOD_DRAWING

#include <glm/glm.hpp>
#include <iostream>

struct DrawingParams{
  float opacity;
  glm::vec3 scale;
  glm::vec3 tint;

  int activeTextureIndex;
};

DrawingParams getDefaultDrawingParams();
void nextTexture(DrawingParams&, int numTotalTextures);
void previousTexture(DrawingParams&);

#endif
