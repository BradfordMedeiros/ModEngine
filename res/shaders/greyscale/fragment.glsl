#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;

void main(){
  vec4 baseColor = texture(framebufferTexture, TexCoords);
  float average = (baseColor.r + baseColor.g + baseColor.b) / 3;
  FragColor = vec4(average, average, average, baseColor.a);
}
