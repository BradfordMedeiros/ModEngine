#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D texture1;
uniform sampler2D texture2;

void main(){
  vec4 baseColor1 = texture(texture1, TexCoords);
  vec4 baseColor2 = texture(texture2, TexCoords);
  vec4 combined = baseColor1 + baseColor2;
  FragColor = combined;
}
