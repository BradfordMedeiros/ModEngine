#version 430 

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in mat3 TangentToWorld;
in vec4 sshadowCoord;
in vec3 ambientVoxelColor;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;
layout (location = 2) out vec4 EncodeId;
layout (location = 3) out vec4 UVCoords;


// per object
uniform sampler2D maintexture;
uniform sampler2D emissionTexture;
uniform sampler2D opacityTexture;  // todo remove this and make this a special shader to accomplish this instead
uniform sampler2D lightDepthTexture;
uniform samplerCube cubemapTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D normalTexture;
uniform vec4 tint;

uniform bool hasDiffuseTexture;
uniform bool hasEmissionTexture;
uniform bool hasOpacityTexture;
uniform bool hasCubemapTexture;
uniform bool hasRoughnessTexture;
uniform bool hasNormalTexture;

uniform vec2 textureOffset;
uniform vec2 textureTiling;
uniform vec2 textureSize;
uniform float discardTexAmount;  // this is rare and should just be done as a special shader if i need this

uniform vec4 encodedid;
uniform int textureid;


// per frame
uniform vec3 cameraPosition;
uniform int numlights;
uniform vec3 lights[ $LIGHT_BUFFER_SIZE ];
uniform vec3 lightscolor[ $LIGHT_BUFFER_SIZE ];
uniform vec3 lightsdir[ $LIGHT_BUFFER_SIZE ];
uniform vec3 lightsatten[ $LIGHT_BUFFER_SIZE ];
uniform float lightsmaxangle[ $LIGHT_BUFFER_SIZE ];
uniform float lightsangledelta[ $LIGHT_BUFFER_SIZE ];
uniform bool lightsisdir[ $LIGHT_BUFFER_SIZE ];



// globalish settings
uniform bool enableLighting;
uniform bool enableAttenutation;
uniform bool enableShadows;
uniform bool enableDiffuse;
uniform bool enableSpecular;
uniform bool enablePBR;
uniform float shadowIntensity;

uniform vec3 ambientAmount;
uniform vec3 emissionAmount;
uniform float bloomThreshold;


// numCellsDim ^ 3 = voxelLightSize
// https://stackoverflow.com/questions/20647207/glsl-replace-large-uniform-int-array-with-buffer-or-texture
// todo make lighting info here a ubo

int numCellsDim = $NUM_CELLS_DIM;
uniform int voxelcellwidth;
uniform bool enableVoxelLighting;

layout(std140, binding = 0) uniform LargeBlock {
  int voxelindexs2[ $VOXEL_ARR_SIZE ];  // vec4 alignment....could pack better probably then
};

float convertBase(float value, float fromBaseLow, float fromBaseHigh, float toBaseLow, float toBaseHigh){
  return ((value - fromBaseLow) * ((toBaseHigh - toBaseLow) / (fromBaseHigh - fromBaseLow))) + toBaseLow;
}
int xyzToIndex(int x, int y, int z){
  return x + (numCellsDim * y) + (numCellsDim * numCellsDim * z);
}

int calcLightIndex(){
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
    return -1;
  }
  int finalIndex2 = xyzToIndex(newValueX, newValueY, newValueZ);
  int lightIndex = voxelindexs2[finalIndex2];
  return lightIndex;
}

void getLights(out int lights[ $LIGHTS_PER_VOXEL ]){
  lights[0] = calcLightIndex();
  for (int i = 1; i < $LIGHTS_PER_VOXEL ; i++){
    lights[i] = -1;  // -1 => no light
  }
}

vec3 lookupAmbientLight(){
  return ambientAmount;
}

int getNumLights(){
  return min(numlights, $LIGHT_BUFFER_SIZE );
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
    EncodeId = vec4(encodedid.x, encodedid.y, encodedid.z, encodedid.w);
    UVCoords = vec4(TexCoord.x, TexCoord.y, textureid, 0);

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
