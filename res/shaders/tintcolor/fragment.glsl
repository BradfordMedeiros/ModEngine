#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;

uniform int redtint;

void main(){
  vec4 baseColor = texture(framebufferTexture, TexCoords).rgba;
  baseColor.r = baseColor.r + redtint;
  FragColor = baseColor;
}
