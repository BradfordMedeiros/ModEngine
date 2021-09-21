#version 330 core 
out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;
uniform sampler2D bloomTexture;
uniform sampler2D depthTexture;

uniform bool enableBloom;
uniform float bloomAmount;
uniform bool enableFog;
//uniform vec3 fogColor;
//vec3 fogColor(5, 1, 1);
//uniform float near;
float near = 0.1; 
//uniform float far;
float far  = 10.0; 
float mincutoff = 0.8;
float maxcuttoff = 0.99999;

void calculateFogEffect(in vec3 fogColor, in float depthAmount, out vec4 fogAmount){
  if (depthAmount < mincutoff || depthAmount > maxcuttoff){
    fogAmount = vec4(0, 0, 0, 0);
  }else{
    vec4 fogColor = vec4(0.4, 0.4, 0.4, -1);
    float fromBaseLow = mincutoff;
    float fromBaseHigh = maxcuttoff;
    float toBaseLow = 0;
    float toBaseHigh = 1;
    float newValue =  ((depthAmount - fromBaseLow) * ((toBaseHigh - toBaseLow) / (fromBaseHigh - fromBaseLow))) + toBaseLow;
    fogAmount = mix(vec4(0, 0, 0, 0), fogColor, newValue);
  }
}

void main(){
  float depth  = texture(depthTexture, TexCoords).r;
  float z = depth * 2.0 - 1.0; 
  float depthAmount  = ((2.0 * near * far) / (far + near - z * (far - near))) / far;  // fraction of near/far
  
  vec4 fogEffect = vec4(0, 0, 0, 0);
  // in the 0.2 remaining, it should fall off 100%
  calculateFogEffect(vec3(0, 0, 1), depthAmount, fogEffect);

  if (enableBloom){
    FragColor = fogEffect + (texture(framebufferTexture, TexCoords) + (bloomAmount * texture(bloomTexture, TexCoords)));
  }else{
    FragColor = fogEffect + texture(framebufferTexture, TexCoords);
  }
  
}
