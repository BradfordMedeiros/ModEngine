#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;

void main(){
  FragColor = texture(framebufferTexture, TexCoords).rgba;
}
