#ifndef MOD_SHADERSTATE
#define MOD_SHADERSTATE

#include "./state.h"

std::vector<UniformData> getDefaultShaderUniforms(std::optional<glm::mat4> projview, glm::vec3 cameraPosition, int numLights, bool enableLighting);
void initDefaultShader(unsigned int shader);
void initSelectionShader(unsigned int shader);
void initBlurShader(unsigned int shader);
void initDepthShader(unsigned int shader);
void initUiShader(unsigned int shader);

void initFramebufferShader(unsigned int shader);
void updateFramebufferShaderFrame(unsigned int shader, float near, float far);

#endif 
