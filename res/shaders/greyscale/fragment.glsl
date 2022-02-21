#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;

void main(){
  vec3 baseColor = texture(framebufferTexture, TexCoords).rgb;
  float average = (baseColor.r + baseColor.g + baseColor.b) / 3;
  FragColor = vec4(average.r, average.g, average.b, 1.0);
}
