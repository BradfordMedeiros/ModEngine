#version 330 core 
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;
uniform sampler2D bloomTexture;

uniform bool enableBloom;

void main(){
  if (enableBloom){
    FragColor = texture(framebufferTexture, TexCoords) + texture(bloomTexture, TexCoords);
  }else{
    FragColor = texture(framebufferTexture, TexCoords);
  }
}
