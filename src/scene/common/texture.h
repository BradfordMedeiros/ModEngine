#ifndef MOD_TEXTURE
#define MOD_TEXTURE

#include <iostream>
#include <glm/glm.hpp>
#include <stdexcept>
#include <stb_image.h>
#include "glad/glad.h"
#include "./util/loadmodel.h"
#include "./util/types.h"

struct Texture {
   unsigned int textureId;
};

Texture loadTexture(std::string textureFilePath);


#endif