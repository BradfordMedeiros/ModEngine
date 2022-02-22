#ifndef MOD_RENDERSTAGES
#define MOD_RENDERSTAGES

#include <vector>
#include "./shaders.h"
#include "./scene/serialization.h"

struct RenderShaders {
  unsigned int blurProgram;
  unsigned int selectionProgram;
  unsigned int shaderProgram;
};

struct RenderDataInt {
  std::string uniformName;
  int value;
};
struct RenderDataFloat {
  std::string uniformName;
  float value;
};

struct RenderStep {
  const char* name;
  unsigned int fbo;
  unsigned int colorAttachment0;
  unsigned int colorAttachment1;
  unsigned int depthTextureIndex;
  unsigned int shader;
  unsigned int quadTexture;
  bool hasColorAttachment1;
  bool renderWorld;
  bool renderSkybox;
  bool renderQuad;
  bool blend;
  bool enableStencil;
  std::vector<RenderDataInt> intUniforms;
  std::vector<RenderDataFloat> floatUniforms;
};

struct RenderStages {
  RenderStep selection;
  RenderStep main;
  RenderStep bloom1;
  RenderStep bloom2;
  std::vector<RenderStep> additionalRenderSteps;
};

RenderStages loadRenderStages(
  unsigned int fbo, 
  unsigned int framebufferTexture, 
  unsigned int framebufferTexture2,
  unsigned int framebufferTexture3,
  RenderShaders shaders
);

unsigned int finalRenderingTexture(RenderStages& stages);

#endif