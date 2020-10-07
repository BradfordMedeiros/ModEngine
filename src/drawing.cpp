#include "./drawing.h"

DrawingParams getDefaultDrawingParams(){
  DrawingParams params { 
    .opacity = 0.1,
    .scale = glm::vec3(1.0f, 1.0f, 1.0f),
    .tint = glm::vec3(1.f, 1.f, 1.f),
  
    .activeTextureIndex = 0,
  };
  return params;
}

void nextTexture(DrawingParams& params, int numTotalTextures){
  params.activeTextureIndex = (params.activeTextureIndex + 1) % numTotalTextures;
  std::cout << "INFO: next texture: texture painting selection"  << params.activeTextureIndex << std::endl;
}

void previousTexture(DrawingParams& params){
  params.activeTextureIndex = params.activeTextureIndex - 1;
  if (params.activeTextureIndex < 0){
    params.activeTextureIndex = 0;
  }
  std::cout << "INFO: previous texture: texture painting selection"  << params.activeTextureIndex << std::endl;
}