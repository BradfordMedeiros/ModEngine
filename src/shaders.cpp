#include <iostream>
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <stdexcept>
#include "./shaders.h"

#define SHADER_INFO_LOG_LENGTH 512

std::string loadFile(std::string filepath){
   std::ifstream file(filepath.c_str());
   if (!file.good()){
     throw std::runtime_error("file not found");
   }
   
   std::stringstream buffer;
   buffer << file.rdbuf();
   return buffer.str();
}

struct shaderError {
  bool isError;
  std::string errorMessage;	
};

shaderError checkShaderError (unsigned int shader){
   int success;
   char infoLog[SHADER_INFO_LOG_LENGTH];
   glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
   if (!success){
      glGetShaderInfoLog(shader, SHADER_INFO_LOG_LENGTH, NULL, infoLog); 
   }    
   shaderError error = {
     .isError = !success,
     .errorMessage = success ? "" : infoLog,
   };
   return error;
}

unsigned int compileShader(std::string shaderContent, unsigned int shaderType){
   unsigned int shader = glCreateShader(shaderType); 
   const char* shaderPointer = shaderContent.c_str();
   glShaderSource(shader, 1, &shaderPointer, NULL);
   glCompileShader(shader);
   return shader;
}

shaderProgram loadShader(std::string vertexShaderFilepath, std::string fragmentShaderFilepath){
   unsigned int vertexShaderId = compileShader(loadFile(vertexShaderFilepath), GL_VERTEX_SHADER);
   shaderError vertexShaderError = checkShaderError(vertexShaderId);
   if (vertexShaderError.isError){
     std::cerr << "ERROR: compiling vertex shader failed: " << vertexShaderError.errorMessage << std::endl;
   }else{
     std::cout << "INFO: compiled vertex shader successfully" << std::endl;
   }

   unsigned int fragmentShaderId = compileShader(loadFile(fragmentShaderFilepath), GL_FRAGMENT_SHADER);
   shaderError fragmentShaderError = checkShaderError(fragmentShaderId);
   if (fragmentShaderError.isError){
     std::cerr << "ERROR: compiling fragment shader failed: " << fragmentShaderError.errorMessage << std::endl;
   }else{
     std::cout << "INFO: compiled fragment shader successfully" << std::endl;
   }

   if (vertexShaderError.isError || fragmentShaderError.isError){
     throw std::runtime_error("error compiling shaders"); 
   }
   shaderProgram program = {
     .vertexShaderId = vertexShaderId,
     .fragmentShaderId = fragmentShaderId,
   };
  
   return program;
}



