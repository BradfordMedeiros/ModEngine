#version 330 core

in vec2 TexCoord;
uniform sampler2D textureData;
uniform bool forceTint;
uniform vec4 tint;

uniform vec4 encodedid;
uniform vec4 encodedid2;

uniform float time;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 FragColor2;

float random(vec2 st) {
    return fract(sin(dot(st.xy,
                        vec2(12.9898,78.233))) * 
                 43758.5453);
}

void main(){

  float noise = random(TexCoord * 100.0 + cos(0.01 * time) * 10.0); // Adjust factors for different effects

  if (forceTint){
    FragColor = tint * vec4(1, 0, 0, 1);
    return;
  }
  vec4 texColor = texture(textureData, vec2(TexCoord.x, TexCoord.y));
  if(texColor.w < 0.1){
    FragColor = vec4(0, 0, 0, 0);
    //discard;
  }else{
    FragColor = texColor * tint * vec4(1, 1, 1, 1) * vec4(noise, noise, noise, 1);
  }
  FragColor2 = encodedid2;
}
