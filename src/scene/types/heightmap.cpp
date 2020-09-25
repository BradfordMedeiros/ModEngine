#include "./heightmap.h"

HeightMapData loadAndAllocateHeightmap(std::string heightmapFilePath){
  std::cout << "INFO: LOADING HEIGHTMAP: " << heightmapFilePath << std::endl;
  
  std::cout << "Event: loading file: " << heightmapFilePath << std::endl;
  int textureWidth, textureHeight, numChannels;

  // https://github.com/nothings/stb/blob/master/stb_image.h
  unsigned char* imageData = stbi_load(heightmapFilePath.c_str(), &textureWidth, &textureHeight, &numChannels, 4);
  if (!imageData ){
    throw std::runtime_error("failed loading texture " + heightmapFilePath + ", reason: " + stbi_failure_reason());
  }
  
  float* newData = new float[textureWidth * textureHeight];
  assert(numChannels == 3);

  // 4 components = 4 bytes (4 components, 1 byte per component -> RGBA)
  for (int i = 0; i < textureWidth; i++){
    for (int j = 0; j < textureHeight; j++){
      int byteOffset = ((i * textureHeight) + j) * 4;
      char r = imageData[byteOffset];
      //char g = imageData[byteOffset + 1];
      //char b = imageData[byteOffset + 2];
      //char a = imageData[byteOffset + 3];
      newData[(i * textureHeight) + j] = r;
    }
  }
  stbi_image_free(imageData);

  HeightMapData heightmapData {
    .data = newData,
    .width = textureWidth,
    .height = textureHeight,
  };
  return heightmapData;
}
