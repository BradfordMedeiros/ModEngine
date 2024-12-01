#ifndef MOD_SHADERS
#define MOD_SHADERS 

#include <iostream>
#include <glad/glad.h>
#include <stdexcept>
#include <functional>
#include <glm/gtc/type_ptr.hpp>
#include "./common/util.h"

unsigned int loadShaderIntoCache(std::string shaderString, std::string vertexShaderFilepath, std::string fragmentShaderFilepath, std::function<std::string(std::string)> readFile);
GLint getShaderByShaderString(std::string shaderString, std::string& shaderFolderPath, std::function<std::string(std::string)> readFile);


struct Sampler2D { 
  int textureUnitId; 
};
struct SamplerCube {
  int textureUnitId;
};
typedef std::variant<bool, float, glm::vec2, glm::vec3, glm::vec4, Sampler2D, SamplerCube, glm::mat4, int> UniformValue;
struct UniformData {
  std::string name;
  UniformValue value;
};
void setUniformData(unsigned int program, std::vector<UniformData>& uniformData, std::vector<const char*>&& excludedNames);
std::string print(std::vector<UniformData>& uniforms);

int shaderGetUniform(unsigned int program, const char* name);

void shaderSetUniformIVec2(unsigned int program, const char* name, glm::ivec2& value);
void shaderSetUniform(unsigned int program, const char* name, glm::vec2& value);
void shaderSetUniform(unsigned int program, const char* name, glm::vec2&& value);
void shaderSetUniform(unsigned int program, const char* name, glm::vec3& value);
void shaderSetUniform(unsigned int program, const char* name, glm::vec3&& value);
void shaderSetUniform(unsigned int program, const char* name, glm::vec4& value);
void shaderSetUniform(unsigned int program, const char* name, glm::vec4&& value);
void shaderSetUniform(unsigned int program, const char* name, float& value);
void shaderSetUniform(unsigned int program, const char* name, float&& value);
void shaderSetUniformInt(unsigned int program, const char* name, int& value);
void shaderSetUniformInt(unsigned int program, const char* name, int&& value);
void shaderSetUniformBool(unsigned int shaderToUse, const char* name, bool& value);
void shaderSetUniformBool(unsigned int shaderToUse, const char* name, bool&& value);
void shaderSetUniform(unsigned int program, const char* name, glm::mat4& value);
void shaderSetUniform(unsigned int program, const char* name, glm::mat4&& value);

void shaderStartSection(const char* str);
void shaderEndSection();
void shaderLogDebug(const char* str);
void shaderSetTextureName(const char* name, unsigned int textureId);


// https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL
struct UniformBuffer {
  GLuint ubo;
};
UniformBuffer generateUniformBuffer(size_t size);
void freeBuffer(UniformBuffer& uniformBuffer);
void updateBufferData(UniformBuffer& uniformBuffer, size_t offsetIntoBuffer, size_t sizeToWrite, void* data);

#endif 
