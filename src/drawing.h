#ifndef MOD_DRAWING
#define MOD_DRAWING

#include <glm/glm.hpp>

struct DrawingParams{
  float opacity;
  glm::vec3 scale;
  glm::vec3 tint;
};

DrawingParams getDefaultDrawingParams();

#endif
