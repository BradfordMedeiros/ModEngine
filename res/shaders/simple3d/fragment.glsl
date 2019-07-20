#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D textureData;

void main(){
  FragColor = vec4(0.0, 0.0, 1.0, 1.0);
}
