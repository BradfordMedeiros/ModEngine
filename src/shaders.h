#ifndef SHADERS
#define SHADERS 
#include <iostream>

std::string loadFile(std::string filepath);

struct shaderProgram {
  unsigned int vertexShaderId;
  unsigned int fragmentShaderId;
};

shaderProgram loadShader(std::string fragmentShaderFilepath, std::string vertexShaderFilepath);

#endif 
