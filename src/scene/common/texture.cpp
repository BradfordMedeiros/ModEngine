#include "./texture.h"

Texture loadTextureEmpty(int textureWidth, int textureHeight, int numChannels){
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  
  bool numChannelsValid = (numChannels == 3 || numChannels == 4);
  if (!numChannelsValid){
    std::cout << "Num channels: " << numChannels << " (width, height) = (" << textureWidth << ", " << textureHeight << ") " << std::endl;
    assert(false);
  }
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // https://stackoverflow.com/questions/15983607/opengl-texture-tilted
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

  // NULL is defined behavior, but the image is uninitialized and the image data itself therefore undefined values. 
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

  Texture tex {
    .textureId = texture,
  };
  return tex;
}
Texture loadTextureData(unsigned char* data, int textureWidth, int textureHeight, int numChannels){
  modassert(textureWidth > 0, "ERROR - loadTextureData - texture width must be > 0");
  modassert(textureHeight > 0, "ERROR - loadTextureData - height width must be > 0");
  
  auto tex = loadTextureEmpty(textureWidth, textureHeight, numChannels);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  return tex;
}

Texture loadTextureDataRed(unsigned char* data, int textureWidth, int textureHeight){
  modassert(textureWidth > 0, "ERROR - loadTextureDataRed - texture width must be > 0");
  modassert(textureHeight > 0, "ERROR - loadTextureDataRed - height width must be > 0");

  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  // set texture options
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);   

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // https://www.khronos.org/opengl/wiki/Texture#Swizzle_mask
  GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_RED};
  glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

  glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RED,
      textureWidth, 
      textureHeight, 
      0,
      GL_RED,
      GL_UNSIGNED_BYTE,
      data 
  );
  Texture tex {
    .textureId = texture,
  };
  return tex;
}

Texture loadTexture(std::string textureFilePath){
  std::cout << "Event: loading texture: " << textureFilePath << std::endl;

  int textureWidth, textureHeight, numChannels;
  int forcedChannels = 4;
  stbi_set_flip_vertically_on_load(true);
  unsigned char* data = stbi_load(textureFilePath.c_str(), &textureWidth, &textureHeight, &numChannels, forcedChannels); 
  if (!data){
    throw std::runtime_error("failed loading texture " + textureFilePath + ", reason: " + stbi_failure_reason());
  }
  auto texture = loadTextureData(data, textureWidth, textureHeight, forcedChannels); // forced to have 4
  stbi_image_free(data);
  return texture;
}

void replaceTexture(Texture& texture, std::string& textureFilePath, bool allowFail){
  int textureWidth, textureHeight, numChannels;
  int forcedChannels = 4;
  stbi_set_flip_vertically_on_load(true);
  unsigned char* data = stbi_load(textureFilePath.c_str(), &textureWidth, &textureHeight, &numChannels, forcedChannels); 
  if (!data){
    if (!allowFail){
      throw std::runtime_error("failed loading texture " + textureFilePath + ", reason: " + stbi_failure_reason());
    }
    std::cout << "warning: failed loading texture" << std::endl;
  }
  modassert(textureWidth > 0, "ERROR - loadTextureData - texture width must be > 0");
  modassert(textureHeight > 0, "ERROR - loadTextureData - height width must be > 0");

  glBindTexture(GL_TEXTURE_2D, texture.textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(data);
}

Texture loadCubemapTextureData(std::vector<unsigned char*> data, std::vector<int> textureWidth, std::vector<int> textureHeight, std::vector<int> numChannels){
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
  
  for (int i = 0; i < data.size(); i++){
    GLint format = GL_RGB;
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, textureWidth.at(i), textureHeight.at(i), 0, format, GL_UNSIGNED_BYTE, data.at(i));
  } 

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  Texture tex {
    .textureId = texture,
  };
  return tex;
}

Texture loadCubemapTexture(std::string textureFilePathRoot){
  std::cout << "Event: loading texture: " << textureFilePathRoot << std::endl;
  std::vector<std::string> filepaths = {
    std::filesystem::weakly_canonical(std::filesystem::path(textureFilePathRoot) / std::filesystem::path("right.jpg")).string(),
    std::filesystem::weakly_canonical(std::filesystem::path(textureFilePathRoot) / std::filesystem::path("left.jpg")).string(),
    std::filesystem::weakly_canonical(std::filesystem::path(textureFilePathRoot) / std::filesystem::path("top.jpg")).string(),
    std::filesystem::weakly_canonical(std::filesystem::path(textureFilePathRoot) / std::filesystem::path("bottom.jpg")).string(),
    std::filesystem::weakly_canonical(std::filesystem::path(textureFilePathRoot) / std::filesystem::path("front.jpg")).string(),
    std::filesystem::weakly_canonical(std::filesystem::path(textureFilePathRoot) / std::filesystem::path("back.jpg")).string(),
  };

  std::vector<unsigned char*> datas;
  std::vector<int> textureWidths;
  std::vector<int> textureHeights;
  std::vector<int> channels;

  std::cout << "start loading raw jpg data" << std::endl;
  for (auto filepath : filepaths){
    int textureWidth, textureHeight, numChannels;
    std::cout << "loading file: " << filepath << std::endl;
  
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(filepath.c_str(), &textureWidth, &textureHeight, &numChannels, 0); 
    if (!data){
      throw std::runtime_error("failed loading texture " + textureFilePathRoot + ", reason: " + stbi_failure_reason());
    }
    datas.push_back(data);
    textureWidths.push_back(textureWidth);
    textureHeights.push_back(textureHeight);
    channels.push_back(numChannels);
  }
  std::cout << "finishing loading raw jpg data" << std::endl;

  std::cout << "start loading cubemap data" << std::endl;
  auto texture = loadCubemapTextureData(datas, textureWidths, textureHeights, channels);
  std::cout << "finishing loading cubemap data" << std::endl;

  for (int i = 0; i < datas.size(); i++){
    std::cout << "start freeing data: " << i << std::endl;
    stbi_image_free(datas.at(i));
    std::cout << "finish freeing data: " << i << std::endl;
  }
  return texture;  
}

void updateTextureData(Texture& texture, unsigned char* data, int textureWidth, int textureHeight){
  glBindTexture(GL_TEXTURE_2D, texture.textureId);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void saveTextureData(std::string filepath, char* data, int width, int height){
  stbi_flip_vertically_on_write(1);
  stbi_write_png(filepath.c_str(), width, height, 3, data, 0); 
}

void freeTexture(Texture& texture){
  std::cout << "Deleted texture: " << texture.textureId << std::endl;
  glDeleteTextures(1, &texture.textureId);
}

TextureSizeInfo getTextureSizeInfo(Texture& texture){
  int width;
  int height;
  glBindTexture(GL_TEXTURE_2D, texture.textureId);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
  return TextureSizeInfo {
    .width = width,
    .height = height,
  };
}