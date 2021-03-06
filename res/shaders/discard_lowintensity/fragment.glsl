#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D maintexture;
uniform sampler2D emissionTexture;
uniform sampler2D opacityTexture;

uniform vec3 tint;
uniform vec3 cameraPosition;

uniform bool enableDiffuse;
uniform bool enableSpecular;
uniform bool hasEmissionTexture;
uniform bool hasOpacityTexture;
uniform vec2 textureOffset;

#define MAX_LIGHTS 32
uniform int numlights;
uniform vec3 lights[MAX_LIGHTS];
uniform vec3 lightscolor[MAX_LIGHTS];
uniform vec3 lightsdir[MAX_LIGHTS];

const float constant = 0.1;
const float linear = 0.1;
const float quadratic = 0.0;

const float emissionAmount = 1;
uniform float discardTexAmount;

void main(){
  if (tint.r < 0.1){
    FragColor = vec4(tint.r, tint.g, tint.b, 1.0);
  }else{
    vec2 adjustedTexCoord = TexCoord + textureOffset;
    vec4 diffuseColor = texture(maintexture, vec2(adjustedTexCoord.x, -adjustedTexCoord.y));
    vec4 emissionColor = texture(emissionTexture, vec2(adjustedTexCoord.x, -adjustedTexCoord.y));
    vec4 opacityColor = texture(opacityTexture, vec2(adjustedTexCoord.x, -adjustedTexCoord.y));

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
        vec3 lightDir = normalize(lightPos - FragPos);
        vec3 normal = normalize(Normal);

        float angle = dot(lightDir, normalize(-lightsdir[i]));
        if (angle < 0.1){
            continue;
        }

        vec3 diffuse = max(dot(normal, lightDir), 0.0) * lightscolor[i];

        vec3 viewDir = normalize(cameraPosition - FragPos);
        vec3 reflectDir = reflect(-lightDir, normal);  
        vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), 32) * vec3(1.0, 1.0, 1.0);  
 
        float distanceToLight = length(lightPos - FragPos);
        float attenuation = 1.0 / (constant + linear * distanceToLight + quadratic * (distanceToLight * distanceToLight));  

        totalDiffuse = totalDiffuse + (attenuation * diffuse * lightscolor[i]);
        totalSpecular = totalSpecular + (attenuation * specular * lightscolor[i]);
    }

    vec3 diffuseValue = enableDiffuse ? totalDiffuse : vec3(0, 0, 0);
    vec3 specularValue = enableSpecular ? totalSpecular : vec3(0, 0, 0);
    vec4 color = vec4(ambient + diffuseValue + specularValue, 1.0) * texColor;
    float sum = color.x + color.y + color.z;
    if (color.x > 0.1){
        discard;
    }

    vec4 colorr = color * vec4(6, 6, 8, 1);
    FragColor = vec4(colorr.xyz, 0.2);
  }
}
