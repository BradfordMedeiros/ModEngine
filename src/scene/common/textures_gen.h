#ifndef MOD_TEXTUREGEN
#define MOD_TEXTUREGEN

#include "glad/glad.h"
#include <cstddef>

void genFramebufferTexture(unsigned int *texture, unsigned int resolutionX, unsigned int resolutionY);

void generateDepthTextures(unsigned int* textures, int numTextures, unsigned int resolutionX, unsigned int resolutionY);
void updateDepthTexturesSize(unsigned int* textures, int numTextures, unsigned int resolutionX, unsigned int resolutionY);
void setActiveDepthTexture(unsigned int fbo, unsigned int* textures, int index);

void generatePortalTextures(unsigned int* portalTextures, int numPortalTextures, unsigned int resolutionX, unsigned int resolutionY);
void updatePortalTexturesSize(unsigned int* portalTextures, int numPortalTextures, unsigned int resolutionX, unsigned int resolutionY);


#endif