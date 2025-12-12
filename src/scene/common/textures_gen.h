#ifndef MOD_TEXTUREGEN
#define MOD_TEXTUREGEN

#include "glad/glad.h"
#include <cstddef>
#include <vector>

void genFramebufferTexture(unsigned int *texture, unsigned int resolutionX, unsigned int resolutionY, const char* debugName);
void updateDepthTexturesSize(unsigned int* textures, int numTextures, unsigned int resolutionX, unsigned int resolutionY);
void setActiveDepthTexture(unsigned int fbo, unsigned int* textures, int index);

struct Framebuffers {
  unsigned int fbo;
  unsigned int framebufferTexture;
  unsigned int framebufferTexture2;
  unsigned int framebufferTexture3;
  unsigned int framebufferTexture4;

  std::vector<unsigned int> portalTextures = std::vector<unsigned int>(16);
  std::vector<unsigned int> depthTextures = std::vector<unsigned int>(32);
  std::vector<unsigned int> textureDepthTextures = std::vector<unsigned int>(1);
};

Framebuffers generateFramebuffers(int resolutionX, int resolutionY);
void updateFramebufferWindowSizeChange(Framebuffers& framebuffers, int resolutionX, int resolutionY);

#endif