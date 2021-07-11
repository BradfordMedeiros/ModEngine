#ifndef MOD_TEXTURE
#define MOD_TEXTURE

#include <iostream>
#include <glm/glm.hpp>
#include <stdexcept>
#include <stb_image.h>
#include <stb_image_write.h>
#include <vector>
#include "glad/glad.h"
#include "./util/loadmodel.h"
#include "./util/types.h"

struct Texture {
   unsigned int textureId;
};

Texture loadTextureData(unsigned char* data, int textureWidth, int textureHeight, int numChannels);
Texture loadTexture(std::string textureFilePath);
Texture loadCubemapTexture(std::string textureFilePathRoot);
void updateTextureData(Texture& texture, unsigned char* data, int textureWidth, int textureHeight);
void saveTextureData(std::string filepath, char* data, int width, int height);

#endif