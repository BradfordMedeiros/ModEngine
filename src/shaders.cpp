#include <iostream>
#include "./shaders.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <stdexcept>

std::string loadFile(std::string filepath){
   std::ifstream file(filepath.c_str());
   if (!file.good()){
     throw std::runtime_error("file not found");
   }
   
   std::stringstream buffer;
   buffer << file.rdbuf();
   return buffer.str();
}


void loadShader(std::string vertexShaderFilepath, std::string fragmentShaderFilepath){
   std::string vertexShader = loadFile(vertexShaderFilepath);
   std::cout << "vertex shader content is: " << vertexShader << std::endl;
   //unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); 
}
