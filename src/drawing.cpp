#include "./drawing.h"

DrawingParams getDefaultDrawingParams(){
  DrawingParams params { 
    .opacity = 0.1,
    .scale = glm::vec3(5.0, 1.0, 1.0),
  };
  return params;
}