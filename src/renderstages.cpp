#include "./renderstages.h"

struct DeserializedRenderStage {
  std::string name;
  std::string shader;
  std::vector<RenderDataInt> intUniforms;
  std::vector<RenderDataFloat> floatUniforms;
  std::vector<RenderDataVec3> vec3Uniforms;
};

enum RenderStageUniformType { RENDER_UNSPECIFIED, RENDER_BOOL, RENDER_FLOAT, RENDER_INT, RENDER_VEC3 };
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
std::vector<DeserializedRenderStage> parseRenderStages(std::string& postprocessingFile){
  auto tokens = parseFormat(loadFile(postprocessingFile));
  std::vector<DeserializedRenderStage> additionalShaders;
  std::map<int, std::map<std::string, RenderStageUniformTypeValue>> stagenameToUniformToValue;

  for (auto token : tokens){
    std::cout << "render stages: (" << token.target << ", " << token.attribute << ", " << token.payload << ")" << std::endl; 
    auto indexForStage = indexForRenderStage(additionalShaders, token.target);
    if (indexForStage == -1){
      additionalShaders.push_back(DeserializedRenderStage{
        .name = token.target,
      });
      indexForStage = additionalShaders.size() - 1;
    }

    auto isUniform = token.attribute.at(0) == '!';
    auto isHint = token.attribute.at(0) == '?';

    if (token.attribute == "shader"){
      additionalShaders.at(indexForStage).shader = token.payload;
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

  // ensure every render step has a shader
  for (auto &additionalShader : additionalShaders){
    if (additionalShader.shader == ""){
      std::cout << "render stages: must specify a shader for: " << additionalShader.name << std::endl;
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

std::vector<RenderStep> parseAdditionalRenderSteps(
  std::string postprocessingFile,
  unsigned int fbo,
  unsigned int framebufferTexture, 
  unsigned int framebufferTexture2
){
  auto additionalShaders = parseRenderStages(postprocessingFile);
  std::vector<RenderStep> additionalRenderSteps;
  for (int i  = 0; i < additionalShaders.size(); i++){
    auto additionalShader = additionalShaders.at(i);
    auto shaderPath = additionalShader.shader;
    unsigned int shaderProgram = loadShader(shaderPath + "/vertex.glsl", shaderPath + "/fragment.glsl");
    bool isEvenIndex = (i % 2) == 0;
    RenderStep renderStep {
      .name = shaderPath.c_str(),
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
      .blend = true,
      .enableStencil = false,
      .intUniforms = additionalShader.intUniforms,
      .floatUniforms = additionalShader.floatUniforms,
      .vec3Uniforms = additionalShader.vec3Uniforms,
    };
    additionalRenderSteps.push_back(renderStep);
  }
  return additionalRenderSteps;
}



// These steps generally assume more knowledge about the pipeline state than would like
// Should make all rendering steps use stages and specify ordering in this
RenderStages loadRenderStages(unsigned int fbo, unsigned int framebufferTexture, unsigned int framebufferTexture2, unsigned int framebufferTexture3,  RenderShaders shaders){
  RenderStep selectionRender {
    .name = "RENDERING-SELECTION",
    .fbo = fbo,
    .colorAttachment0 = framebufferTexture,
    .colorAttachment1 = 0,
    .depthTextureIndex = 0,
    .shader = shaders.selectionProgram,
    .quadTexture = 0,
    .hasColorAttachment1 = false,
    .renderWorld = true,
    .renderSkybox = false,
    .renderQuad = false,
    .blend = false,
    .enableStencil = false,
    .intUniforms = {},
  };
  RenderStep mainRender {
    .name = "MAIN_RENDERING",
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
    .blend = true,
    .enableStencil = true,
    .intUniforms = {},
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
    .fbo = fbo,
    .colorAttachment0 = framebufferTexture3,
    .colorAttachment1 = 0,
    .depthTextureIndex = 0,
    .shader = shaders.blurProgram,
    .quadTexture = framebufferTexture2,
    .hasColorAttachment1 = true,
    .renderWorld = false,
    .renderSkybox = false,
    .renderQuad = true,
    .blend = true,
    .enableStencil = false,
    .intUniforms = {
      RenderDataInt { .uniformName = "useDepthTexture", .value = false },
      RenderDataInt { .uniformName = "firstpass",       .value = true },
      //RenderDataInt { .uniformName = "amount",          .value = static_cast<int>(state.bloomBlurAmount) }
      RenderDataInt { .uniformName = "amount",          .value = 5 }
    },
  };

  RenderStep bloomStep2 {
    .name = "BLOOM-RENDERING-SECOND",
    .fbo = fbo,
    .colorAttachment0 = framebufferTexture2,
    .colorAttachment1 = 0,
    .depthTextureIndex = 0,
    .shader = shaders.blurProgram,
    .quadTexture = framebufferTexture3,
    .hasColorAttachment1 = true,
    .renderWorld = false,
    .renderSkybox = false,
    .renderQuad = true,
    .blend = true,
    .enableStencil = false,
    .intUniforms = {
      RenderDataInt { .uniformName = "useDepthTexture", .value = false },
      RenderDataInt { .uniformName = "firstpass",       .value = false },
      //RenderDataInt { .uniformName = "amount",          .value = static_cast<int>(state.bloomBlurAmount) }
      RenderDataInt { .uniformName = "amount",          .value = 5 }
    },
  };

  auto additionalRenderSteps = parseAdditionalRenderSteps("./res/postprocessing", fbo, framebufferTexture, framebufferTexture2);

  RenderStages stages {
    .selection = selectionRender,
    .main = mainRender,
    .bloom1 = bloomStep1,
    .bloom2 = bloomStep2,
    .additionalRenderSteps = additionalRenderSteps,
  };
  return stages;
}

unsigned int finalRenderingTexture(RenderStages& stages){   // additional render steps ping pong result between framebufferTexture and framebufferTexture2
  if (stages.additionalRenderSteps.size() % 2 == 1){
    return stages.main.colorAttachment1;  
  }
  return stages.main.colorAttachment0;   
}