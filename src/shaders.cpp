#include "./shaders.h"

#define SHADER_INFO_LOG_LENGTH 512

std::map<std::string, GLint> shaderstringToId;

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
     modlog("shaders", "compiled vertex shader success");
   }

   unsigned int fragmentShaderId = compileShader(readFile(fragmentShaderFilepath), GL_FRAGMENT_SHADER);
   shaderError fragmentShaderError = checkShaderError(fragmentShaderId);
   if (fragmentShaderError.isError){
     std::cerr << "ERROR: compiling fragment shader failed: " << fragmentShaderError.errorMessage << std::endl;
   }else{
     modlog("shaders", "compiled fragment shader success");
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
   modlog("shaders", "compiled shader program success");

   glDeleteShader(vertexShaderId);
   glDeleteShader(fragmentShaderId);
   return shaderProgramId;
}

unsigned int loadShaderIntoCache(std::string shaderString, std::string vertexShaderFilepath, std::string fragmentShaderFilepath, std::function<std::string(std::string)> readFile){
  modassert(shaderstringToId.find(shaderString) == shaderstringToId.end(), "shader already loaded")
  auto shaderId = loadShader(vertexShaderFilepath, fragmentShaderFilepath, readFile);
  shaderstringToId[shaderString] = shaderId;
  modlog("loading shader into cache", shaderString + std::string(" ") + std::to_string(shaderId));
  return shaderId;
}

struct ShaderStringVals {
  std::optional<std::string> vertex;
  std::optional<std::string> fragment;
};

ShaderStringVals parseShaderString(std::string& shaderString){
  // format: fragment, vertex (whitespace trimmed). 
  auto values = filterWhitespace(split(shaderString, ','));
  modassert(values.size() == 1 || values.size() == 2, std::string("shader string size must be 1 or 2, got ") + shaderString);
  return ShaderStringVals {
    .vertex = values.size() > 1 ? values.at(1) : std::optional<std::string>(std::nullopt),
    .fragment = values.size() > 0 ? values.at(0) : std::optional<std::string>(std::nullopt),
  };
}

GLint getShaderByShaderString(std::string shaderString, std::string& shaderFolderPath, std::function<std::string(std::string)> readFile){
  modassert(shaderString.size() != 0, "getShaderByShaderString shader string size is 0");
  if (shaderstringToId.find(shaderString) == shaderstringToId.end()){
    auto parsedShaderString = parseShaderString(shaderString);
    auto vertexShaderPath = parsedShaderString.vertex.has_value() ? parsedShaderString.vertex.value() : shaderFolderPath + "/vertex.glsl";
    auto fragmentShaderPath = parsedShaderString.fragment.has_value() ? parsedShaderString.fragment.value() : shaderFolderPath + "/fragment.glsl";
    loadShaderIntoCache(shaderString, vertexShaderPath, fragmentShaderPath, readFile);
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
      }else if (type == GL_FLOAT_VEC2){
        glm::vec2 uniformValue;
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
        GLint textureUnit;
        glGetUniformiv(program, glGetUniformLocation(program, name), &textureUnit);
        uniformValues.push_back(UniformData {
          .name = name,
          .value = Sampler2D {
            .textureUnitId = textureUnit,
          },
        });
      }else if (type == GL_SAMPLER_CUBE){
        GLint textureUnit;
        glGetUniformiv(program, glGetUniformLocation(program, name), &textureUnit);
        uniformValues.push_back(UniformData {
          .name = name,
          .value = SamplerCube {
            .textureUnitId = textureUnit,
          },
        });
      }else if (type == GL_INT){
        int intValue;
        glGetUniformiv(program, glGetUniformLocation(program, name), &intValue);
        uniformValues.push_back(UniformData {
          .name = name,
          .value = intValue,
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
    auto requiredVec2Type = std::get_if<glm::vec2>(&requiredUniform.value);
    auto requiredVec3Type = std::get_if<glm::vec3>(&requiredUniform.value);
    auto requiredVec4Type = std::get_if<glm::vec4>(&requiredUniform.value);
    auto requiredSampler2DType = std::get_if<Sampler2D>(&requiredUniform.value);
    auto requiredSamplerCubeType = std::get_if<SamplerCube>(&requiredUniform.value);
    auto requiredMat4Type = std::get_if<glm::mat4>(&requiredUniform.value);
    auto requiredIntType = std::get_if<int>(&requiredUniform.value);

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
        }else if (requiredVec2Type){
          auto vec2Type = std::get_if<glm::vec2>(&uniform.value);
          if (vec2Type){
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
        }else if (requiredSamplerCubeType){
          auto sampleCubeType = std::get_if<SamplerCube>(&uniform.value);
          if (sampleCubeType){
            correctType = true;
            break;
          }
        }else if (requiredMat4Type){
          auto mat4Type = std::get_if<glm::mat4>(&uniform.value);
          if (mat4Type){
            correctType = true;
            break;
          } 
        }else if (requiredIntType){
          auto intType = std::get_if<int>(&uniform.value);
          if (intType){
            correctType = true;
            break;            
          }
        }
        modassert(false, "assert types, invalid type");
      }
    }
    modassert(foundUniform, std::string("did not find matching uniform name: ") + requiredUniform.name);
    modassert(correctType, std::string("did not find matching uniform type for name: ") + requiredUniform.name);
  }
}

#define ASSERT_UNIFORMS_SET

void setUniformData(unsigned int program, std::vector<UniformData>& uniformData, std::vector<const char*>&& excludedNames){
  #ifdef ASSERT_UNIFORMS_SET
  assertAllUniformsSet(program, uniformData, excludedNames);
  #endif
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
    auto vec2Type = std::get_if<glm::vec2>(&uniform.value);
    if (vec2Type){
      glUniform2fv(glGetUniformLocation(program, uniform.name.c_str()), 1, glm::value_ptr(*vec2Type));
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

    auto sampleCubeType = std::get_if<SamplerCube>(&uniform.value);
    if (sampleCubeType){
      glUniform1i(glGetUniformLocation(program, uniform.name.c_str()), sampleCubeType -> textureUnitId);
      continue;
    }


    auto mat4Type = std::get_if<glm::mat4>(&uniform.value);
    if (mat4Type){
      glUniformMatrix4fv(glGetUniformLocation(program, uniform.name.c_str()), 1, GL_FALSE, glm::value_ptr(*mat4Type));
      continue;
    }

    auto intType = std::get_if<int>(&uniform.value);
    if (intType){
      glUniform1i(glGetUniformLocation(program, uniform.name.c_str()), *intType);
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
    auto vec2Ptr = std::get_if<glm::vec2>(&uniform.value);
    if (vec2Ptr){
      value += print(*vec2Ptr);
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

    auto sampleCubePtr = std::get_if<SamplerCube>(&uniform.value);
    if (sampleCubePtr){
      value += std::string("sampleCube(") + std::to_string(sampleCubePtr -> textureUnitId) + ")";
    }

    auto intPtr = std::get_if<int>(&uniform.value);
    if (intPtr){
      value += std::to_string(*intPtr);
    }
    value += "] ";
  }
  return value;
}

std::optional<std::string> nameByShaderId(GLint shaderProgram){
  for (auto &[shaderString, shaderId] : shaderstringToId){
    if (shaderId == shaderProgram){
      modlog("shader nameByShaderId lookup", "found");
      return shaderString;
    }
    modlog("shader name to id: ", shaderString);
  }
  modlog("shader nameByShaderId lookup", std::string("not found: ") + std::to_string(shaderProgram));
  modassert(false, "");
  return std::nullopt;
}

int shaderGetUniform(unsigned int shaderProgram, const char* name){
  auto uniformValue = glGetUniformLocation(shaderProgram, name);
  //modassert(uniformValue != -1, std::string("uniform value invalid: " + std::string(name) + " " + print(nameByShaderId(uniformValue))));
  return uniformValue;
}