#include "./shaderstate.h"


float getTotalTimeGame();
float getTotalTime();
extern engineState state;

std::vector<UniformData> getDefaultShaderUniforms(std::optional<glm::mat4> projview, glm::vec3 cameraPosition, int numLights, bool enableLighting){
  std::vector<UniformData> uniformData;
  uniformData.push_back(UniformData {
    .name = "maintexture",
    .value = Sampler2D {
      .textureUnitId = 0,
    },
  });
  /* obviously texture id an maintexture shouldn't be same here */
  uniformData.push_back(UniformData { 
    .name = "textureid",
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
  if (projview.has_value()){
    uniformData.push_back(UniformData {
      .name = "projview",
      .value = projview.value(),
    });
  }
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
    .name = "enableLighting",
    .value = enableLighting,
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
    .name = "cameraPosition",
    .value = cameraPosition,
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
    .name = "numlights",
    .value = numLights,
  });
  uniformData.push_back(UniformData {
    .name = "time",
    .value = getTotalTimeGame(),
  });
  uniformData.push_back(UniformData {
    .name = "realtime",
    .value = getTotalTime(),
  });
  return uniformData;  
}

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


  /*
  if (projview.has_value()){
    uniformData.push_back(UniformData {
      .name = "projview",
      .value = projview.value(),
    });
  }
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
    .name = "enableLighting",
    .value = enableLighting,
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
    .name = "",
    .value = cameraPosition,
  });
  uniformData.push_back(UniformData {
    .name = "shadowIntensity",
    .value = state.shadowIntensity,
  });
  uniformData.push_back(UniformData {
    .name = ,
    .value = state.enableShadows,
  });
  uniformData.push_back(UniformData {
    .name = "numlights",
    .value = numLights,
  });
  uniformData.push_back(UniformData {
    .name = "time",
    .value = getTotalTimeGame(),
  });
  uniformData.push_back(UniformData {
    .name = "realtime",
    .value = getTotalTime(),
  });*/
  setUniformData(shader, uniformData, { 
  	"ambientAmount", "bloomThreshold",
	 	"bones[0]", "hasBones", "cameraPosition", "discardTexAmount", "emissionAmount", "enableAttenutation", "enableDiffuse",
	 	"enableLighting", "enablePBR", "enableShadows", "enableSpecular",
 		"model", "numlights", "shadowIntensity", "useBoneTransform",
    "hasCubemapTexture", "hasDiffuseTexture", "hasEmissionTexture", "hasNormalTexture", "hasOpacityTexture",
    "lights[0]", "lightsangledelta[0]", "lightsatten[0]", "lightscolor[0]", "lightsdir[0]", "lightsisdir[0]", "lightsmaxangle[0]", "voxelindexs2[0]", "voxelcellwidth",
    "lightsprojview", "textureOffset", "textureSize", "textureTiling", "tint", "projview", "realtime", "time"
  });
}
void updateDefaultShaderPerFrame(unsigned int shader){

}


void initSelectionShader(unsigned int shader){

}
void initBlurShader(unsigned int shader){

}
void initDepthShader(unsigned int shader){

}
void initUiShader(unsigned int shader){

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