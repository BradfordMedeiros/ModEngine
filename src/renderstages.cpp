#include "./renderstages.h"

struct DeserializedRenderStage {
  std::string name;
  std::string shader;
};

int indexForRenderStage(std::vector<DeserializedRenderStage>& shaders, std::string& name){
  for (int i = 0; i < shaders.size(); i++){
    if (shaders.at(i).name == name){
      return i;
    }
  }
  return -1;  
}
std::vector<DeserializedRenderStage> parseRenderStages(std::string& postprocessingFile){
  auto tokens = parseFormat(loadFile(postprocessingFile));
  std::vector<DeserializedRenderStage> additionalShaders;
  for (auto token : tokens){
    std::cout << "render stages: (" << token.target << ", " << token.attribute << ", " << token.payload << ")" << std::endl; 
    auto indexForStage = indexForRenderStage(additionalShaders, token.target);
    if (indexForStage == -1){
      additionalShaders.push_back(DeserializedRenderStage{
        .name = token.target,
      });
      indexForStage = additionalShaders.size() - 1;
    }
    if (token.attribute == "shader"){
      additionalShaders.at(indexForStage).shader = token.payload;
    }else{
      std::cout << "parse render stages: " << token.target << " - attribute is not supported: " << token.attribute << std::endl;
      assert(false);
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
    auto shaderPath = additionalShaders.at(i).shader;
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
      .intUniforms = {
       // RenderDataInt { .uniformName = "redtint", .value = 2 },
      },
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