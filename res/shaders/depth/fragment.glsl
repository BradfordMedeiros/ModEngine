#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D maintexture;

float near = 0.1; 
float far  = 1000.0; 

void main(){
  float depth  = texture(maintexture, TexCoords).r;
  float z = depth * 2.0 - 1.0; 
  float depthColor  = ((2.0 * near * far) / (far + near - z * (far - near))) / far;

   FragColor = vec4(vec3(depthColor), 1);
}

