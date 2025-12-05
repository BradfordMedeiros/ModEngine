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
  std::unordered_map<int, std::unordered_map<std::string, RenderStageUniformTypeValue>>& stagenameToUniformToValue,
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
  std::unordered_map<int, std::unordered_map<std::string, RenderStageUniformTypeValue>> stagenameToUniformToValue;

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
    .colorAttachment2 = std::nullopt,
    .depthTextureIndex = 0,
    .shader = shaders.selectionProgram,
    .quadTexture = 0,
    .renderWorld = false,
    .renderSkybox = false,
    .renderQuad = false,
    .renderQuad3D = false,
    .blend = false,
    .enableStencil = false,
    .allowShaderOverride = false,
    .textBoundingOnly = true,
    .renderParticles = false,
    .uniforms {
      .intUniforms = {},
      .floatUniforms = {},
      .floatArrUniforms = {},
      .vec3Uniforms = {},
    },
    .textures = {},
  };
  RenderStep shadowMapRender {
      .name = "SHADOWMAP-RENDERING",
      .enable = true,
      .fbo = fbo,
      .colorAttachment0 = framebufferTexture, 
      .colorAttachment1 = framebufferTexture2,
      .colorAttachment2 = std::nullopt,
      .depthTextureIndex = 1, // but maybe use 0?  doesn't really matter
      .shader = shaders.selectionProgram,
      .quadTexture = 0,
      .renderWorld = true,
      .renderSkybox = false,
      .renderQuad = false,
      .renderQuad3D = false,
      .blend = true,
      .enableStencil = false,
      .allowShaderOverride = false,
      .textBoundingOnly = false,
      .renderParticles = false,
      .uniforms {
        .intUniforms = {},
        .floatUniforms = {},
        .floatArrUniforms = {},
        .vec3Uniforms = {},
      },
      .textures = {},
  };

  RenderStep mainRender {
    .name = "MAIN_RENDERING",
    .enable = true,
    .fbo = fbo,
    .colorAttachment0 = framebufferTexture,   // normal rgb
    .colorAttachment1 = framebufferTexture2,  // filtered values to pass to bloom (can I just do this filtering in the bloom filter?)
    .colorAttachment2 = framebufferTexture3,  // selection id
    .colorAttachment3 = framebufferTexture4,   // uvcoords
    .depthTextureIndex = 0,
    .shader = shaders.shaderProgram,
    .quadTexture = 0,
    .renderWorld = true,
    .renderSkybox = true,
    .renderQuad = false,
    .renderQuad3D = false,
    .blend = true,
    .enableStencil = true,
    .allowShaderOverride = true,
    .textBoundingOnly = false,
    .renderParticles = true,
    .uniforms = {
      .intUniforms = {},
      .floatUniforms = {},
      .floatArrUniforms = {},
      .vec3Uniforms = {},
    },
    .textures = {},
  };
  RenderStep portalRender {
      .name = "PORTAL-RENDERING",
      .enable = true,
      .fbo = fbo,
      .colorAttachment0 = portalTextures[0], // this gets updated
      .colorAttachment1 = std::nullopt,
      .colorAttachment2 = std::nullopt,
      .depthTextureIndex = 1, // but maybe use 0?  doesn't really matter
      .shader = shaders.shaderProgram,
      .quadTexture = framebufferTexture,
      .renderWorld = true,
      .renderSkybox = true,
      .renderQuad = false,
      .renderQuad3D = false,
      .blend = true,
      .enableStencil = false,
      .allowShaderOverride = false,
      .textBoundingOnly = false,
      .renderParticles = false,
      .uniforms {
        .intUniforms = {},
        .floatUniforms = {},
        .floatArrUniforms = {},
        .vec3Uniforms = {},
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
    .colorAttachment2 = std::nullopt,
    .depthTextureIndex = 1,
    .shader = shaders.blurProgram,
    .quadTexture = framebufferTexture2,
    .renderWorld = false,
    .renderSkybox = false,
    .renderQuad = true,
    .renderQuad3D = false,
    .blend = true,
    .enableStencil = false,
    .allowShaderOverride = false,
    .textBoundingOnly = false,
    .renderParticles = false,
    .uniforms = {
      .intUniforms = {
        RenderDataInt { .uniformName = "useDepthTexture", .value = false },
        RenderDataInt { .uniformName = "firstpass",       .value = true },
        RenderDataInt { .uniformName = "amount",          .value = state.bloomBlurAmount },
      },
      .floatUniforms = {},
      .floatArrUniforms = {},
      .vec3Uniforms = {},
    },
    .textures = {},
  };

  RenderStep bloomStep2 {
    .name = "BLOOM-RENDERING-SECOND",
    .enable = true,
    .fbo = fbo,
    .colorAttachment0 = framebufferTexture2,
    .colorAttachment1 = 0,
    .colorAttachment2 = std::nullopt,
    .depthTextureIndex = 1,
    .shader = shaders.blurProgram,
    .quadTexture = framebufferTexture3,
    .renderWorld = false,
    .renderSkybox = false,
    .renderQuad = true,
    .renderQuad3D = false,
    .blend = true,
    .enableStencil = false,
    .allowShaderOverride = false,
    .textBoundingOnly = false,
    .renderParticles = false,
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
    },
    .textures = {},
  };

  RenderStep dof1{
    .name = "DOF-RENDERING",
    .enable = true,
    .fbo = fbo,
    .colorAttachment0 = framebufferTexture3,
    .colorAttachment1 = std::nullopt,
    .colorAttachment2 = std::nullopt,
    .depthTextureIndex = 1, // but maybe use 0?  doesn't really matter
    .shader = shaders.blurProgram,
    .quadTexture = framebufferTexture,
    .renderWorld = false,
    .renderSkybox = false,
    .renderQuad = true,
    .renderQuad3D = false,
    .blend = false,
    .enableStencil = false,
    .allowShaderOverride = false,
    .textBoundingOnly = false,
    .renderParticles = false,
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

  RenderStages stages {
    .selection = selectionRender,
    .shadowmap = shadowMapRender,
    .main = mainRender,
    .portal = portalRender,
    .bloom1 = bloomStep1,
    .bloom2 = bloomStep2,
    .dof1 = dof1,
    .dof2 = dof2,
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

unsigned int finalRenderingTexture(RenderStages& stages){
  return stages.main.colorAttachment0;   
}

std::string renderStageToString(RenderStep& step){
  std::string text = step.name + "\n---------\n";
  text = text + "shader: " + std::to_string(*step.shader) + "\n";
  text = text + "enable: " + (step.enable ?  "true" : "false") + "\n";
  text = text + "colorAttachment0: " + std::to_string(step.colorAttachment0) + "\n";
  //text = text + "colorAttachment1: " + std::to_string(step.colorAttachment1) + " (has attachment = " + (step.hasColorAttachment1 ? "true" : "false") + ")\n";
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

  renderingSystem = renderingSystem + "}";
  return renderingSystem;
}