#version 330 core 
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D depthTexture;
uniform float near;
uniform float far;

void main(){
  float depth  = texture(depthTexture, TexCoords).r;
  float z = depth * 2.0 - 1.0; 
  float depthAmount  = ((2.0 * near * far) / (far + near - z * (far - near))) / far;  // fraction of near/far
  FragColor = vec4(depthAmount, depthAmount, depthAmount, 1);

}
