#include "./textures_gen.h"

void genFramebufferTexture(unsigned int *texture, unsigned int resolutionX, unsigned int resolutionY){
  glGenTextures(1, texture);
  glBindTexture(GL_TEXTURE_2D, *texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolutionX, resolutionY, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void generateDepthTextures(unsigned int* textures, int numTextures, unsigned int resolutionX, unsigned int resolutionY){
  glGenTextures(numTextures, textures);
  for (int i = 0; i < numTextures; i++){
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    updateDepthTexturesSize(textures, numTextures, resolutionX, resolutionY);
  }
}
void updateDepthTexturesSize(unsigned int* textures, int numTextures, unsigned int resolutionX, unsigned int resolutionY){
  for (int i = 0; i < numTextures; i++){
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    // GL_DEPTH_COMPONENT32F
    glTexImage2D(GL_TEXTURE_2D, 0,  GL_DEPTH_STENCIL, resolutionX, resolutionY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }
}
void setActiveDepthTexture(unsigned int fbo, unsigned int* textures, int index){
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);  
  unsigned int texture = textures[index];
  // GL_DEPTH_ATTACHMENT
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
}