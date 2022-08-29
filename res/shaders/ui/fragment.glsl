#version 330 core

in vec2 TexCoord;
uniform sampler2D textureData;
uniform bool forceTint;
uniform vec4 tint;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragColor2;

void main(){
  if (forceTint){
    FragColor = tint;
    return;
  }
  vec4 texColor = texture(textureData, vec2(TexCoord.x, TexCoord.y));
  if(texColor.w < 0.1){
  	discard;
  }
  FragColor = texColor * tint;
  FragColor2 = vec4(0, 0, 1, 1);
}
