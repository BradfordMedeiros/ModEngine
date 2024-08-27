#version 330 core 
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;
uniform sampler2D bloomTexture;
uniform sampler2D depthTexture;

uniform bool enableBloom;
uniform float bloomAmount;
uniform bool enableFog;
uniform vec4 fogColor;
uniform float near;
uniform float far;
uniform float mincutoff;
uniform float maxcuttoff;
uniform float exposure;

bool enableGammaCorrection = false;

uniform vec4 encodedid;
uniform vec4 encodedid2;

void calculateFogEffect(in float depthAmount, out vec4 fogAmount){
  if (depthAmount < mincutoff || depthAmount > maxcuttoff){
    fogAmount = vec4(0, 0, 0, 0);
  }else{
    float fromBaseLow = mincutoff;
    float fromBaseHigh = maxcuttoff;
    float toBaseLow = 0;
    float toBaseHigh = 1;
    float newValue =  ((depthAmount - fromBaseLow) * ((toBaseHigh - toBaseLow) / (fromBaseHigh - fromBaseLow))) + toBaseLow;
    fogAmount = mix(vec4(0, 0, 0, 0), fogColor, newValue);
  }
}

void main(){
  FragColor = texture(framebufferTexture, TexCoords);
}
