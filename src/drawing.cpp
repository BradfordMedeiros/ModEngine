#include "./drawing.h"

DrawingParams getDefaultDrawingParams(){
  DrawingParams params { 
    .opacity = 1,
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

std::string activeTextureName(DrawingParams& params, std::map<std::string, Texture> worldTextures){
  int currentTextureIndex = 0;
  for (auto [name, _] : worldTextures){
    if (currentTextureIndex >= params.activeTextureIndex){
        std::cout << "active texture name: " << name << std::endl;
      return name;
    }
    currentTextureIndex++;
  }
  assert(false);
}