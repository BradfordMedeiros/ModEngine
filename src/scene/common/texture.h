#ifndef MOD_TEXTURE
#define MOD_TEXTURE

#include <iostream>
#include <glm/glm.hpp>
#include <stdexcept>
#include <stb_image.h>
#include <stb_image_write.h>
#include <vector>
#include <set>
#include "glad/glad.h"
#include "./util/loadmodel.h"
#include "./util/types.h"

struct Texture {
   unsigned int textureId;
};

struct TextureRef {
  std::set<objid> owners;
  Texture texture;
};

Texture loadTextureEmpty(int textureWidth, int textureHeight, int numChannels);
Texture loadTextureData(unsigned char* data, int textureWidth, int textureHeight, int numChannels);
Texture loadTextureDataRed(unsigned char* data, int textureWidth, int textureHeight);
Texture loadTexture(std::string textureFilePath);
Texture loadCubemapTexture(std::string textureFilePathRoot);
void updateTextureData(Texture& texture, unsigned char* data, int textureWidth, int textureHeight);
void saveTextureData(std::string filepath, char* data, int width, int height);
void freeTexture(Texture& texture);

struct TextureSizeInfo {
  int width;
  int height;
};

// Side effect binding the texture
TextureSizeInfo getTextureSizeInfo(Texture& texture);

#endif