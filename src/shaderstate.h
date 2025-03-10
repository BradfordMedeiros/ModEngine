#ifndef MOD_SHADERSTATE
#define MOD_SHADERSTATE

#include "./state.h"
#include "./scene/scene_object.h"

// application rendering stuff
struct RenderingResources { 
  unsigned int* framebufferProgram;
  unsigned int* uiShaderProgram;
  Framebuffers framebuffers;
  UniformBuffer voxelLighting;
};

void initDefaultShader(unsigned int shader);
void updateDefaultShaderPerFrame(unsigned int shader, std::vector<LightInfo>& lights, bool isSelection, glm::vec3 cameraPosition, std::vector<glm::mat4>& lightMatrixs);

void initSelectionShader(unsigned int shader);
void updateSelectionShaderPerFrame(unsigned int shader, std::vector<LightInfo>& lights, glm::vec3 cameraPosition, std::vector<glm::mat4>& lightMatrixs);

void initBlurShader(unsigned int shader);

void initDepthShader(unsigned int shader);
void updateDepthShaderPerFrame(unsigned int shader, float near, float far);

void initUiShader(unsigned int shader);
void updateUiShaderPerFrame(unsigned int shader);

void initFramebufferShader(unsigned int shader);
void updateFramebufferShaderFrame(unsigned int shader, float near, float far);

#endif 
