#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;
uniform ivec2 screenresolution;
uniform float uvamount;

void main(){
  //float uvx = gl_FragCoord.x / screenresolution.x;
  float uvy = gl_FragCoord.y / screenresolution.y;
  if (uvy < uvamount|| uvy > (1- uvamount)){
    discard;
  }

  FragColor = texture(framebufferTexture, TexCoords);
}
