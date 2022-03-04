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
struct RenderDataFloatArr {
  std::string uniformName;
  std::vector<float> value;
};
struct RenderDataVec3 {
  std::string uniformName;
  glm::vec3 value;
};
struct RenderDataBuiltIn {
  std::string uniformName;
  std::string builtin;
};

enum RenderTextureType { RENDER_TEXTURE_REGULAR, RENDER_TEXTURE_FRAMEBUFFER };
struct RenderTexture {
  std::string nameInShader;
  RenderTextureType type;
  std::string textureName;
  int framebufferTextureId;
};

struct RenderStep {
  std::string name;
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
  std::vector<RenderDataFloatArr> floatArrUniforms;
  std::vector<RenderDataVec3> vec3Uniforms;
  std::vector<RenderDataBuiltIn> builtInUniforms;
  std::vector<RenderTexture> textures;
};

struct RenderStages {
  RenderStep selection;
  RenderStep shadowmap;
  RenderStep main;
  RenderStep portal;
  RenderStep bloom1;
  RenderStep bloom2;
  RenderStep dof1;
  RenderStep dof2;
  std::vector<RenderStep> additionalRenderSteps;

  // dependent data
  unsigned int* portalTextures;
  int numPortalTextures;  
  int numDepthTextures;
};

RenderStages loadRenderStages(
  unsigned int fbo, 
  unsigned int framebufferTexture, 
  unsigned int framebufferTexture2,
  unsigned int framebufferTexture3,
  unsigned int* depthTextures, int numDepthTextures,
  unsigned int* portalTextures, int numPortalTextures,
  RenderShaders shaders
);

struct RenderStagesDofInfo {
  int blurAmount;
  float minBlurDistance;
  float maxBlurDistance;
  float nearplane;
  float farplane;
};
void updateRenderStages(RenderStages& stages, RenderStagesDofInfo& dofInfo);
void renderStagesSetPortal(RenderStages& stages, unsigned int portalNumber);
void renderStagesSetShadowmap(RenderStages& stages, unsigned int shadowmapNumber);

unsigned int finalRenderingTexture(RenderStages& stages);
std::string renderStagesToString(RenderStages& stages);

#endif