#include "./renderstages.h"

extern engineState state;  // static-state extern

enum RenderStageUniformType { RENDER_UNSPECIFIED, RENDER_BOOL, RENDER_FLOAT, RENDER_ARR_FLOAT, RENDER_INT, RENDER_VEC3 };
struct RenderStageUniformTypeValue {
  RenderStageUniformType type;
  std::string rawValue;
};

int indexForRenderStage(std::vector<DeserializedRenderStage>& shaders, std::string& name){
  for (int i = 0; i < shaders.size(); i++){
    if (shaders.at(i).name == name){
      return i;
    }
  }
  return -1;  
}

void ensureUniformExists(
  std::map<int, std::map<std::string, RenderStageUniformTypeValue>>& stagenameToUniformToValue,
  int stageindex,
  std::string& uniformName
){
  if (stagenameToUniformToValue.find(stageindex) == stagenameToUniformToValue.end()){
    stagenameToUniformToValue[stageindex] = {};
  }
  if (stagenameToUniformToValue.at(stageindex).find(uniformName) == stagenameToUniformToValue.at(stageindex).end()){
    stagenameToUniformToValue.at(stageindex)[uniformName] = RenderStageUniformTypeValue { 
      .type = RENDER_UNSPECIFIED,
      .rawValue = "",
    };
  }
}

bool isRenderStageToken(Token& token){
  return token.attribute == "shader" || token.attribute == "enable" ||  token.attribute.at(0) == '!' || token.attribute.at(0) == '?' || token.attribute.at(0) == '@' || token.attribute.at(0) == '&';
}

std::vector<DeserializedRenderStage> parseRenderStages(std::vector<Token>& tokens, unsigned int fb0, unsigned int fb1){
  std::vector<DeserializedRenderStage> additionalShaders;
  std::map<int, std::map<std::string, RenderStageUniformTypeValue>> stagenameToUniformToValue;

  for (auto token : tokens){
    std::cout << "render stages: (" << token.target << ", " << token.attribute << ", " << token.payload << ")" << std::endl; 
    modassert(isRenderStageToken(token), "not a valid render token");
    auto indexForStage = indexForRenderStage(additionalShaders, token.target);
    if (indexForStage == -1){
      additionalShaders.push_back(DeserializedRenderStage{
        .name = token.target,
        .enable = true,
      });
      indexForStage = additionalShaders.size() - 1;
    }

    auto isUniform = token.attribute.at(0) == '!';
    auto isHint = token.attribute.at(0) == '?';
    auto isBuiltInUniform = token.attribute.at(0) == '@';
    auto isTexture = token.attribute.at(0) == '&';

    if (token.attribute == "shader"){
      additionalShaders.at(indexForStage).shader = token.payload;
    }else if (token.attribute == "enable"){
      additionalShaders.at(indexForStage).enable = !(token.payload == "false");
    }else if (isTexture){
      auto textureNameInShader = token.attribute.substr(1, token.attribute.size());
      auto isFramebufferType = token.payload.at(0) == '$';
      std::cout << "is framebuffer: " << (isFramebufferType ? "framebuffer" : "normal tex") << std::endl;
      int framebufferTextureId = 0;
      if (isFramebufferType){
        if (token.payload == "$fb0"){
          framebufferTextureId = fb0;
        }else if (token.payload == "$fb1"){
          framebufferTextureId = fb1;
        }else{
          std::cout << "invalid framebuffer number: " << token.payload << std::endl;
          assert(false);
        }
      }

      assert(textureNameInShader.size() > 0);
      additionalShaders.at(indexForStage).textures.push_back(RenderTexture{
        .nameInShader = textureNameInShader,
        .type = isFramebufferType ? RENDER_TEXTURE_FRAMEBUFFER : RENDER_TEXTURE_REGULAR,
        .textureName = token.payload,
        .framebufferTextureId = framebufferTextureId,
      });

      if (additionalShaders.at(indexForStage).textures.size() > 2){
        std::cout << "render steps -> only two textures supported" << std::endl;
        assert(false);
      }
    }else if (isUniform || isHint){
      auto attribute = token.attribute.substr(1, token.attribute.size());
      ensureUniformExists(stagenameToUniformToValue, indexForStage, attribute);
      assert(attribute.size() > 0);
      if (isUniform){
        std::cout << token.attribute << " render stages: is a uniform" << std::endl;
        stagenameToUniformToValue.at(indexForStage).at(attribute).rawValue = token.payload;
      }
      if (isHint){
        std::cout << token.attribute << " render stages: is a hint" << std::endl;
        if (token.payload == "int"){
          stagenameToUniformToValue.at(indexForStage).at(attribute).type = RENDER_INT;
        }else if (token.payload == "bool"){
          stagenameToUniformToValue.at(indexForStage).at(attribute).type = RENDER_BOOL;          
        }else if (token.payload == "float"){
          stagenameToUniformToValue.at(indexForStage).at(attribute).type = RENDER_FLOAT;
        }else if (token.payload == "arr-float"){
          stagenameToUniformToValue.at(indexForStage).at(attribute).type = RENDER_ARR_FLOAT;
        }else if (token.payload == "vec3"){
          stagenameToUniformToValue.at(indexForStage).at(attribute).type = RENDER_VEC3;
        }else{
          std::cout << "render stages: invalid type: " << token.payload << std::endl;
          assert(false);
        }
      }
    }else if (isBuiltInUniform){
      auto attribute = token.attribute.substr(1, token.attribute.size());
      additionalShaders.at(indexForStage).builtInUniforms.push_back(RenderDataBuiltIn{
        .uniformName = attribute,
        .builtin = token.payload,
      });
    }else{
      std::cout << "parse render stages: " << token.target << " - attribute is not supported: " << token.attribute << std::endl;
      assert(false);
    }
  }

  for (auto &[stageIndex, uniformNameToValue] : stagenameToUniformToValue){
    for (auto &[uniformname, uniformValue] : uniformNameToValue){ 
      if (uniformValue.rawValue == ""){
        std::cout << "render stages: uniform value not provided for: " << uniformname << std::endl;
        assert(false);
      }
      if (uniformValue.type == RENDER_INT){
        additionalShaders.at(stageIndex).intUniforms.push_back(RenderDataInt{
          .uniformName = uniformname,
          .value = std::atoi(uniformValue.rawValue.c_str()),
        });
      }else if (uniformValue.type == RENDER_BOOL){
        auto isTrue = uniformValue.rawValue == "true";
        auto isFalse = uniformValue.rawValue == "false";
        if (!isTrue && !isFalse){
          std::cout << "render stages: uniform type unspecified for: " << uniformname << " of wrong type, expected true/false" << std::endl;
          assert(false);
        }
        additionalShaders.at(stageIndex).intUniforms.push_back(RenderDataInt{
          .uniformName = uniformname,
          .value = isTrue ? true : false,
        });
      }else if (uniformValue.type == RENDER_FLOAT){
        additionalShaders.at(stageIndex).floatUniforms.push_back(RenderDataFloat{
          .uniformName = uniformname,
          .value = std::atof(uniformValue.rawValue.c_str()),
        });
      }else if (uniformValue.type == RENDER_ARR_FLOAT){
        additionalShaders.at(stageIndex).floatArrUniforms.push_back(RenderDataFloatArr{
          .uniformName = uniformname,
          .value = parseFloatVec(uniformValue.rawValue),
        });
      }else if (uniformValue.type == RENDER_VEC3){
        additionalShaders.at(stageIndex).vec3Uniforms.push_back(RenderDataVec3{
          .uniformName = uniformname,
          .value = parseVec(uniformValue.rawValue),
        });
      }else{
        std::cout << "render stages: uniform type unspecified for: " << uniformname << std::endl;
        assert(false);
      }
    }
  }

  return additionalShaders; 
}

std::vector<DeserializedRenderStage> filterEnabledShaders(std::vector<DeserializedRenderStage> & unfilteredShaders){
  std::vector<DeserializedRenderStage> shaders;
  for (auto &shader : unfilteredShaders){
    if (shader.enable){
      shaders.push_back(shader);
    }
  }
  return shaders;
}

std::vector<RenderStep> parseAdditionalRenderSteps(
  std::string postprocessingFile,
  unsigned int fbo,
  unsigned int framebufferTexture, 
  unsigned int framebufferTexture2,
  std::function<std::string(std::string)> readFile,
  std::unordered_map<std::string, std::string>& args
){
  auto tokens = parseFormat(readFile(postprocessingFile));
  auto additionalShaders = parseRenderStages(tokens, framebufferTexture, framebufferTexture2);
  for (auto &additionalShader : additionalShaders){
    if (additionalShader.shader == ""){
      std::cout << "render stages: must specify a shader for: " << additionalShader.name << std::endl;
      assert(false);
    }
  }

  additionalShaders = filterEnabledShaders(additionalShaders);

  std::vector<RenderStep> additionalRenderSteps;
  for (int i  = 0; i < additionalShaders.size(); i++){
    auto additionalShader = additionalShaders.at(i);
    auto shaderPath = additionalShader.shader;
    unsigned int* shaderProgram = loadShaderIntoCache(shaderPath, shaderPath + "/vertex.glsl", shaderPath + "/fragment.glsl", readFile, args);
    bool isEvenIndex = (i % 2) == 0;
    RenderStep renderStep {
      .name = additionalShader.name,
      .enable = additionalShader.enable,
      .fbo = fbo,
      .colorAttachment0 = isEvenIndex ? framebufferTexture2 : framebufferTexture,
      .colorAttachment1 = 0,
      .depthTextureIndex = 0,
      .shader = shaderProgram,
      .quadTexture = isEvenIndex ? framebufferTexture : framebufferTexture2,
      .hasColorAttachment1 = false,
      .renderWorld = false,
      .renderSkybox = false,
      .renderQuad = true,
      .renderQuad3D = false,
      .blend = true,
      .enableStencil = false,
      .allowShaderOverride = false,
      .textBoundingOnly = false,
      .uniforms {
        .intUniforms = additionalShader.intUniforms,
        .floatUniforms = additionalShader.floatUniforms,
        .floatArrUniforms = additionalShader.floatArrUniforms,
        .vec3Uniforms = additionalShader.vec3Uniforms,
        .builtInUniforms = additionalShader.builtInUniforms,
      },
      .textures = additionalShader.textures,
    };
    additionalRenderSteps.push_back(renderStep);
  }
  return additionalRenderSteps;
}



// These steps generally assume more knowledge about the pipeline state than would like
// Should make all rendering steps use stages and specify ordering in this
RenderStages loadRenderStages(
  unsigned int fbo, 
  unsigned int framebufferTexture, unsigned int framebufferTexture2, unsigned int framebufferTexture3, unsigned int framebufferTexture4,
  unsigned int* depthTextures, int numDepthTextures,
  unsigned int* portalTextures, int numPortalTextures,
  RenderShaders& shaders,
  std::function<std::string(std::string)> readFile,
  std::unordered_map<std::string, std::string>& args
){
  assert(numDepthTextures > 1);
  assert(numPortalTextures > 1);
  RenderStep selectionRender {
    .name = "RENDERING-SELECTION",
    .enable = true,
    .fbo = fbo,
    .colorAttachment0 = framebufferTexture4,
    .colorAttachment1 = framebufferTexture2,  // this stores UV coord
    .depthTextureIndex = 0,
    .shader = shaders.selectionProgram,
    .quadTexture = 0,
    .hasColorAttachment1 = true,
    .renderWorld = true,
    .renderSkybox = false,
    .renderQuad = false,
    .renderQuad3D = false,
    .blend = false,
    .enableStencil = false,
    .allowShaderOverride = false,
    .textBoundingOnly = true,
    .uniforms {
      .intUniforms = {},
      .floatUniforms = {},
      .floatArrUniforms = {},
      .vec3Uniforms = {},
      .builtInUniforms = {},
    },
    .textures = {},
  };
  RenderStep shadowMapRender {
      .name = "SHADOWMAP-RENDERING",
      .enable = true,
      .fbo = fbo,
      .colorAttachment0 = framebufferTexture, 
      .colorAttachment1 = framebufferTexture2,
      .depthTextureIndex = 1, // but maybe use 0?  doesn't really matter
      .shader = shaders.selectionProgram,
      .quadTexture = 0,
      .hasColorAttachment1 = true,
      .renderWorld = true,
      .renderSkybox = false,
      .renderQuad = false,
      .renderQuad3D = false,
      .blend = true,
      .enableStencil = false,
      .allowShaderOverride = false,
      .textBoundingOnly = false,
      .uniforms {
        .intUniforms = {},
        .floatUniforms = {},
        .floatArrUniforms = {},
        .vec3Uniforms = {},
        .builtInUniforms = {},
      },
      .textures = {},
  };

  RenderStep mainRender {
    .name = "MAIN_RENDERING",
    .enable = true,
    .fbo = fbo,
    .colorAttachment0 = framebufferTexture,
    .colorAttachment1 = framebufferTexture2,
    .depthTextureIndex = 0,
    .shader = shaders.shaderProgram,
    .quadTexture = 0,
    .hasColorAttachment1 = true,
    .renderWorld = true,
    .renderSkybox = true,
    .renderQuad = false,
    .renderQuad3D = false,
    .blend = true,
    .enableStencil = true,
    .allowShaderOverride = true,
    .textBoundingOnly = false,
    .uniforms = {
      .intUniforms = {},
      .floatUniforms = {},
      .floatArrUniforms = {},
      .vec3Uniforms = {},
      .builtInUniforms = {},
    },
    .textures = {},
  };
  RenderStep portalRender {
      .name = "PORTAL-RENDERING",
      .enable = true,
      .fbo = fbo,
      .colorAttachment0 = portalTextures[0], // this gets updated
      .colorAttachment1 = 0,
      .depthTextureIndex = 1, // but maybe use 0?  doesn't really matter
      .shader = shaders.shaderProgram,
      .quadTexture = framebufferTexture,
      .hasColorAttachment1 = false,
      .renderWorld = true,
      .renderSkybox = true,
      .renderQuad = false,
      .renderQuad3D = false,
      .blend = true,
      .enableStencil = false,
      .allowShaderOverride = false,
      .textBoundingOnly = false,
      .uniforms {
        .intUniforms = {},
        .floatUniforms = {},
        .floatArrUniforms = {},
        .vec3Uniforms = {},
        .builtInUniforms = {},
      },
      .textures = {},
  };    

    // depends on framebuffer texture, outputs to framebuffer texture 2
    // Blurring draws the framebuffer texture 
    // The blur program blurs it one in one direction and saves in framebuffer texture 3 
    // then we take framebuffer texture 3, and use that like the original framebuffer texture
    // run it through again, blurring in other fucking direction 
    // We swap to attachment 2 which was just the old bloom attachment for final render pass
    // Big bug:  doesn't sponsor depth value, also just kind of looks bad
  RenderStep bloomStep1 {
    .name = "BLOOM-RENDERING-FIRST",
    .enable = true,
    .fbo = fbo,
    .colorAttachment0 = framebufferTexture3,
    .colorAttachment1 = 0,
    .depthTextureIndex = 1,
    .shader = shaders.blurProgram,
    .quadTexture = framebufferTexture2,
    .hasColorAttachment1 = true,
    .renderWorld = false,
    .renderSkybox = false,
    .renderQuad = true,
    .renderQuad3D = false,
    .blend = true,
    .enableStencil = false,
    .allowShaderOverride = false,
    .textBoundingOnly = false,
    .uniforms = {
      .intUniforms = {
        RenderDataInt { .uniformName = "useDepthTexture", .value = false },
        RenderDataInt { .uniformName = "firstpass",       .value = true },
        RenderDataInt { .uniformName = "amount",          .value = state.bloomBlurAmount },
      },
      .floatUniforms = {},
      .floatArrUniforms = {},
      .vec3Uniforms = {},
      .builtInUniforms = {},
    },
    .textures = {},
  };

  RenderStep bloomStep2 {
    .name = "BLOOM-RENDERING-SECOND",
    .enable = true,
    .fbo = fbo,
    .colorAttachment0 = framebufferTexture2,
    .colorAttachment1 = 0,
    .depthTextureIndex = 1,
    .shader = shaders.blurProgram,
    .quadTexture = framebufferTexture3,
    .hasColorAttachment1 = true,
    .renderWorld = false,
    .renderSkybox = false,
    .renderQuad = true,
    .renderQuad3D = false,
    .blend = true,
    .enableStencil = false,
    .allowShaderOverride = false,
    .textBoundingOnly = false,
    .uniforms = {
      .intUniforms = {
        RenderDataInt { .uniformName = "useDepthTexture", .value = false },
        RenderDataInt { .uniformName = "firstpass",       .value = false },
        //RenderDataInt { .uniformName = "amount",          .value = static_cast<int>(state.bloomBlurAmount) }
        RenderDataInt { .uniformName = "amount",          .value = 5 }
      },
      .floatUniforms = {},
      .floatArrUniforms = {},
      .vec3Uniforms = {},
      .builtInUniforms = {},
    },
    .textures = {},
  };

  RenderStep dof1{
    .name = "DOF-RENDERING",
    .enable = true,
    .fbo = fbo,
    .colorAttachment0 = framebufferTexture3,
    .colorAttachment1 = 0,
    .depthTextureIndex = 1, // but maybe use 0?  doesn't really matter
    .shader = shaders.blurProgram,
    .quadTexture = framebufferTexture,
    .hasColorAttachment1 = false,
    .renderWorld = false,
    .renderSkybox = false,
    .renderQuad = true,
    .renderQuad3D = false,
    .blend = false,
    .enableStencil = false,
    .allowShaderOverride = false,
    .textBoundingOnly = false,
    .uniforms = {
      .intUniforms = {
        RenderDataInt { .uniformName = "firstpass", .value = true },
        RenderDataInt{ .uniformName = "amount", .value = 0 },             // updates during updateRenderStages 
        RenderDataInt{ .uniformName = "useDepthTexture", .value = true },
      },
      .floatUniforms = {
        RenderDataFloat{ .uniformName = "minBlurDistance", .value = 0 },  // updates during updateRenderStages
        RenderDataFloat{ .uniformName = "maxBlurDistance", .value = 0 },  // updates during updateRenderStages
        RenderDataFloat{ .uniformName = "near", .value = 0 },             // updates during updateRenderStages
        RenderDataFloat{ .uniformName = "far", .value = 0 },              // updates during updateRenderStages
      },
      .floatArrUniforms = {},
      .vec3Uniforms = {},
      .builtInUniforms = {},
    },
    .textures = {
      RenderTexture {
        .nameInShader = "framebufferTexture",
        .type = RENDER_TEXTURE_FRAMEBUFFER,
        .textureName = "",
        .framebufferTextureId = framebufferTexture,
      },
      RenderTexture {
        .nameInShader = "depthTexture",
        .type = RENDER_TEXTURE_FRAMEBUFFER,
        .textureName = "",
        .framebufferTextureId = depthTextures[0],
      },
    },
  };

  RenderStep dof2 = dof1;
  dof2.name = "DOF-RENDERING-2";
  dof2.enable = true;
  dof2.colorAttachment0 = framebufferTexture;
  dof2.quadTexture = framebufferTexture3;
  dof2.uniforms.intUniforms.at(0).value = false;
  dof2.textures.at(0).framebufferTextureId = framebufferTexture3;

  auto additionalRenderSteps = parseAdditionalRenderSteps("./res/postprocessing", fbo, framebufferTexture, framebufferTexture2, readFile, args);

  RenderStages stages {
    .selection = selectionRender,
    .shadowmap = shadowMapRender,
    .main = mainRender,
    .portal = portalRender,
    .bloom1 = bloomStep1,
    .bloom2 = bloomStep2,
    .dof1 = dof1,
    .dof2 = dof2,
    .additionalRenderSteps = additionalRenderSteps,
    .portalTextures = portalTextures,
    .numPortalTextures = numPortalTextures,
    .numDepthTextures = numDepthTextures,
  };
  return stages;
}

void updateRenderStages(RenderStages& stages, RenderStagesDofInfo& dofInfo){ 
  stages.dof1.uniforms.intUniforms.at(1).value = dofInfo.blurAmount;
  stages.dof1.uniforms.floatUniforms.at(0).value = dofInfo.minBlurDistance;
  stages.dof1.uniforms.floatUniforms.at(1).value = dofInfo.maxBlurDistance;
  stages.dof1.uniforms.floatUniforms.at(2).value = dofInfo.nearplane;
  stages.dof1.uniforms.floatUniforms.at(3).value = dofInfo.farplane;
  stages.dof2.uniforms.intUniforms.at(1).value = dofInfo.blurAmount;
  stages.dof2.uniforms.floatUniforms.at(0).value = dofInfo.minBlurDistance;
  stages.dof2.uniforms.floatUniforms.at(1).value = dofInfo.maxBlurDistance;
  stages.dof2.uniforms.floatUniforms.at(2).value = dofInfo.nearplane;
  stages.dof2.uniforms.floatUniforms.at(3).value = dofInfo.farplane;
}

void renderStagesSetPortal(RenderStages& stages, unsigned int portalNumber){
  assert(portalNumber < stages.numPortalTextures);
  stages.portal.colorAttachment0 = stages.portalTextures[portalNumber];
}
void renderStagesSetShadowmap(RenderStages& stages, unsigned int shadowmapNumber){
  assert(shadowmapNumber < (stages.numDepthTextures - 1));
  stages.shadowmap.depthTextureIndex = shadowmapNumber + 1;
}

void setRenderStageState(RenderStages& stages, ObjectValue& value){
  RenderStep* step = NULL;
  for (auto &stage : stages.additionalRenderSteps){
    if (stage.name == value.object){
      step = &stage;
      break;
    }
  } 
  if (step != NULL){
    for (auto &uniform : step -> uniforms.floatUniforms){
      if (value.attribute == uniform.uniformName){
        auto floatValue = std::get_if<float>(&value.value);
        assert(floatValue != NULL);
        uniform.value = *floatValue;
        return;
      }
    }
    for (auto &uniform : step -> uniforms.intUniforms){
      if (value.attribute == uniform.uniformName){
        auto floatValue = std::get_if<float>(&value.value);
        assert(floatValue != NULL);
        auto intvalue = static_cast<int>(*floatValue);
        uniform.value = intvalue;
        return;
      }      
    }
    for (auto &uniform : step -> uniforms.floatArrUniforms){
      if (value.attribute == uniform.uniformName){
        auto strValue = std::get_if<std::string>(&value.value);
        assert(strValue != NULL);
        uniform.value = parseFloatVec(*strValue);
        return;
      }      
    }
    for (auto &uniform : step -> uniforms.vec3Uniforms){
      if (value.attribute == uniform.uniformName){
        auto vec3Value = std::get_if<glm::vec3>(&value.value);
        assert(vec3Value != NULL);
        uniform.value = *vec3Value;
        return;
      }     
    }
    for (auto &uniform : step -> uniforms.builtInUniforms){
      if (value.attribute == uniform.uniformName){
        std::cout << "uniform type not supported" << std::endl;
        assert(false);
      }     
    }
    std::cout << "No uniform named: " << value.attribute << std::endl;
    assert(false);
  }else{
    std::cout << "could not find stage name: " << value.object << std::endl;
    assert(false);
  }
}

unsigned int finalRenderingTexture(RenderStages& stages){   // additional render steps ping pong result between framebufferTexture and framebufferTexture2
  if (stages.additionalRenderSteps.size() % 2 == 1){
    return stages.main.colorAttachment1;  
  }
  return stages.main.colorAttachment0;   
}

std::string renderStageToString(RenderStep& step){
  std::string text = step.name + "\n---------\n";
  text = text + "shader: " + std::to_string(*step.shader) + "\n";
  text = text + "enable: " + (step.enable ?  "true" : "false") + "\n";
  text = text + "colorAttachment0: " + std::to_string(step.colorAttachment0) + "\n";
  text = text + "colorAttachment1: " + std::to_string(step.colorAttachment1) + " (has attachment = " + (step.hasColorAttachment1 ? "true" : "false") + ")\n";
  text = text + "renderWorld: " + (step.renderWorld ? "true" : "false") + "\n";
  text = text + "renderSkybox: " + (step.renderSkybox ? "true" : "false") + "\n";
  text = text + "renderQuad: " + (step.renderQuad ? "true" : "false") + "\n";
  text = text + "renderQuad3D: " + (step.renderQuad3D ? "true" : "false") + "\n";


  return text;
}
std::string renderStagesToString(RenderStages& stages){
  std::string renderingSystem = "\n digraph rendering { \n";

  renderingSystem = renderingSystem + "\"" + renderStageToString(stages.selection) + "\" -> \"?\" \n";
  renderingSystem = renderingSystem + "\"" + renderStageToString(stages.shadowmap) + "\" -> \"?\" \n";
  renderingSystem = renderingSystem + "\"" + renderStageToString(stages.main) + "\" -> \"?\" \n";
  renderingSystem = renderingSystem + "\"" + renderStageToString(stages.portal) + "\" -> \"?\" \n";
  renderingSystem = renderingSystem + "\"" + renderStageToString(stages.bloom1) + "\" -> \"?\" \n";
  renderingSystem = renderingSystem + "\"" + renderStageToString(stages.bloom2) + "\" -> \"?\" \n";
  renderingSystem = renderingSystem + "\"" + renderStageToString(stages.dof1) + "\" -> \"?\" \n";
  renderingSystem = renderingSystem + "\"" + renderStageToString(stages.dof2) + "\" -> \"?\" \n";

  for (int i = 0; i < stages.additionalRenderSteps.size(); i++){
    auto& additionalStep = stages.additionalRenderSteps.at(i);
    RenderStep* nextStep = NULL;
    if (i < (stages.additionalRenderSteps.size() - 1)){
      nextStep = &stages.additionalRenderSteps.at(i+1);
    }

    auto leftSide = std::string("\"") + renderStageToString(additionalStep) + " \"" ;
    auto hasRightSide = nextStep != NULL;
    auto rightSide = !hasRightSide ? "" : (std::string("\"") + renderStageToString(*nextStep) + " \"");
    auto fullLine = !hasRightSide ? leftSide : (leftSide + " -> " + rightSide);
    renderingSystem = renderingSystem + fullLine + "\n";

  }
  renderingSystem = renderingSystem + "}";
  return renderingSystem;
}