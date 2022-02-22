#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;

uniform int redtint;
uniform float mult;
uniform bool enable;

uniform vec3 colortint;

void main(){
  vec3 baseColor = texture(framebufferTexture, TexCoords).rgb;
  if (enable){
    baseColor = baseColor * colortint;
  }

  FragColor = vec4(baseColor.rgb, 1) ;
}
