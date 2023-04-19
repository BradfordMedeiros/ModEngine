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

unsigned int loadShader(std::string vertexShaderFilepath, std::string fragmentShaderFilepath, std::function<std::string(std::string)> readFile){
   unsigned int vertexShaderId = compileShader(readFile(vertexShaderFilepath), GL_VERTEX_SHADER);
   shaderError vertexShaderError = checkShaderError(vertexShaderId);
   if (vertexShaderError.isError){
     std::cerr << "ERROR: compiling vertex shader failed: " << vertexShaderError.errorMessage << std::endl;
   }else{
     std::cout << "INFO: compiled vertex shader successfully" << std::endl;
   }

   unsigned int fragmentShaderId = compileShader(readFile(fragmentShaderFilepath), GL_FRAGMENT_SHADER);
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

struct ShaderStringVals {
  std::optional<std::string> vertex;
  std::optional<std::string> fragment;
};

ShaderStringVals parseShaderString(std::string& shaderString){
  // format: fragment, vertex (whitespace trimmed). 
  auto values = filterWhitespace(split(shaderString, ','));
  modassert(values.size() == 1 || values.size() == 2, "shader string size must be 1 or 2, got " + shaderString);
  return ShaderStringVals {
    .vertex = values.size() > 1 ? values.at(1) : std::optional<std::string>(std::nullopt),
    .fragment = values.size() > 0 ? values.at(0) : std::optional<std::string>(std::nullopt),
  };
}

GLint getShaderByShaderString(std::map<std::string, GLint>& shaderstringToId, std::string shaderString, GLint shaderProgram, bool allowShaderOverride, std::string& shaderFolderPath, std::function<std::string(std::string)> readFile, bool* _loadedShader){
  *_loadedShader = false;
  if (shaderString == "" || !allowShaderOverride){
    return shaderProgram;
  }
  if (shaderstringToId.find(shaderString) == shaderstringToId.end()){
    auto parsedShaderString = parseShaderString(shaderString);
    auto vertexShaderPath = parsedShaderString.vertex.has_value() ? parsedShaderString.vertex.value() : shaderFolderPath + "/vertex.glsl";
    auto fragmentShaderPath = parsedShaderString.fragment.has_value() ? parsedShaderString.fragment.value() : shaderFolderPath + "/fragment.glsl";

    auto shaderId = loadShader(vertexShaderPath, fragmentShaderPath, readFile);
    shaderstringToId[shaderString] = shaderId;
    *_loadedShader = true;
  }
  return shaderstringToId.at(shaderString);
}
