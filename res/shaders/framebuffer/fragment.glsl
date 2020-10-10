#version 330 core 
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;
uniform sampler2D bloomTexture;

uniform bool enableBloom;
uniform float bloomAmount;

void main(){
  if (enableBloom){
    FragColor = texture(framebufferTexture, TexCoords) + (bloomAmount * texture(bloomTexture, TexCoords));
  }else{
    FragColor = texture(framebufferTexture, TexCoords);
  }
}
