#ifndef MOD_SHADERS
#define MOD_SHADERS 

#include <iostream>
#include <glad/glad.h>
#include <stdexcept>
#include <functional>
#include <glm/gtc/type_ptr.hpp>
#include "./common/util.h"

struct ShaderInformation {
  unsigned int programId;
  std::string vertexShader;
  std::string fragmentShader;
};

unsigned int* loadShaderIntoCache(std::string shaderString, std::string vertexShaderFilepath, std::string fragmentShaderFilepath, std::function<std::string(std::string)> readFile, std::unordered_map<std::string, std::string>& args);
unsigned int* getShaderByShaderString(std::string& shaderString, std::string& shaderFolderPath, std::function<std::string(std::string)> readFile, std::function<std::unordered_map<std::string, std::string>&()> getArgs, bool* loadedNewShader);
std::optional<unsigned int> shaderByName(std::string name);
void reloadShaders(std::function<std::string(std::string)> readFile, std::unordered_map<std::string, std::string>& args);
void unloadShader(objid shaderId);

void setUniformData(unsigned int program, UniformData& uniform);
void setUniformData(unsigned int program, std::vector<UniformData>& uniformData, std::vector<const char*>&& excludedNames, bool validateUniforms = true);
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
struct ShaderStorageBuffer {
  GLuint ubo;
};
ShaderStorageBuffer generateShaderStorageBuffer(size_t size);
void freeBuffer(ShaderStorageBuffer& uniformBuffer);
void updateBufferData(ShaderStorageBuffer& uniformBuffer, size_t offsetIntoBuffer, size_t sizeToWrite, void* data);
void readBufferData(ShaderStorageBuffer& uniformBuffer, size_t sizeToRead, void* _data);

#endif 
