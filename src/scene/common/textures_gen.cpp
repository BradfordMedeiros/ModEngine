#include "./textures_gen.h"

void genFramebufferTexture(unsigned int *texture, unsigned int resolutionX, unsigned int resolutionY, const char* debugName){
  glGenTextures(1, texture);
  glBindTexture(GL_TEXTURE_2D, *texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolutionX, resolutionY, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  if (debugName){
    glObjectLabel(GL_TEXTURE, *texture, -1, debugName);
  }

}

void updateDepthTexturesSize(unsigned int* textures, int numTextures, unsigned int resolutionX, unsigned int resolutionY){
  for (int i = 0; i < numTextures; i++){
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    // GL_DEPTH_COMPONENT32F
    glTexImage2D(GL_TEXTURE_2D, 0,  GL_DEPTH_STENCIL, resolutionX, resolutionY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }
}

void generateDepthTextures(unsigned int* textures, int numTextures, unsigned int resolutionX, unsigned int resolutionY){
  glGenTextures(numTextures, textures);
  for (int i = 0; i < numTextures; i++){
    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);  // why nlinear and
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    updateDepthTexturesSize(textures, numTextures, resolutionX, resolutionY);
  }
}

void setActiveDepthTexture(unsigned int fbo, unsigned int* textures, int index){
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);  
  unsigned int texture = textures[index];
  // GL_DEPTH_ATTACHMENT
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
}

void updatePortalTexturesSize(unsigned int* portalTextures, int numPortalTextures, unsigned int resolutionX, unsigned int resolutionY){
  for (int i = 0; i < numPortalTextures; i++){
    glBindTexture(GL_TEXTURE_2D, portalTextures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolutionX, resolutionY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);   
  }
}

void generatePortalTextures(unsigned int* portalTextures, int numPortalTextures, unsigned int resolutionX, unsigned int resolutionY){
  glGenTextures(numPortalTextures, portalTextures);
  for (int i = 0; i < numPortalTextures; i++){
    glBindTexture(GL_TEXTURE_2D, portalTextures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    updatePortalTexturesSize(portalTextures, numPortalTextures, resolutionX, resolutionY);
  }
}


Framebuffers generateFramebuffers(int resolutionX, int resolutionY){
  Framebuffers framebuffers;

  glGenFramebuffers(1, &framebuffers.fbo);

  genFramebufferTexture(&framebuffers.framebufferTexture, resolutionX, resolutionY, "framebuffer_texture_0");
  genFramebufferTexture(&framebuffers.framebufferTexture2, resolutionX, resolutionY, "framebuffer_texture_2");
  genFramebufferTexture(&framebuffers.framebufferTexture3, resolutionX, resolutionY, "framebuffer_texture_3");
  genFramebufferTexture(&framebuffers.framebufferTexture4, resolutionX, resolutionY, "framebuffer_texture_4");

  generateDepthTextures(&framebuffers.depthTextures.at(0), framebuffers.depthTextures.size(), resolutionX, resolutionY);
  generateDepthTextures(&framebuffers.textureDepthTextures.at(0), 1, resolutionX, resolutionY);

  generatePortalTextures(&framebuffers.portalTextures.at(0), framebuffers.portalTextures.size(), resolutionX, resolutionY);

  return framebuffers;
}

void updateFramebufferWindowSizeChange(Framebuffers& framebuffers, int resolutionX, int resolutionY){
  glBindTexture(GL_TEXTURE_2D, framebuffers.framebufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolutionX, resolutionY, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, framebuffers.framebufferTexture2);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolutionX, resolutionY, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, framebuffers.framebufferTexture3);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolutionX, resolutionY, 0, GL_RGBA, GL_FLOAT, NULL);

  glBindTexture(GL_TEXTURE_2D, framebuffers.framebufferTexture4);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolutionX, resolutionY, 0, GL_RGBA, GL_FLOAT, NULL);

  updateDepthTexturesSize(&framebuffers.depthTextures.at(0), framebuffers.depthTextures.size(), resolutionX, resolutionY);
  updatePortalTexturesSize(&framebuffers.portalTextures.at(0), framebuffers.portalTextures.size(), resolutionX, resolutionY);
}