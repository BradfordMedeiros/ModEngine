#include "./shaderstate.h"


float getTotalTimeGame();
float getTotalTime();
extern engineState state;
extern glm::mat4 ndiOrtho;


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
  });
}
void updateDefaultShaderPerFrame(unsigned int shader){
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

  shaderSetUniformInt(shader, "voxelcellwidth", getLightingCellWidth());

 // uniformData.push_back(UniformData {
 //   .name = "voxelcellwidth",
 //   .value = 8,
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
    
    "cameraPosition", "projview", "numlights", "enableLighting"
  });



}


void initSelectionShader(unsigned int shader){
  initDefaultShader(shader);
}
void updateSelectionShaderPerFrame(unsigned int shader){
  updateDefaultShaderPerFrame(shader);
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

