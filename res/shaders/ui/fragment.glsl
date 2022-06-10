#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D textureData;
uniform bool forceTint;
uniform vec4 tint;

void main(){
  if (forceTint){
    FragColor = tint;
    return;
  }
  vec4 texColor = texture(textureData, vec2(TexCoord.x, -1 * TexCoord.y));
  if(texColor.w < 0.1){
  	discard;
  }
  FragColor = texColor * tint;
}
