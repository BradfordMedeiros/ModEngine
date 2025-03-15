#ifndef MOD_RENDERSTAGES
#define MOD_RENDERSTAGES

#include <vector>
#include "./shaders.h"
#include "./scene/serialization.h"
#include "./state.h"
#include "./common/util.h"

struct RenderShaders {
  unsigned int* blurProgram;
  unsigned int* selectionProgram;
  unsigned int* uiShaderProgram;
  unsigned int* shaderProgram;
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
  std::optional<unsigned int> colorAttachment1;
  std::optional<unsigned int> colorAttachment2;
  std::optional<unsigned int> colorAttachment3;
  unsigned int depthTextureIndex;
  unsigned int* shader;
  unsigned int quadTexture;
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
  RenderShaders& shaders,
  std::function<std::string(std::string)> readFile,
  std::unordered_map<std::string, std::string>& args
);

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
  std::vector<RenderTexture> textures;
};

bool isRenderStageToken(Token& token);
std::vector<DeserializedRenderStage> parseRenderStages(std::vector<Token>& tokens, unsigned int fb0, unsigned int fb1);

#endif