#ifndef MOD_SHADERS
#define MOD_SHADERS 

#include <iostream>
#include <glad/glad.h>
#include <stdexcept>
#include <functional>
#include "./common/util.h"

unsigned int loadShader(std::string vertexShaderFilepath, std::string fragmentShaderFilepath, std::function<std::string(std::string)> readFile);

struct ShaderStringVals {
  std::optional<std::string> vertex;
  std::optional<std::string> fragment;
};

ShaderStringVals parseShaderString(std::string& shaderString);

#endif 
