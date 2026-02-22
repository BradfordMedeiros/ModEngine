#version 430 

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in mat3 TangentToWorld;
in vec4 sshadowCoord;
in vec3 ambientVoxelColor;
flat in vec3 vertColor;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;
layout(location = 2) out vec4 EncodeId;
layout(location = 3) out vec4 UVCoords;


uniform sampler2D maintexture;
uniform sampler2D emissionTexture;
uniform sampler2D opacityTexture;
uniform sampler2D lightDepthTexture;
uniform samplerCube cubemapTexture;
uniform sampler2D roughnessTexture;
uniform sampler2D normalTexture;
uniform sampler2D lightTexture;

uniform vec4 tint;
uniform vec3 cameraPosition;

uniform bool enableLighting;
uniform bool enableAttenutation;
uniform bool enableShadows;
uniform bool enableDiffuse;
uniform bool enableSpecular;
uniform bool visualizeVoxelLighting;
uniform bool enablePBR;
uniform bool hasDiffuseTexture;
uniform bool hasEmissionTexture;
uniform bool hasOpacityTexture;
uniform bool hasCubemapTexture;
uniform bool hasRoughnessTexture;
uniform bool hasNormalTexture;
uniform bool sky;
uniform float shadowIntensity;

uniform vec2 textureOffset;
uniform vec2 textureTiling;
uniform vec2 textureSize;

uniform int numlights;
uniform vec3 lights[ $LIGHT_BUFFER_SIZE ];
uniform vec3 lightscolor[ $LIGHT_BUFFER_SIZE ];
uniform vec3 lightsdir[ $LIGHT_BUFFER_SIZE ];
uniform mat4 lightsdirmat[ $LIGHT_BUFFER_SIZE ];
uniform vec3 lightsatten[ $LIGHT_BUFFER_SIZE ];
uniform float lightsmaxangle[ $LIGHT_BUFFER_SIZE ];
uniform float lightsangledelta[ $LIGHT_BUFFER_SIZE ];
uniform bool lightsisdir[ $LIGHT_BUFFER_SIZE ];
uniform vec4 lightstexindex[ $LIGHT_BUFFER_SIZE ];

uniform vec3 ambientAmount;
uniform vec3 emissionAmount;
uniform float discardTexAmount;
uniform float bloomThreshold;

//uniform int textureid;
//uniform vec4 encodedid;

// numCellsDim ^ 3 = voxelLightSize
// https://stackoverflow.com/questions/20647207/glsl-replace-large-uniform-int-array-with-buffer-or-texture
// todo make lighting info here a ubo

int numCellsDim = $NUM_CELLS_DIM;
uniform int voxelcellwidth;
uniform bool enableVoxelLighting;
uniform int defaultVoxelLight;

layout(std430, binding = 0) buffer LargeBlock {
  int voxelindexs2[];  // vec4 alignment....could pack better probably then
};

float convertBase(float value, float fromBaseLow, float fromBaseHigh, float toBaseLow, float toBaseHigh){
  return ((value - fromBaseLow) * ((toBaseHigh - toBaseLow) / (fromBaseHigh - fromBaseLow))) + toBaseLow;
}
int xyzToIndex(int x, int y, int z){
  return x + (numCellsDim * y) + (numCellsDim * numCellsDim * z);
}

ivec3 calcLightIndexValues(out bool outOfRange){
  outOfRange = false;

  vec3 voxelSamplingPosition = FragPos;
  float newValueXFloat = convertBase(voxelSamplingPosition.x, voxelcellwidth * numCellsDim * -0.5, voxelcellwidth * numCellsDim * 0.5, 0, numCellsDim);
  int newValueX = int(newValueXFloat);
  if (newValueXFloat >= numCellsDim || newValueXFloat < 0){
    outOfRange = true;
  }

  float newValueYFloat = convertBase(voxelSamplingPosition.y, voxelcellwidth * numCellsDim * -0.5, voxelcellwidth * numCellsDim * 0.5, 0, numCellsDim);
  int newValueY = int(newValueYFloat);
  if (newValueYFloat >= numCellsDim || newValueYFloat < 0){
    outOfRange = true;
  }

  float newValueZFloat = convertBase(voxelSamplingPosition.z, voxelcellwidth * numCellsDim * -0.5, voxelcellwidth * numCellsDim * 0.5, 0, numCellsDim);
  int newValueZ = int(newValueZFloat);
  if (newValueZFloat >= numCellsDim || newValueZFloat < 0){
    outOfRange = true; 
  }
  if (outOfRange){  // maybe i should clamp this instead? 
    return ivec3(0, 0, 0);
  }
  return ivec3(newValueX, newValueY, newValueZ);
}

int calcLightIndex(){
  bool outOfRange = false;
  ivec3 indexs = calcLightIndexValues(outOfRange);
  if (outOfRange){  // maybe i should clamp this instead? 
    return -1;
  }
  int finalIndex2 = xyzToIndex(indexs.x, indexs.y, indexs.z);
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
  return ambientAmount + vertColor;
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

uniform float time;

vec4 calcWaveOffsetColor(vec3 position){
  vec4 value = 0 * cos(time) * vec4(0, 0.05 * cos(position.x ), 0 * cos(position.x + time), 0) + (emissionAmount.x * vec4(1, 0, 0, 0));
  return vec4(value.xyz, 1);
}


vec2 scrollWater(vec2 coords, float waterSpeed){
	float yOffset = cos(time * waterSpeed);
	return vec2(coords.x, coords.y + yOffset);
}

void main(){
    if (sky){
      discard;
    }
    //EncodeId = vec4(encodedid.x, encodedid.y, encodedid.z, encodedid.w);
    //UVCoords = vec4(TexCoord.x, TexCoord.y, textureid, 0);
    

    vec3 shadowCoord = sshadowCoord.xyz * 0.5 + 0.5;
    vec2 offsetTexCoord = scrollWater(vec2(TexCoord.x, TexCoord.y), 0.01); 
    vec2 adjustedTexCoord = mod(offsetTexCoord * textureTiling, 1) * textureSize + textureOffset;

    vec2 offsetTexCoordNormal = scrollWater(vec2(TexCoord.x, TexCoord.y), 0.03); 


    vec4 diffuseColor = hasDiffuseTexture ? texture(maintexture, adjustedTexCoord) : vec4(1, 1, 1, 1);

    vec3 normal = normalize(Normal);
    if (hasNormalTexture ){
      vec3 normalTexColor = texture(normalTexture, offsetTexCoordNormal).rgb ;
      //normal = normalize(TangentToWorld * vec3(normalTexColor.r * 2 - 1, normalTexColor.g * 2 - 1, normalTexColor.b * 2 - 1));
      normal = normalize(transpose(TangentToWorld) * vec3(normalTexColor.r * 2 - 1, normalTexColor.g * 2 - 1, normalTexColor.b * 2 - 1));
    }

    if (hasCubemapTexture){
      vec3 viewDir = normalize(FragPos - cameraPosition);
      vec3 reflectDir = reflect(viewDir, normal);
      vec4 cubemap = texture(cubemapTexture, reflectDir);
      diffuseColor.r = 1 * (0.6 * diffuseColor.r) + (0.4 * cubemap.r);
      diffuseColor.g = 1 * (0.6 * diffuseColor.g) + (0.4 * cubemap.g);
      diffuseColor.b = 1 * (0.6 * diffuseColor.b) + (0.4 * cubemap.b);

      vec3 viewDir2 = normalize(cameraPosition - FragPos);

      float fresnel = pow(1.0 - dot(viewDir2, normal), 5);
      diffuseColor.r += 0.0001 * fresnel;
      diffuseColor.g += 0.0001 * fresnel;
      diffuseColor.b += 0.0001 * fresnel;

    }

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



    /*if (hasNormalTexture){
      vec3 normalTexColor = texture(normalTexture, adjustedTexCoord).rgb ;
      normal = normalize(vec3(normalTexColor.r * 2 - 1, normalTexColor.g * 2 - 1, normalTexColor.b * 2 - 1));
    }*/
//    texColor = texture(normalTexture, adjustedTexCoord).rgba;
    

    vec3 lightPosition = vec3(0, 0, 0);
    bool hasLight = false;
    mat4 lightRot = mat4(1.f);

    if (lightstexindex[0].x == 0){
      hasLight = true;
    }
    vec4 color  = vec4(calculatePhongLight(normal, lightPosition, hasLight, visualizeVoxelLighting, lightRot), 1.0) * texColor;


    bool inShadow = (shadowCoord.z - 0.00001) > closestDepth;
    float shadowDelta = (enableShadows && inShadow) ? shadowIntensity : 1.0;


    if (enableLighting){
      FragColor = (tint * ( calcWaveOffsetColor(FragPos) + vec4(color.xyz * shadowDelta, 0)));
    }else{
      FragColor = tint * texColor * 0;
    }


    if (hasLight){
      vec3 dir = normalize(FragPos - lightPosition);  // Light-to-fragment direction
      vec4 newDir = inverse(lightRot) * vec4(dir.xyz, 1.0);
      dir = newDir.xyz;

      float azimuth = atan(dir.z, dir.x);         // [-π, π]
      float elevation = acos(clamp(dir.y, -1.0, 1.0)); // [0, π]

      // Convert to [0,1] range
      float u = (azimuth + 3.1416) / (2.0 * 3.1416);
      float v = elevation / PI;
      vec2 baseUV = vec2(u, v);  // These are your texture coordinates

      vec2 uv = baseUV + (0.1 * time) * vec2(0.1, 0.2); // tweak speeds

      vec4 lightTextureColor = texture(lightTexture, uv) / (0.2 * distance(FragPos, lightPosition)) ;
  
      FragColor = FragColor + (lightTextureColor * 1.0);      
    }



    //if (FragColor.r > 0.2 || FragColor.g > 0.2 || FragColor.b > 0.2){
    //  FragColor *= 1.5;
    //  FragColor.w = 1;
    //}

    // TODO -> what would be a better thesholding function? 
    float brightness = FragColor.r + FragColor.g + FragColor.b;
    if(brightness > bloomThreshold){
      BloomColor = vec4(FragColor.rgb, 1.0);
    }else{
      BloomColor = vec4(0.0, 0.0, 0.0, 1.0);    
    }       
}
