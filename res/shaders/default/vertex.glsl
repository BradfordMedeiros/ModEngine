#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
//uniform mat4 worldtolight;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragLight;

void main(){
  vec4 modelPosition = model * vec4(aPos.xyz, 1.0);
  gl_Position = projection * view * modelPosition;
  TexCoord = aTexCoords;
  Normal = mat3(transpose(inverse(model))) * aNormal;  
  FragPos = modelPosition.xyz;
  //FragLight = worldtolight * vec4(FragPos.xyz, 1.0);
} 
