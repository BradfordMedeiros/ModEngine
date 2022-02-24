#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;

uniform vec3 threshold;
uniform vec3 color;

void main(){
  vec4 values = texture(framebufferTexture, TexCoords);
  if ((values.r < threshold.r) || (values.g < threshold.g) || (values.b < threshold.b)){
    discard;
  }else{
    FragColor = vec4(color.r, color.g, color.b, values.a);
  }
}
