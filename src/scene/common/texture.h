#ifndef MOD_TEXTURE
#define MOD_TEXTURE

#include <iostream>
#include <glm/glm.hpp>
#include <stdexcept>
#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_image_resize2.h>
#include <vector>
#include <set>
#include "glad/glad.h"
#include "./util/loadmodel.h"
#include "./util/types.h"
#include "./textures_gen.h"

struct Texture {
   unsigned int textureId;
};

struct TextureRef {
  std::set<objid> owners;
  Texture texture;
  std::optional<objid> mappingTexture;
};

unsigned char* stbiloadImage(const char* textureFilePath, int* _textureWidth, int* _textureHeight, int* _numChannels, int forcedChannels);
Texture loadTextureEmpty(int textureWidth, int textureHeight, int numChannels);
Texture loadTextureSelection(int textureWidth, int textureHeight);
Texture loadTextureData(unsigned char* data, int textureWidth, int textureHeight, int numChannels);
Texture loadTextureDataRed(unsigned char* data, int textureWidth, int textureHeight);
Texture loadTexture(std::string textureFilePath);
void replaceTexture(Texture& texture, std::string& textureFilePath, bool allowFail);
Texture loadCubemapTexture(std::string textureFilePathRoot);
void updateTextureData(Texture& texture, unsigned char* data, int textureWidth, int textureHeight);
void saveTextureData(std::string filepath, char* data, int width, int height);
void freeTexture(Texture& texture);

int calculateAtlasImageDimension(int numTextures);
Texture loadTextureAtlas(std::vector<std::string> textureFilePaths, std::optional<std::string> cacheFileName);

struct TextureSizeInfo {
  int width;
  int height;
};

// Side effect binding the texture
TextureSizeInfo getTextureSizeInfo(Texture& texture);

#endif