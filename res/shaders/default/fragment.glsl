#version 430 

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in mat3 TangentToWorld;
in vec4 sshadowCoord;
in vec3 ambientVoxelColor;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

uniform sampler2D maintexture;
uniform sampler2D emissionTexture;
uniform sampler2D opacityTexture;
uniform sampler2D lightDepthTexture;
uniform samplerCube cubemapTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D normalTexture;

uniform vec4 tint;
uniform vec3 cameraPosition;

uniform bool enableLighting;
uniform bool enableAttenutation;
uniform bool enableShadows;
uniform bool enableDiffuse;
uniform bool enableSpecular;
uniform bool enablePBR;
uniform bool hasDiffuseTexture;
uniform bool hasEmissionTexture;
uniform bool hasOpacityTexture;
uniform bool hasCubemapTexture;
uniform bool hasRoughnessTexture;
uniform bool hasNormalTexture;
uniform float shadowIntensity;

uniform vec2 textureOffset;
uniform vec2 textureTiling;
uniform vec2 textureSize;

#define MAX_LIGHTS 32
uniform int numlights;
uniform vec3 lights[MAX_LIGHTS];
uniform vec3 lightscolor[MAX_LIGHTS];
uniform vec3 lightsdir[MAX_LIGHTS];
uniform vec3 lightsatten[MAX_LIGHTS];
uniform float lightsmaxangle[MAX_LIGHTS];
uniform float lightsangledelta[MAX_LIGHTS];
uniform bool lightsisdir[MAX_LIGHTS];

uniform vec3 ambientAmount;
uniform vec3 emissionAmount;
uniform float discardTexAmount;
uniform float bloomThreshold;


// numCellsDim ^ 3 = voxelLightSize

int numCellsDim = 8;
uniform vec3 voxellights[512];

uniform int voxelcellwidth;
uniform bool enableVoxelLighting;

float convertBase(float value, float fromBaseLow, float fromBaseHigh, float toBaseLow, float toBaseHigh){
  return ((value - fromBaseLow) * ((toBaseHigh - toBaseLow) / (fromBaseHigh - fromBaseLow))) + toBaseLow;
}
int xyzToIndex(int x, int y, int z){
  return x + (numCellsDim * y) + (numCellsDim * numCellsDim * z);
}
vec3 lookupAmbientLight(){
  if (!enableVoxelLighting){
    return ambientAmount;
  }

  bool outOfRange = false;

  float newValueXFloat = convertBase(FragPos.x, voxelcellwidth * numCellsDim * -0.5, voxelcellwidth * numCellsDim * 0.5, 0, numCellsDim);
  int newValueX = int(newValueXFloat);
  if (newValueXFloat >= numCellsDim || newValueXFloat < 0){
    outOfRange = true;
  }

  float newValueYFloat = convertBase(FragPos.y, voxelcellwidth * numCellsDim * -0.5, voxelcellwidth * numCellsDim * 0.5, 0, numCellsDim);
  int newValueY = int(newValueYFloat);
  if (newValueYFloat >= numCellsDim || newValueYFloat < 0){
    outOfRange = true;
  }

  float newValueZFloat = convertBase(FragPos.z, voxelcellwidth * numCellsDim * -0.5, voxelcellwidth * numCellsDim * 0.5, 0, numCellsDim);
  int newValueZ = int(newValueZFloat);
  if (newValueZFloat >= numCellsDim || newValueZFloat < 0){
    outOfRange = true; 
  }

  if (outOfRange){  // maybe i should clamp this instead? 
    return vec3(0, 0, 0);
  }
  int finalIndex2 = xyzToIndex(newValueX, newValueY, newValueZ);
  return voxellights[finalIndex2];
}

int getNumLights(){
  return min(numlights, MAX_LIGHTS);
}

float calcAttenutation(int lightNumber){
  vec3 lightPos = lights[lightNumber];
  float distanceToLight = length(lightPos - FragPos);
  vec3 attenuationTerms = lightsatten[lightNumber];
  float constant = attenuationTerms.x;
  float linear = attenuationTerms.y;
  float quadratic = attenuationTerms.z;
  float attenuation = enableAttenutation ? (1.0 / (constant + (linear * distanceToLight) + (quadratic * (distanceToLight * distanceToLight)))) : 1;
  return attenuation;
}


#include "./res/shaders/default/algorithms/phong.glsl"
#include "./res/shaders/default/algorithms/cooktorrence.glsl"


void main(){
    if (hasCubemapTexture){
      FragColor = tint * texture(cubemapTexture, FragPos);
      return;
    }

    vec3 shadowCoord = sshadowCoord.xyz * 0.5 + 0.5;
    vec2 offsetTexCoord = vec2(TexCoord.x, TexCoord.y); 
  
    vec2 adjustedTexCoord = mod(offsetTexCoord * textureTiling, 1) * textureSize + textureOffset;

    vec4 diffuseColor = hasDiffuseTexture ? texture(maintexture, adjustedTexCoord) : vec4(1, 1, 1, 1);
    float closestDepth = texture(lightDepthTexture, shadowCoord.xy).r;

    vec4 emissionColor = texture(emissionTexture, adjustedTexCoord);
    vec4 opacityColor = texture(opacityTexture, adjustedTexCoord);

    bool discardTexture = hasOpacityTexture && opacityColor.r < discardTexAmount;   
    vec4 texColor;

    vec3 finalEmission = vec3(0, 0, 0);
    if (discardTexture){
        discard;
    }else{
        texColor = diffuseColor;
        finalEmission = (hasEmissionTexture ? vec3(emissionAmount.r * emissionColor.r, emissionAmount.g * emissionColor.g, emissionAmount.b * emissionColor.b) : vec3(0, 0, 0));
    }
    if (texColor.a < 0.1){
      discard;
    }

    vec3 normal = normalize(Normal);
    if (hasNormalTexture){
      vec3 normalTexColor = texture(normalTexture, adjustedTexCoord).rgb ;
      //normal = normalize(TangentToWorld * vec3(normalTexColor.r * 2 - 1, normalTexColor.g * 2 - 1, normalTexColor.b * 2 - 1));
      normal = normalize(transpose(TangentToWorld) * vec3(normalTexColor.r * 2 - 1, normalTexColor.g * 2 - 1, normalTexColor.b * 2 - 1));
    }

    /*if (hasNormalTexture){
      vec3 normalTexColor = texture(normalTexture, adjustedTexCoord).rgb ;
      normal = normalize(vec3(normalTexColor.r * 2 - 1, normalTexColor.g * 2 - 1, normalTexColor.b * 2 - 1));
    }*/
//    texColor = texture(normalTexture, adjustedTexCoord).rgba;
    
 
    vec4 color  = enablePBR ? calculateCookTorrence(normal, texColor.rgb, 0.2, 0.5) : vec4(calculatePhongLight(normal), 1.0) * texColor;

    bool inShadow = (shadowCoord.z - 0.00001) > closestDepth;
    float shadowDelta = (enableShadows && inShadow) ? shadowIntensity : 1.0;

    if (enableLighting){
      FragColor = (tint *  vec4(color.xyz * shadowDelta, color.w) + vec4(finalEmission.rgb, 0));
    }else{
      FragColor = tint * texColor;
    }

    // TODO -> what would be a better thesholding function? 
    float brightness = FragColor.r + FragColor.g + FragColor.b;
    if(brightness > bloomThreshold){
      BloomColor = vec4(FragColor.rgb, 1.0);
    }else{
      BloomColor = vec4(0.0, 0.0, 0.0, 1.0);    
    }       
}
