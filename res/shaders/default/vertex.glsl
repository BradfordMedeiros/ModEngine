#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec2 aBoneIndex[10];
layout (location = 4) in vec2 aBoneWeights[10];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 bones[10];
uniform bool hasBones;

//uniform mat4 worldtolight;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragLight;

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(){
  vec4 modelPosition = model * vec4(aPos.xyz, 1.0);

  if (hasBones){
    gl_Position = projection * view * modelPosition;
    TexCoord = aTexCoords;
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    FragPos = modelPosition.xyz;
  }else{
    gl_Position = projection * view * modelPosition;
    TexCoord = aTexCoords;
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    FragPos = modelPosition.xyz;
  }

  //FragLight = worldtolight * vec4(FragPos.xyz, 1.0);
} 
