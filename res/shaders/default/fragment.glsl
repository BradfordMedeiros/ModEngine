#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// temporary
in vec4 glFragPos;
flat in vec4 overcolor;
//

out vec4 FragColor;

uniform sampler2D maintexture;
uniform sampler2D emissionTexture;

uniform vec3 tint;
uniform vec3 cameraPosition;

uniform bool enableDiffuse;
uniform bool enableSpecular;
uniform bool hasEmissionTexture;

#define MAX_LIGHTS 32
uniform int numlights;
uniform vec3 lights[MAX_LIGHTS];
uniform bool hasBones;
uniform mat4 bones[100];

const float constant = 0.4;
const float linear = 0.2;
const float quadratic = 0.0;

const float emissionAmount = 1;

void main(){
  if (tint.r < 0.1){
    FragColor = vec4(tint.r, tint.g, tint.b, 1.0);
  }else{
    vec4 diffuseColor = texture(maintexture, vec2(TexCoord.x, -TexCoord.y));
    vec4 emissionColor = texture(emissionTexture, vec2(TexCoord.x, -TexCoord.y));
    vec4 texColor = diffuseColor + emissionAmount * (hasEmissionTexture ? emissionColor : vec4(0, 0, 0, 0));

    if (texColor.a < 0.1){
      discard;
    }

    vec3 ambient = vec3(0.6, 0.6, 0.6);     
    vec3 totalSpecular = vec3(0, 0, 0);
    vec3 totalDiffuse  = vec3(0, 0, 0);
    
    for (int i = 0; i < min(numlights, MAX_LIGHTS); i++){
        vec3 lightPos = lights[i];
        vec3 lightDir = normalize(lightPos - FragPos);
        vec3 normal = normalize(Normal);

        vec3 diffuse = max(dot(normal, lightDir), 0.0) * vec3(1.0, 1.0, 1.0);

        vec3 viewDir = normalize(cameraPosition - FragPos);
        vec3 reflectDir = reflect(-lightDir, normal);  
        vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), 32) * vec3(1.0, 1.0, 1.0);  
 
        float distanceToLight = length(lightPos - FragPos);
        float attenuation = 1.0 / (constant + linear * distanceToLight + quadratic * (distanceToLight * distanceToLight));  

        totalDiffuse = totalDiffuse + (attenuation * diffuse);
        totalSpecular = totalSpecular + (attenuation * specular);
    }

    vec3 diffuseValue = enableDiffuse ? totalDiffuse : vec3(0, 0, 0);
    vec3 specularValue = enableSpecular ? totalSpecular : vec3(0, 0, 0);
    vec4 color = vec4(ambient + diffuseValue + specularValue, 1.0) * texColor;
    FragColor = color * vec4(tint.x, tint.y, tint.z, 1.0);
  }
}
