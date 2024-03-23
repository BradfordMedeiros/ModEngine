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

std::vector<UniformData> queryUniforms(unsigned int program){
  GLint size;
  GLenum type;
  GLchar name[128];
  GLsizei length;
  GLint count;
  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
  std::vector<UniformData> uniformValues;
  for (int i = 0; i < count; i++){
      glGetActiveUniform(program, (GLuint)i, 128, &length, &size, &type, name);
      //printf("Uniform #%d Type: %u Name: %s\n", i, type, name);

      if (type == GL_FLOAT_VEC4){
        glm::vec4 uniformValue;
        glGetUniformfv(program, glGetUniformLocation(program, name), glm::value_ptr(uniformValue));
        uniformValues.push_back(UniformData {
          .name = name,
          .value = uniformValue,
        });
      }else if (type == GL_FLOAT_VEC3){
        glm::vec3 uniformValue;
        glGetUniformfv(program, glGetUniformLocation(program, name), glm::value_ptr(uniformValue));
        uniformValues.push_back(UniformData {
          .name = name,
          .value = uniformValue,
        });     
      }else if (type == GL_BOOL){
        GLint boolValue;
        glGetUniformiv(program,  glGetUniformLocation(program, name), &boolValue);
        uniformValues.push_back(UniformData {
          .name = name,
          .value = boolValue != 0,
        });            
      }else if (type == GL_FLOAT){
        GLfloat floatValue;
        glGetUniformfv(program, glGetUniformLocation(program, name), &floatValue);
        uniformValues.push_back(UniformData {
          .name = name,
          .value = floatValue,
        });    
      }else if (type == GL_SAMPLER_2D){
        //modassert(false, "sampler 2d not supported");
        GLint textureUnit;
        glGetUniformiv(program, glGetUniformLocation(program, name), &textureUnit);
        uniformValues.push_back(UniformData {
          .name = name,
          .value = Sampler2D {
            .textureUnitId = textureUnit,
          },
        });
      }else if (type == GL_FLOAT_MAT4){
        glm::mat4 value;

        // check this, this might be wrong
        glGetUniformfv(program, glGetUniformLocation(program, name), glm::value_ptr(value));
        uniformValues.push_back(UniformData {
          .name = name,
          .value = value,
        });
      }else {
        modassert(false, std::string("uniform type retrieval not yet supported for this type: ") + name);
      }
  }
  return uniformValues;
}

void assertAllUniformsSet(unsigned int program, std::vector<UniformData>& uniformData, std::vector<const char*>& excludedNames){
  auto allRequiredUniforms = queryUniforms(program);
  for (auto &requiredUniform : allRequiredUniforms){
    bool isExcludedName = false;
    for (auto &name : excludedNames){
      if (requiredUniform.name == name){
        isExcludedName = true;
        break;
      }
    }
    if (isExcludedName){
      continue;
    }

    auto requiredBoolType = std::get_if<bool>(&requiredUniform.value);
    auto requiredFloatType = std::get_if<float>(&requiredUniform.value);
    auto requiredVec3Type = std::get_if<glm::vec3>(&requiredUniform.value);
    auto requiredVec4Type = std::get_if<glm::vec4>(&requiredUniform.value);
    auto requiredSampler2DType = std::get_if<Sampler2D>(&requiredUniform.value);

    bool foundUniform = false;
    bool correctType = false;
    for (auto &uniform : uniformData){
      if (requiredUniform.name == uniform.name){
        foundUniform = true;
        if (requiredBoolType){
          auto boolType = std::get_if<bool>(&uniform.value);
          if (boolType){
            correctType = true;
            break;
          }
        }else if (requiredFloatType){
          auto floatType = std::get_if<float>(&uniform.value);
          if (floatType){
            correctType = true;
            break;
          }
        }else if (requiredVec3Type){
          auto vec3Type = std::get_if<glm::vec3>(&uniform.value);
          if (vec3Type){
            correctType = true;
            break;
          }
        }else if (requiredVec4Type){
          auto vec4Type = std::get_if<glm::vec4>(&uniform.value);
          if (vec4Type){
            correctType = true;
            break;
          }

        }else if (requiredSampler2DType){
          auto sampler2DType = std::get_if<Sampler2D>(&uniform.value);
          if (sampler2DType){
            correctType = true;
            break;
          }
        }{
          modassert(false, "assert types, invalid type");
        }
      }
    }
    modassert(foundUniform, std::string("did not find matching uniform name: ") + requiredUniform.name);
    modassert(correctType, std::string("did not find matching uniform type for name: ") + requiredUniform.name);
  }
}

void setUniformData(unsigned int program, std::vector<UniformData>& uniformData, std::vector<const char*>&& excludedNames){
  assertAllUniformsSet(program, uniformData, excludedNames);
  for (auto &uniform : uniformData){
    auto boolType = std::get_if<bool>(&uniform.value);
    if (boolType){
      glUniform1i(glGetUniformLocation(program, uniform.name.c_str()), *boolType);
      continue;
    }
    auto floatType = std::get_if<float>(&uniform.value);
    if (floatType){
      glUniform1f(glGetUniformLocation(program, uniform.name.c_str()), *floatType);
      continue;
    }
    auto vec3Type = std::get_if<glm::vec3>(&uniform.value);
    if (vec3Type){
      glUniform3fv(glGetUniformLocation(program, uniform.name.c_str()), 1, glm::value_ptr(*vec3Type));
      continue;
    }
    auto vec4Type = std::get_if<glm::vec4>(&uniform.value);
    if (vec4Type){
      glUniform4fv(glGetUniformLocation(program, uniform.name.c_str()), 1, glm::value_ptr(*vec4Type));
      continue;
    }

    auto sampler2DType = std::get_if<Sampler2D>(&uniform.value);
    if (sampler2DType){
      glUniform1i(glGetUniformLocation(program, uniform.name.c_str()), sampler2DType -> textureUnitId);
      continue;
    }

    auto mat4Type = std::get_if<glm::mat4>(&uniform.value);
    if (mat4Type){
      glUniformMatrix4fv(glGetUniformLocation(program, uniform.name.c_str()), 1, GL_FALSE, glm::value_ptr(*mat4Type));
      continue;
    }

    modassert(false, "setUniformData invalid type");
  }
}

std::string print(std::vector<UniformData>& uniforms){
  std::string value = "";
  for (auto &uniform : uniforms){
    value += "[" + uniform.name + " ";
    auto boolPtr = std::get_if<bool>(&uniform.value);
    if (boolPtr){
      value += (*boolPtr ? "true" : "false");
    }
    auto vec3Ptr = std::get_if<glm::vec3>(&uniform.value);
    if (vec3Ptr){
      value += print(*vec3Ptr);
    }
    auto vec4Ptr = std::get_if<glm::vec4>(&uniform.value);
    if (vec4Ptr){
      value += print(*vec4Ptr);
    }
    auto floatPtr = std::get_if<float>(&uniform.value);
    if (floatPtr){
      value += std::to_string(*floatPtr);
    }
    auto sample2DPtr = std::get_if<Sampler2D>(&uniform.value);
    if (sample2DPtr){
      value += std::string("sample2D(") + std::to_string(sample2DPtr -> textureUnitId) + ")";
    }
    value += "] ";
  }
  return value;
}