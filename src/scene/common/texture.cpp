#include "./texture.h"

std::string readFileOrPackage(std::string filepath); // i dont really like directly referencing this here, but...it's ok
bool fileExistsFromPackage(std::string path);

unsigned char* stbiloadImage(const char* textureFilePath, int* _textureWidth, int* _textureHeight, int* _numChannels, int forcedChannels){
  auto textureData = readFileOrPackage(textureFilePath); 
  unsigned char* image_data = stbi_load_from_memory((const unsigned char*)textureData.c_str(), textureData.size(), _textureWidth, _textureHeight, _numChannels, forcedChannels);
  return image_data;
}


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

Texture loadTextureSelection(int textureWidth, int textureHeight){
  unsigned int texture;
  glGenTextures(1, &texture);
  genFramebufferTexture(&texture, textureWidth, textureHeight);
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
  modlog("texture-loading", textureFilePath);

  int textureWidth, textureHeight, numChannels;
  int forcedChannels = 4;
  stbi_set_flip_vertically_on_load(true);
  modlog("load file loadTexture", textureFilePath);
  unsigned char* data = stbiloadImage(textureFilePath.c_str(), &textureWidth, &textureHeight, &numChannels, forcedChannels); 
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

  modlog("load file replaceTexture", textureFilePath);
  unsigned char* data = stbiloadImage(textureFilePath.c_str(), &textureWidth, &textureHeight, &numChannels, forcedChannels); 
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
  
  modassert(data.size() == 6, "invalid cubemap texture size");
  GLint format = GL_RGB;

  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, format, textureWidth.at(0), textureHeight.at(0), 0, format, GL_UNSIGNED_BYTE, data.at(0));
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, format, textureWidth.at(1), textureHeight.at(1), 0, format, GL_UNSIGNED_BYTE, data.at(1));
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, format, textureWidth.at(2), textureHeight.at(2), 0, format, GL_UNSIGNED_BYTE, data.at(2));
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, format, textureWidth.at(3), textureHeight.at(3), 0, format, GL_UNSIGNED_BYTE, data.at(3));
  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, format, textureWidth.at(4), textureHeight.at(4), 0, format, GL_UNSIGNED_BYTE, data.at(4));
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, format, textureWidth.at(5), textureHeight.at(5), 0, format, GL_UNSIGNED_BYTE, data.at(5));

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
    std::filesystem::path(textureFilePathRoot) / std::filesystem::path("right.jpg").string(),
    std::filesystem::path(textureFilePathRoot) / std::filesystem::path("left.jpg").string(),
    std::filesystem::path(textureFilePathRoot) / std::filesystem::path("top.jpg").string(),
    std::filesystem::path(textureFilePathRoot) / std::filesystem::path("bottom.jpg").string(),
    std::filesystem::path(textureFilePathRoot) / std::filesystem::path("front.jpg").string(),
    std::filesystem::path(textureFilePathRoot) / std::filesystem::path("back.jpg").string(),
  };

  std::vector<unsigned char*> datas;
  std::vector<int> textureWidths;
  std::vector<int> textureHeights;
  std::vector<int> channels;

  std::cout << "start loading raw jpg data" << std::endl;
  for (auto filepath : filepaths){
    int textureWidth, textureHeight, numChannels;
  
    stbi_set_flip_vertically_on_load(false);

    modlog("load file loadCubemapTexture", textureFilePathRoot);
    unsigned char* data = stbiloadImage(filepath.c_str(), &textureWidth, &textureHeight, &numChannels, 0); 
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
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGBA, GL_UNSIGNED_BYTE, data);
}
//   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);


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

int calculateAtlasImageDimension(int numTextures){
  float nextSquare = glm::sqrt(numTextures);
  int roundedUp = std::ceil(nextSquare);
  return roundedUp;
}


Texture loadTextureAtlasMaybeWriteCache(std::vector<std::string> textureFilePaths, std::optional<std::string> cacheFileName){
  int imageWidth = 500;
  int imageHeight = 500;
  int numImagesWide = calculateAtlasImageDimension(textureFilePaths.size());
  int numImagesHeight = numImagesWide;

  int newWidth = imageWidth * numImagesWide;
  int newHeight = imageHeight * numImagesHeight;
  unsigned char* textureAtlasData = (unsigned char *)malloc(newWidth * newHeight * 4);
  std::memset(textureAtlasData, 0, newWidth * newHeight * 4);

  auto atlasTexture = loadTextureData(textureAtlasData, newWidth, newHeight, 4);

  for (int i = 0; i < textureFilePaths.size(); i++){
    int xIndex = (i % numImagesWide) * imageWidth;
    int yIndex = (i / numImagesWide) * imageHeight;

    int textureWidth, textureHeight, numChannels;
    int forcedChannels = 4;

    modlog("load file loadTextureAtlasMaybeWriteCache", textureFilePaths.at(i));
    unsigned char* data = stbiloadImage(textureFilePaths.at(i).c_str(), &textureWidth, &textureHeight, &numChannels, forcedChannels); 
    if (!data){
      throw std::runtime_error("failed loading texture " + textureFilePaths.at(i) + ", reason: " + stbi_failure_reason());
    }
    unsigned char* resizedData = (unsigned char*)malloc(imageWidth * imageHeight * 4);
    stbir_resize_uint8_linear(data, textureWidth, textureHeight, 0, resizedData, imageWidth, imageHeight, 0, STBIR_RGBA);
    glTexSubImage2D(GL_TEXTURE_2D, 0, xIndex, yIndex, imageWidth, imageHeight, GL_RGBA, GL_UNSIGNED_BYTE, resizedData);
   
    free(resizedData);
    stbi_image_free(data);
  }

  if (cacheFileName.has_value()){
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureAtlasData);
    stbi_flip_vertically_on_write(1);
    stbi_write_png(cacheFileName.value().c_str(), newWidth, newHeight, 4, textureAtlasData, 0);     
  }

  free(textureAtlasData);
  return atlasTexture;
}

Texture loadTextureAtlas(std::vector<std::string> textureFilePaths, std::optional<std::string> cacheFileName){
  // check if the file exists, if it does just load it, assemble it and then load
  if (cacheFileName.has_value() && fileExistsFromPackage(cacheFileName.value())){
    return loadTexture(cacheFileName.value());
  }
  return loadTextureAtlasMaybeWriteCache(textureFilePaths, cacheFileName);
}