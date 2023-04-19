#ifndef MOD_SHADERS
#define MOD_SHADERS 

#include <iostream>
#include <glad/glad.h>
#include <stdexcept>
#include <functional>
#include "./common/util.h"

unsigned int loadShader(std::string vertexShaderFilepath, std::string fragmentShaderFilepath, std::function<std::string(std::string)> readFile);
GLint getShaderByShaderString(std::map<std::string, GLint>& shaderstringToId, std::string shaderString, GLint shaderProgram, bool allowShaderOverride, std::string& shaderFolderPath, std::function<std::string(std::string)> readFile, bool* _loadedShader);


#endif 
