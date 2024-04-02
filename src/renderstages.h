#ifndef MOD_RENDERSTAGES
#define MOD_RENDERSTAGES

#include <vector>
#include "./shaders.h"
#include "./scene/serialization.h"
#include "./state.h"

struct RenderShaders {
  unsigned int blurProgram;
  unsigned int selectionProgram;
  unsigned int uiShaderProgram;
  unsigned int shaderProgram;
  unsigned int basicProgram;
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
  bool enable;
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
  bool renderQuad3D;
  bool blend;
  bool enableStencil;
  bool allowShaderOverride;
  bool textBoundingOnly;
  RenderUniforms uniforms;
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
  RenderStep basicTexture;
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
  unsigned int framebufferTexture4,
  unsigned int* depthTextures, int numDepthTextures,
  unsigned int* portalTextures, int numPortalTextures,
  RenderShaders shaders,
  std::function<std::string(std::string)> readFile
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

void setRenderStageState(RenderStages& stages, ObjectValue& value);

unsigned int finalRenderingTexture(RenderStages& stages);
std::string renderStagesToString(RenderStages& stages);

struct DeserializedRenderStage {
  std::string name;
  bool enable;
  std::string shader;
  std::vector<RenderDataInt> intUniforms;
  std::vector<RenderDataFloat> floatUniforms;
  std::vector<RenderDataFloatArr> floatArrUniforms;
  std::vector<RenderDataVec3> vec3Uniforms;
  std::vector<RenderDataBuiltIn> builtInUniforms;
  std::vector<RenderTexture> textures;
};

bool isRenderStageToken(Token& token);
std::vector<DeserializedRenderStage> parseRenderStages(std::vector<Token>& tokens, unsigned int fb0, unsigned int fb1);

#endif