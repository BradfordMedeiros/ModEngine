#ifndef MOD_SHADERS
#define MOD_SHADERS 

#include <iostream>
#include <glad/glad.h>
#include <stdexcept>
#include <functional>

unsigned int loadShader(std::string vertexShaderFilepath, std::string fragmentShaderFilepath, std::function<std::string(std::string)> readFile);

#endif 
