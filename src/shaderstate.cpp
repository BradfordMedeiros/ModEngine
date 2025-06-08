#include "./shaderstate.h"


float getTotalTimeGame();
float getTotalTime();
extern engineState state;
extern glm::mat4 ndiOrtho;
extern RenderingResources renderingResources;


void initDefaultShader(unsigned int shader){
  std::vector<UniformData> uniformData;
  uniformData.push_back(UniformData {
    .name = "maintexture",
    .value = Sampler2D {
      .textureUnitId = 0,
    },
  });
  uniformData.push_back(UniformData {
    .name = "emissionTexture",
    .value = Sampler2D {
      .textureUnitId = 1,
    },
  });
  uniformData.push_back(UniformData {
    .name = "opacityTexture",
    .value = Sampler2D {
      .textureUnitId = 2,
    },
  });
  uniformData.push_back(UniformData {
    .name = "lightDepthTexture",
    .value = Sampler2D {
      .textureUnitId = 3,
    },
  });
  uniformData.push_back(UniformData {
    .name = "cubemapTexture",
    .value = SamplerCube {
      .textureUnitId = 4,
    },
  });
  uniformData.push_back(UniformData {
    .name = "roughnessTexture",
    .value = Sampler2D {
      .textureUnitId = 5,
    },
  });
  uniformData.push_back(UniformData {
    .name = "normalTexture",
    .value = Sampler2D {
      .textureUnitId = 6,
    },
  });
  uniformData.push_back(UniformData {
    .name = "enableLighting",
    .value = true,
  });

  setUniformData(shader, uniformData, { 
  	"ambientAmount", "bloomThreshold",
	 	"bones[0]", "hasBones", "cameraPosition", "discardTexAmount", "emissionAmount", "enableAttenutation", "enableDiffuse",
	 	"enableLighting", "enablePBR", "enableShadows", "enableSpecular",
 		"model", "numlights", "shadowIntensity", "useBoneTransform",
    "hasCubemapTexture", "hasDiffuseTexture", "hasEmissionTexture", "hasNormalTexture", "hasOpacityTexture",
    "lights[0]", "lightsangledelta[0]", "lightsatten[0]", "lightscolor[0]", "lightsdir[0]", "lightsisdir[0]", "lightsmaxangle[0]", "voxelindexs2[0]", "voxelcellwidth",
    "lightsprojview", "textureOffset", "textureSize", "textureTiling", "tint", "projview", "realtime", "time",

    "encodedid", "textureid",
    "forceTint", // selection shader only
    "useInstancing", "instanceOffsets[0]",
    "groupToModel",
  });
}
void updateDefaultShaderPerFrame(unsigned int shader, std::vector<LightInfo>& lights, bool isSelection, glm::vec3 cameraPosition, std::vector<glm::mat4>& lightMatrixs){
  std::vector<UniformData> uniformData;
  /*if (projview.has_value()){
    uniformData.push_back(UniformData {
      .name = "projview",
      .value = projview.value(),
    });
  }
  uniformData.push_back(UniformData {
    .name = "cameraPosition",
    .value = cameraPosition,
  });
  uniformData.push_back(UniformData {
    .name = "numlights",
    .value = numLights,
  });
  uniformData.push_back(UniformData {
    .name = "enableLighting",
    .value = enableLighting,
  });
*/

  shaderSetUniform(shader, "cameraPosition", cameraPosition);

  if (!isSelection){
    if (lightMatrixs.size() > 0){
      shaderSetUniform(shader, "lightsprojview", lightMatrixs.at(0)); // TODO we only use one of the light depth textures in the shader right now 
    }

    auto lightUpdates = getLightUpdates();
    for (auto &lightUpdate : lightUpdates){
      //std::cout << "voxel lighting : " << lightUpdate.index << std::endl;
      int lightArrayIndex = getLightsArrayIndex(lights, lightUpdate.lightIndex);
      modassert(lightUpdate.index < 512, "lightUpdate.index too large");
      updateBufferData(renderingResources.voxelLighting , sizeof(glm::vec4) * lightUpdate.index, sizeof(int), &lightArrayIndex);
    }
  
    shaderSetUniformInt(shader, "voxelcellwidth", getLightingCellWidth());

    for (int i = 0; i < lights.size(); i++){
      glm::vec3 position = lights.at(i).transform.position;
      auto& light = lights.at(i); 
      shaderSetUniform(shader, ("lights[" + std::to_string(i) + "]").c_str(), position);
      if (!light.light.disabled){
        shaderSetUniform(shader, ("lightscolor[" + std::to_string(i) + "]").c_str(), light.light.color);
      }else{
        shaderSetUniform(shader, ("lightscolor[" + std::to_string(i) + "]").c_str(), glm::vec3(0.f, 0.f, 0.f));
      }
      shaderSetUniform(shader, ("lightsdir[" + std::to_string(i) + "]").c_str(), directionFromQuat(light.transform.rotation));
      shaderSetUniform(shader, ("lightsatten[" + std::to_string(i) + "]").c_str(), light.light.attenuation);
      shaderSetUniform(shader,  ("lightsmaxangle[" + std::to_string(i) + "]").c_str(), light.light.type == LIGHT_SPOTLIGHT ? light.light.maxangle : -10.f);
      shaderSetUniform(shader,  ("lightsangledelta[" + std::to_string(i) + "]").c_str(), light.light.angledelta);
      shaderSetUniformBool(shader,  ("lightsisdir[" + std::to_string(i) + "]").c_str(), light.light.type == LIGHT_DIRECTIONAL);
    }    


    uniformData.push_back(UniformData { 
      .name = "numlights", 
      .value = static_cast<int>(lights.size()) 
    });
  }


 // uniformData.push_back(UniformData {
 //   .name = "voxelcellwidth",
 //   .value = getLightingCellWidth(),
 // });

  uniformData.push_back(UniformData {
    .name = "showBoneWeight",
    .value = state.showBoneWeight,
  });
  uniformData.push_back(UniformData {
    .name = "useBoneTransform",
    .value = state.useBoneTransform,
  });
  uniformData.push_back(UniformData {
    .name = "enableDiffuse",
    .value = state.enableDiffuse,
  });
  uniformData.push_back(UniformData {
    .name = "enablePBR",
    .value = state.enablePBR,
  });
  uniformData.push_back(UniformData {
    .name = "enableSpecular",
    .value = state.enableSpecular,
  });
  uniformData.push_back(UniformData {
    .name = "enableVoxelLighting",
    .value = state.enableVoxelLighting,
  });
  uniformData.push_back(UniformData {
    .name = "ambientAmount",
    .value = glm::vec3(state.ambient),
  });
  uniformData.push_back(UniformData {
    .name = "bloomThreshold",
    .value = state.bloomThreshold,
  });
  uniformData.push_back(UniformData {
    .name = "enableAttenutation",
    .value = state.enableAttenuation,
  });
  uniformData.push_back(UniformData {
    .name = "shadowIntensity",
    .value = state.shadowIntensity,
  });
  uniformData.push_back(UniformData {
    .name = "enableShadows",
    .value = state.enableShadows,
  });
  uniformData.push_back(UniformData {
    .name = "time",
    .value = getTotalTimeGame(),
  });
  uniformData.push_back(UniformData {
    .name = "realtime",
    .value = getTotalTime(),
  });


  setUniformData(shader, uniformData, {
    "textureid", "bones[0]", "encodedid", "hasBones", "model", "discardTexAmount", 
    "emissionAmount", 
    "hasCubemapTexture", "hasDiffuseTexture", "hasEmissionTexture", "hasNormalTexture", "hasOpacityTexture",
    "lights[0]", "lightsangledelta[0]", "lightsatten[0]", "lightscolor[0]", "lightsdir[0]", "lightsisdir[0]", "lightsmaxangle[0]", "voxelindexs2[0]", "colors[0]", "voxelcellwidth",
    "lightsprojview", "textureOffset", "textureSize", "textureTiling", "tint", "projview",

    "maintexture", "textureid", "emissionTexture", "opacityTexture", "lightDepthTexture", "cubemapTexture", "roughnessTexture", "normalTexture",
    
    "cameraPosition", "projview", "numlights", "enableLighting",
    "forceTint", // selection shader only
    "useInstancing", "instanceOffsets[0]",
    "groupToModel",
  });



}


void initSelectionShader(unsigned int shader){
  initDefaultShader(shader);
}
void updateSelectionShaderPerFrame(unsigned int shader, std::vector<LightInfo>& lights, glm::vec3 cameraPosition, std::vector<glm::mat4>& lightMatrixs){
  updateDefaultShaderPerFrame(shader, lights, true, cameraPosition, lightMatrixs);
}


void initBlurShader(unsigned int shader){
  std::vector<UniformData> uniformData;
  uniformData.push_back(UniformData {
    .name = "framebufferTexture",
    .value = Sampler2D {
      .textureUnitId = 0,
    },
  });
  uniformData.push_back(UniformData {
    .name = "depthTexture",
    .value = Sampler2D {
      .textureUnitId = 1,
    },
  });
  setUniformData(shader, uniformData, { "useDepthTexture", "amount", "firstpass", "near", "far", "minBlurDistance", "maxBlurDistance" });
}

void initDepthShader(unsigned int shader){
  std::vector<UniformData> uniformData;
  uniformData.push_back(UniformData {
    .name = "depthTexture",
    .value = Sampler2D {
      .textureUnitId = 2,
    },
  });
  setUniformData(shader, uniformData, { "near", "far" });
}
void updateDepthShaderPerFrame(unsigned int shader, float near, float far){
  std::vector<UniformData> uniformData;
  uniformData.push_back(UniformData {
    .name = "near",
    .value = near,
  });
  uniformData.push_back(UniformData {
    .name = "far",
    .value = far,
  });
  setUniformData(shader, uniformData, { "depthTexture" });
}

void initUiShader(unsigned int shader){
  std::vector<UniformData> uniformData;
  uniformData.push_back(UniformData {
    .name = "projection",
    .value = ndiOrtho,
  });
  uniformData.push_back(UniformData {
    .name = "textureData",
    .value = Sampler2D {
      .textureUnitId = 0,
    },
  });

  setUniformData(shader, uniformData, { "time", "realtime", "model", "encodedid", "tint", "forceTint" });
}
void updateUiShaderPerFrame(unsigned int shader){
  std::vector<UniformData> uniformData;
  uniformData.push_back(UniformData {
    .name = "time",
    .value = getTotalTimeGame(),
  });
  uniformData.push_back(UniformData {
    .name = "realtime",
    .value = getTotalTime(),
  });
  setUniformData(shader, uniformData, { "forceTint", "model", "encodedid", "tint", "time", "textureData", "projection" }); 
}

void initFramebufferShader(unsigned int shader){
  std::vector<UniformData> uniformData;
  uniformData.push_back(UniformData {
    .name = "framebufferTexture",
    .value = Sampler2D {
      .textureUnitId = 0,
    }
  });
  uniformData.push_back(UniformData {
    .name = "bloomTexture",
    .value = Sampler2D {
      .textureUnitId = 1,
    }
  });
  uniformData.push_back(UniformData {
    .name = "depthTexture",
    .value = Sampler2D {
      .textureUnitId = 2,
    }
  });
  setUniformData(shader, uniformData, { 
  	"enableBloom",
  	"bloomAmount",
  	"enableFog",
  	"fogColor",
  	"near",
  	"far",
  	"mincutoff",
  	"maxcuttoff",
  	"exposure",
  	"enableGammaCorrection",
  	"enableExposure",
  });
}

void updateFramebufferShaderFrame(unsigned int shader, float near, float far){
  std::vector<UniformData> uniformData;
  uniformData.push_back(UniformData {
    .name = "bloomAmount",
    .value = state.bloomAmount,
  });
  uniformData.push_back(UniformData {
    .name = "enableBloom",
    .value = state.enableBloom,
  });
  uniformData.push_back(UniformData {
    .name = "enableExposure",
    .value = state.enableExposure,
  });
  uniformData.push_back(UniformData {
    .name = "exposure",
    .value = state.exposure,
  });
  uniformData.push_back(UniformData {
    .name = "enableFog",
    .value = state.enableFog,
  });
  uniformData.push_back(UniformData {
    .name = "enableGammaCorrection",
    .value = state.enableGammaCorrection,
  });
  uniformData.push_back(UniformData {
    .name = "near",
    .value = near,
  });
  uniformData.push_back(UniformData {
    .name = "far",
    .value = far,
  });
  uniformData.push_back(UniformData {
    .name = "fogColor",
    .value = state.fogColor,
  });
  uniformData.push_back(UniformData {
    .name = "mincutoff",
    .value = state.fogMinCutoff,
  });
  uniformData.push_back(UniformData {
    .name = "maxcuttoff",
    .value = state.fogMaxCutoff,
  });

  setUniformData(shader, uniformData, { "framebufferTexture", "bloomTexture", "depthTexture" });
}

