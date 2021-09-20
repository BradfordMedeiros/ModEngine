#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 sshadowCoord;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

uniform sampler2D maintexture;
uniform sampler2D emissionTexture;
uniform sampler2D opacityTexture;
uniform sampler2D lightDepthTexture;
uniform samplerCube cubemapTexture;

uniform vec3 tint;
uniform vec3 cameraPosition;

uniform bool enableDiffuse;
uniform bool enableSpecular;
uniform bool hasDiffuseTexture;
uniform bool hasEmissionTexture;
uniform bool hasOpacityTexture;
uniform bool hasCubemapTexture;

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
uniform bool lightsisdir[MAX_LIGHTS];

//uniform mat4 lightsprojview[MAX_LIGHTS];

uniform float emissionAmount;
uniform float discardTexAmount;
uniform float time;

void main(){
   // vec3 shadowCoord = sshadowCoord.xyz / sshadowCoord.w;
    // real close => 0 , real far => 1
//    shadowCoord = shadowCoord * 0.5 + 0.5;
    if (hasCubemapTexture){
      FragColor = texture(cubemapTexture, FragPos);
      return;
    }

    vec3 shadowCoord = sshadowCoord.xyz * 0.5 + 0.5;
    vec2 offsetTexCoord = vec2(TexCoord.x, -1 * TexCoord.y);  // -1 because the images we feed in are flipped b/c the image encoding has them flipped.  Could reverse them there, but 
  
    vec2 adjustedTexCoord = mod(offsetTexCoord * textureTiling, 1) * textureSize + textureOffset;

    vec4 diffuseColor = hasDiffuseTexture ? texture(maintexture, adjustedTexCoord) : vec4(0, 0, 1, 1);
    float closestDepth = texture(lightDepthTexture, shadowCoord.xy).r;

    vec4 emissionColor = texture(emissionTexture, adjustedTexCoord);
    vec4 opacityColor = texture(opacityTexture, adjustedTexCoord);

    bool discardTexture = hasOpacityTexture && opacityColor.r < discardTexAmount;     // This is being derived from emission map but going to use different map (in progress)

    vec4 texColor;
    if (discardTexture){
        discard;
    }else{
        texColor = diffuseColor + emissionAmount * (hasEmissionTexture ? emissionColor : vec4(0, 0, 0, 0));
    }

    if (texColor.a < 0.1){
      discard;
    }

    vec3 ambient = vec3(0.1, 0.1, 0.1);     
    vec3 totalSpecular = vec3(0.2, 0.2, 0.2);
    vec3 totalDiffuse  = vec3(0.2, 0.2, 0.2);
    
    for (int i = 0; i < min(numlights, MAX_LIGHTS); i++){
        vec3 lightPos = lights[i];
        vec3 lightDir = lightsisdir[i] ?  lightsdir[i] : normalize(lightPos - FragPos);
        vec3 normal = normalize(Normal);

        float angle = dot(lightDir, normalize(-lightsdir[i]));
        if (angle < lightsmaxangle[i]){
            continue;
        }

        vec3 diffuse = max(dot(normal, lightDir), 0.0) * lightscolor[i];

        vec3 viewDir = normalize(cameraPosition - FragPos);
        vec3 reflectDir = reflect(-lightDir, normal);  
        vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), 32) * vec3(1.0, 1.0, 1.0);  
 
        float distanceToLight = length(lightPos - FragPos);

        vec3 attenuationTerms = lightsatten[i];
        float constant = attenuationTerms.x;
        float linear = attenuationTerms.y;
        float quadratic = attenuationTerms.z;
        float attenuation = 1.0 / (constant + (linear * distanceToLight) + (quadratic * (distanceToLight * distanceToLight)));  

        //totalDiffuse = totalDiffuse + (attenuation * diffuse * lightscolor[i]);
        //totalSpecular = totalSpecular + (attenuation * specular * lightscolor[i]);

        totalDiffuse = totalDiffuse + (diffuse * lightscolor[i]);
        totalSpecular = totalSpecular + (specular * lightscolor[i]);
    }

    vec3 diffuseValue = enableDiffuse ? totalDiffuse : vec3(0, 0, 0);
    vec3 specularValue = enableSpecular ? totalSpecular : vec3(0, 0, 0);
    vec4 color = vec4(ambient + diffuseValue + specularValue, 1.0) * texColor;

    bool inShadow = (shadowCoord.z - 0.00001) > closestDepth;
    float shadowDelta = (false && inShadow) ? 0.2 : 1.0;

    FragColor = vec4(tint * color.xyz * shadowDelta, color.w);

    // TODO -> what would be a better thesholding function? 
    float brightness = FragColor.r + FragColor.g + FragColor.b;
    if(brightness > 2.7){
      BloomColor = vec4(FragColor.rgb, 1.0);
    }else{
      BloomColor = vec4(0.0, 0.0, 0.0, 0.0);    
    }

       
}

