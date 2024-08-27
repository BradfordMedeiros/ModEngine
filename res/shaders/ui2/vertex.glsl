#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 projection;

uniform float time;

out vec2 TexCoord;

void main(){
  vec4 finalPosition = (projection *  model * vec4(aPos.xy, 0.0, 1.0));
  gl_Position =  finalPosition + vec4(0,  0.01 * cos(10 * time), 0, 0);
  TexCoord = aTexCoords;
} 
