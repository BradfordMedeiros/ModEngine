#include "./shaders.h"

#define SHADER_INFO_LOG_LENGTH 512

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
shaderError checkProgramLinkError(unsigned int program){
  int success;
  char infoLog[SHADER_INFO_LOG_LENGTH];
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success){
    glGetProgramInfoLog(program, SHADER_INFO_LOG_LENGTH, NULL, infoLog);
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

unsigned int loadShader(std::string vertexShaderFilepath, std::string fragmentShaderFilepath){
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
   
   unsigned int shaderProgramId = glCreateProgram();
   glAttachShader(shaderProgramId, vertexShaderId);
   glAttachShader(shaderProgramId, fragmentShaderId);
   glLinkProgram(shaderProgramId);
   shaderError programError = checkProgramLinkError(shaderProgramId);
   if (programError.isError){
     std::cerr << "ERROR: linking shader program" << programError.errorMessage << std::endl;
     throw std::runtime_error("error linking shaders");
   }
   std::cout << "INFO: compiled shader program successfully" << std::endl;

   glDeleteShader(vertexShaderId);
   glDeleteShader(fragmentShaderId);
   return shaderProgramId;
}

