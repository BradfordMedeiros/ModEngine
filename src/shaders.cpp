#include "./shaders.h"

#define SHADER_INFO_LOG_LENGTH 512

std::unordered_map<std::string, ShaderInformation> shaderstringToId; // TODO STATIC

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

// #include "somefilepath"    spacing doesn't matter, but there needs to be a single space between the file and include
std::string getIncludePath(std::string& includeLine){
  auto tokens = filterWhitespace(split(includeLine, ' '));
  modassert(tokens.at(0) == "#include", "getIncludePath not an #include string");
  modassert(tokens.size() == 2, "getIncludePath token size should be 2, extra tokens on string?");
  auto filePath = tokens.at(1);
  modassert(filePath.at(0) == '\"' && filePath.at(filePath.size() -1) == '\"', "getIncludePath invalid path");
  auto filepathTrimmed = filePath.substr(1, filePath.size() - 2);
  return filepathTrimmed;
}

std::vector<std::string> getIncludes(std::string& fileContent, std::function<std::string(std::string)> readFile, std::set<std::string> alreadyIncludedFiles){
  auto values = split(fileContent, '\n');
  std::vector<std::string> finalContent;
  for (auto &value : values){
    if (value.find("#include") != std::string::npos){
      auto includeFilePath = getIncludePath(value);
      modassert(alreadyIncludedFiles.count(includeFilePath) == 0, std::string("shader include loop detected: ") + includeFilePath);
      alreadyIncludedFiles.insert(includeFilePath);
      auto includeStrContent = readFile(includeFilePath);
      auto includeContent = getIncludes(includeStrContent, readFile, alreadyIncludedFiles);
      for (auto &includedValue : includeContent){
        finalContent.push_back(includedValue);
      }
      continue;
    }
    std::cout << value << std::endl;
    finalContent.push_back(value);
  }
  return finalContent;
}

std::string readShaderResolveIncludes(std::string file, std::function<std::string(std::string)> readFile){
  auto shaderStr = readFile(file);
  return join(getIncludes(shaderStr, readFile, { file }), '\n');
}

std::optional<unsigned int> loadShader(std::string vertexShaderFilepath, std::string fragmentShaderFilepath, std::function<std::string(std::string)> readFile, std::unordered_map<std::string, std::string>& args){
   auto programCodeVertex = envSubst(readShaderResolveIncludes(vertexShaderFilepath, readFile), args);
   if (!programCodeVertex.valid){
     modlog("shaders", "failed loading vertex: " + programCodeVertex.error);
     return std::nullopt;
   }
   auto programCodeFragment = envSubst(readShaderResolveIncludes(fragmentShaderFilepath, readFile), args);
   if (!programCodeFragment.valid){
     modlog("shaders", "failed loading fragment: " + programCodeFragment.error);
    return std::nullopt;
   }

   unsigned int vertexShaderId = compileShader(programCodeVertex.result, GL_VERTEX_SHADER);
   shaderError vertexShaderError = checkShaderError(vertexShaderId);
   if (vertexShaderError.isError){
     std::cerr << "ERROR: compiling vertex shader failed: " << vertexShaderError.errorMessage << ", file = " << vertexShaderFilepath <<std::endl;
   }else{
     modlog("shaders", "compiled vertex shader success");
   }

   unsigned int fragmentShaderId = compileShader(programCodeFragment.result, GL_FRAGMENT_SHADER);
   shaderError fragmentShaderError = checkShaderError(fragmentShaderId);
   if (fragmentShaderError.isError){
     std::cerr << "ERROR: compiling fragment shader failed: " << fragmentShaderError.errorMessage << ", file = " << fragmentShaderFilepath << std::endl;
   }else{
     modlog("shaders", "compiled fragment shader success");
   }

   if (vertexShaderError.isError || fragmentShaderError.isError){
     return std::nullopt;
   }
   
   unsigned int shaderProgramId = glCreateProgram();
   glAttachShader(shaderProgramId, vertexShaderId);
   glAttachShader(shaderProgramId, fragmentShaderId);
   glLinkProgram(shaderProgramId);

   glDeleteShader(vertexShaderId);
   glDeleteShader(fragmentShaderId);

   shaderError programError = checkProgramLinkError(shaderProgramId);
   if (programError.isError){
     std::cerr << "ERROR: linking shader program" << programError.errorMessage << std::endl;
     return std::nullopt;
   }
   modlog("shaders", "compiled shader program success");

   return shaderProgramId;
}

unsigned int* loadShaderIntoCache(std::string shaderString, std::string vertexShaderFilepath, std::string fragmentShaderFilepath, std::function<std::string(std::string)> readFile, std::unordered_map<std::string, std::string>& args){
  modassert(shaderstringToId.find(shaderString) == shaderstringToId.end(), "shader already loaded")
  auto shaderId = loadShader(vertexShaderFilepath, fragmentShaderFilepath, readFile, args).value();
  modlog("shader loaded shader", shaderString);
  shaderstringToId[shaderString] = ShaderInformation {
    .programId = shaderId,
    .vertexShader = vertexShaderFilepath,
    .fragmentShader = fragmentShaderFilepath,
  };

  std::ostringstream oss;
  unsigned int* ptr = &shaderstringToId.at(shaderString).programId;
  oss << ptr;
  std::string addr = oss.str();
  modlog("loading shader into cache", shaderString + std::string(" ") + addr);
  return &shaderstringToId.at(shaderString).programId;
}

void reloadShaders(std::function<std::string(std::string)> readFile, std::unordered_map<std::string, std::string>& args){
  modlog("shaders", "reloading");
  for (auto &[shaderString, shaderInfo] : shaderstringToId){
    auto shaderId = loadShader(shaderInfo.vertexShader, shaderInfo.fragmentShader, readFile, args);
    if (shaderId.has_value()){
      glDeleteProgram(shaderInfo.programId);
      shaderInfo.programId = shaderId.value();
    }

    std::ostringstream oss;
    unsigned int* ptr = &shaderstringToId.at(shaderString).programId;
    oss << ptr;
    std::string addr = oss.str();
    modlog("reloading shader into cache", shaderString + std::string(" ")  + addr);

  }
}

void unloadShader(objid shaderId){
  std::optional<std::string> shaderStringToDelete;
  for (auto &[shaderString, shaderInfo] : shaderstringToId){
    if (shaderInfo.programId == shaderId){
      shaderStringToDelete = shaderString;
    } 
  }

  if (shaderStringToDelete.has_value()){
    glDeleteProgram(shaderId);
    shaderstringToId.erase(shaderStringToDelete.value());
  }
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

unsigned int* getShaderByShaderString(std::string& shaderString, std::string& shaderFolderPath, std::function<std::string(std::string)> readFile, std::function<std::unordered_map<std::string, std::string>&()> getArgs, bool* loadedNewShader){
  modassert(shaderString.size() != 0, "getShaderByShaderString shader string size is 0");
  if (loadedNewShader){
    *loadedNewShader = false;
  }
  if (shaderstringToId.find(shaderString) == shaderstringToId.end()){
    auto parsedShaderString = parseShaderString(shaderString);
    auto vertexShaderPath = parsedShaderString.vertex.has_value() ? parsedShaderString.vertex.value() : shaderFolderPath + "/vertex.glsl";
    auto fragmentShaderPath = parsedShaderString.fragment.has_value() ? parsedShaderString.fragment.value() : shaderFolderPath + "/fragment.glsl";
    auto args = getArgs();
    loadShaderIntoCache(shaderString, vertexShaderPath, fragmentShaderPath, readFile, args);
    if (loadedNewShader){
      *loadedNewShader = true;
    }
  }
  return &shaderstringToId.at(shaderString).programId;
}

std::optional<unsigned int> shaderByName(std::string name){
  if (shaderstringToId.find(name) == shaderstringToId.end()){
    return std::nullopt;
  }
  return shaderstringToId.at(name).programId; 
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
      if (requiredUniform.name == name || requiredUniform.name.at(0) == '_'){
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

void setUniformData(unsigned int program, UniformData& uniform){
    auto boolType = std::get_if<bool>(&uniform.value);
    if (boolType){
      glProgramUniform1i(program, glGetUniformLocation(program, uniform.name.c_str()), *boolType);
      return;
    }
    auto floatType = std::get_if<float>(&uniform.value);
    if (floatType){
      glProgramUniform1f(program, glGetUniformLocation(program, uniform.name.c_str()), *floatType);
      return;
    }
    auto vec2Type = std::get_if<glm::vec2>(&uniform.value);
    if (vec2Type){
      glProgramUniform2fv(program, glGetUniformLocation(program, uniform.name.c_str()), 1, glm::value_ptr(*vec2Type));
      return;
    }    
    auto vec3Type = std::get_if<glm::vec3>(&uniform.value);
    if (vec3Type){
      glProgramUniform3fv(program, glGetUniformLocation(program, uniform.name.c_str()), 1, glm::value_ptr(*vec3Type));
      return;
    }
    auto vec4Type = std::get_if<glm::vec4>(&uniform.value);
    if (vec4Type){
      glProgramUniform4fv(program, glGetUniformLocation(program, uniform.name.c_str()), 1, glm::value_ptr(*vec4Type));
      return;
    }

    auto sampler2DType = std::get_if<Sampler2D>(&uniform.value);
    if (sampler2DType){
      glProgramUniform1i(program, glGetUniformLocation(program, uniform.name.c_str()), sampler2DType -> textureUnitId);
      return;
    }

    auto sampleCubeType = std::get_if<SamplerCube>(&uniform.value);
    if (sampleCubeType){
      glProgramUniform1i(program, glGetUniformLocation(program, uniform.name.c_str()), sampleCubeType -> textureUnitId);
      return;
    }


    auto mat4Type = std::get_if<glm::mat4>(&uniform.value);
    if (mat4Type){
      glProgramUniformMatrix4fv(program, glGetUniformLocation(program, uniform.name.c_str()), 1, GL_FALSE, glm::value_ptr(*mat4Type));
      return;
    }

    auto intType = std::get_if<int>(&uniform.value);
    if (intType){
      glProgramUniform1i(program, glGetUniformLocation(program, uniform.name.c_str()), *intType);
      return;
    }

    modassert(false, "setUniformData invalid type");
}

void setUniformData(unsigned int program, std::vector<UniformData>& uniformData, std::vector<const char*>&& excludedNames, bool validateUniforms){
  #ifdef ASSERT_UNIFORMS_SET
  if (validateUniforms){
    assertAllUniformsSet(program, uniformData, excludedNames);
  }
  #endif
  for (auto &uniform : uniformData){
    setUniformData(program, uniform);
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
  for (auto &[shaderString, shaderInfo] : shaderstringToId){
    if (shaderInfo.programId == shaderProgram){
      return shaderString;
    }
  }
  modassert(false, "");
  return std::nullopt;
}



const bool VALIDATE_SHADER_EXISTS = false;
int shaderGetUniform(unsigned int shaderProgram, const char* name){
  auto uniformValue = glGetUniformLocation(shaderProgram, name);
  if (VALIDATE_SHADER_EXISTS){
    modassert(nameByShaderId(shaderProgram).has_value(), "shader does not have a value: ");
  }
  if (uniformValue == -1){
    modassert(false, std::string("uniform value invalid: " + std::string(name) + " " + print(nameByShaderId(shaderProgram))));
  }
  return uniformValue;
}

void shaderSetUniformIVec2(unsigned int shaderProgram, const char* name, glm::ivec2& value){
  glProgramUniform2iv(shaderProgram, shaderGetUniform(shaderProgram, name), 1, glm::value_ptr(value));
}
void shaderSetUniform(unsigned int shaderProgram, const char* name, glm::vec2& value){
  glProgramUniform2fv(shaderProgram, shaderGetUniform(shaderProgram, name), 1, glm::value_ptr(value));
}
void shaderSetUniform(unsigned int shaderProgram, const char* name, glm::vec2&& value){
  shaderSetUniform(shaderProgram, name, value);
}
void shaderSetUniform(unsigned int shaderProgram, const char* name, glm::vec3& value){
  glProgramUniform3fv(shaderProgram, shaderGetUniform(shaderProgram, name), 1, glm::value_ptr(value));
}
void shaderSetUniform(unsigned int shaderProgram, const char* name, glm::vec3&& value){
  shaderSetUniform(shaderProgram, name, value);
}

void shaderSetUniform(unsigned int shaderProgram, const char* name, glm::vec4& value){
  glProgramUniform4fv(shaderProgram, shaderGetUniform(shaderProgram, name), 1, glm::value_ptr(value));
}
void shaderSetUniform(unsigned int shaderProgram, const char* name, glm::vec4&& value){
  shaderSetUniform(shaderProgram, name, value);
}

void shaderSetUniform(unsigned int shaderToUse, const char* name, float& value){
  glProgramUniform1f(shaderToUse, shaderGetUniform(shaderToUse, name), value);
}
void shaderSetUniform(unsigned int shaderToUse, const char* name, float&& value){
  shaderSetUniform(shaderToUse, name, value);
}

void shaderSetUniformInt(unsigned int shaderToUse, const char* name, int& value){
   glProgramUniform1i(shaderToUse, shaderGetUniform(shaderToUse, name), value);
}
void shaderSetUniformInt(unsigned int shaderToUse, const char* name, int&& value){
  shaderSetUniformInt(shaderToUse, name, value);
}

void shaderSetUniformBool(unsigned int shaderToUse, const char* name, bool& value){
  glProgramUniform1i(shaderToUse, shaderGetUniform(shaderToUse, name), value ? 1 : 0);
}
void shaderSetUniformBool(unsigned int shaderToUse, const char* name, bool&& value){
  shaderSetUniformBool(shaderToUse, name, value);
}

void shaderSetUniform(unsigned int shaderToUse, const char* name, glm::mat4& value){
  glProgramUniformMatrix4fv(shaderToUse, shaderGetUniform(shaderToUse, name), 1, GL_FALSE, glm::value_ptr(value));
}
void shaderSetUniform(unsigned int shaderToUse, const char* name, glm::mat4&& value){
  shaderSetUniform(shaderToUse, name, value);
}


#define SHADER_DEBUG_INFO

// https://www.khronos.org/opengl/wiki/Debug_Output
void shaderStartSection(const char* str){
  #ifdef SHADER_DEBUG_INFO
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, str);
  #endif
}
void shaderEndSection(){
  #ifdef SHADER_DEBUG_INFO
    glPopDebugGroup();
  #endif
}

void shaderSetTextureName(const char* name, unsigned int textureId){
  #ifdef SHADER_DEBUG_INFO
    glObjectLabel(GL_TEXTURE, textureId, -1, name);
  #endif
}

void shaderLogDebug(const char* str){
  #ifdef SHADER_DEBUG_INFO  
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,  GL_DEBUG_SEVERITY_MEDIUM, -1 /* -1 => null terminated string*/, str); 
  #endif
}

//////////////////////
ShaderStorageBuffer generateShaderStorageBuffer(size_t size){
  modlog("generateUniformBuffer creating buffer size", std::to_string(size));
  unsigned int ubo;
  glGenBuffers(1, &ubo);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ubo);
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_STATIC_DRAW);
  glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0 /* binding port, aka layout in the uniform block in glsl*/, ubo, 0, size);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  
  return ShaderStorageBuffer {
    .ubo = ubo,
  };
}
void freeBuffer(ShaderStorageBuffer& uniformBuffer){
  glDeleteBuffers(1, &uniformBuffer.ubo);
}
void updateBufferData(ShaderStorageBuffer& uniformBuffer, size_t offsetIntoBuffer, size_t sizeToWrite, void* data){
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniformBuffer.ubo);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, offsetIntoBuffer, sizeToWrite, data);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);  
}

void readBufferData(ShaderStorageBuffer& uniformBuffer, size_t sizeToRead, void* _data){
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniformBuffer.ubo);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeToRead, _data);
}