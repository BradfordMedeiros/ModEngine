#ifndef MOD_DRAWING
#define MOD_DRAWING

#include <glm/glm.hpp>
#include <iostream>
#include <map>
#include "./scene/common/texture.h"

struct DrawingParams{
  float opacity;
  glm::vec3 scale;
  glm::vec3 tint;

  int activeTextureIndex;
};

DrawingParams getDefaultDrawingParams();
void nextTexture(DrawingParams&, int numTotalTextures);
void previousTexture(DrawingParams&);
std::string activeTextureName(DrawingParams& params, std::map<std::string, TextureRef> worldTextures);

#endif
