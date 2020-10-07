#include "./drawing.h"

DrawingParams getDefaultDrawingParams(){
  DrawingParams params { 
    .opacity = 0.1,
    .scale = glm::vec3(5.0f, 1.0f, 1.0f),
    .tint = glm::vec3(1.f, 1.f, 1.f),
  };
  return params;
}