#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 projview;

out vec2 TexCoord;

void main(){
  gl_Position = projview * model * vec4(aPos.xyz, 1.0);
  TexCoord = aTexCoords;
} 
