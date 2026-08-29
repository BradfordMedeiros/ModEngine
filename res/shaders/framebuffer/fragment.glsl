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
uniform bool enableGammaCorrection;
uniform bool enableExposure;
uniform bool enableDepthVisualization;
uniform vec3 colorGrade;
uniform float saturation;
uniform float contrast;
uniform float realtime;
uniform vec2 chromatic;
uniform ivec2 resolution;

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
  // CRT WARP EFFECT vec2 p = TexCoords * 2.0 - 1.0;
  // CRT WARP EFFECT float curvature = 0.02;
  // CRT WARP EFFECT p *= 1.0 + curvature * dot(p, p);
  // CRT WARP EFFECT vec2 EffectTexCoords = p * 0.5 + 0.5;

  vec2 EffectTexCoords = TexCoords;
  
  if (enableDepthVisualization){
    float depth  = texture(depthTexture, EffectTexCoords).r;
    float z = depth * 2.0 - 1.0; 
    float depthAmount  = ((2.0 * near * far) / (far + near - z * (far - near))) / far;  // fraction of near/far
    FragColor = vec4(depthAmount, depthAmount, depthAmount, 1);
    return;
  }


  float depth  = texture(depthTexture, EffectTexCoords).r;
  float z = depth * 2.0 - 1.0; 
  float depthAmount  = ((2.0 * near * far) / (far + near - z * (far - near))) / far;  // fraction of near/far
  
  vec4 fogEffect = vec4(0, 0, 0, 0);
  // in the 0.2 remaining, it should fall off 100%
  if (enableFog){
    calculateFogEffect(depthAmount, fogEffect);
  }

  if (enableBloom){
    FragColor = fogEffect + (texture(framebufferTexture, EffectTexCoords) + (bloomAmount * texture(bloomTexture, TexCoords)));
  }else{
    FragColor = fogEffect + texture(framebufferTexture, EffectTexCoords);
  }

  // should restore after testing a bit more
  //vec3 color = FragColor.rgb;

  //   // chromatic abberation 

  //vec2 offset = vec2(cos(realtime) * 0.005, 0.002);

  vec2 offset = vec2(chromatic.x, chromatic.y);

  vec3 color = vec3(
    texture(framebufferTexture, EffectTexCoords + offset).r,
    texture(framebufferTexture, EffectTexCoords).g,
    texture(framebufferTexture, EffectTexCoords - offset).b
  );

  if (enableExposure){
    color = vec3(1.0) - exp(-FragColor.rgb * exposure);
  }
  if (enableGammaCorrection){
    color = pow(color, vec3(1.0 / 2.2));  
  }

  // This is saturation / desaturation 
  float luminance = dot(color, vec3(0.299, 0.587, 0.114)); // Rec. 601 luma coefficients
  color = mix(color, vec3(luminance), saturation);

  // Contrast
  color = (color - 0.5) * contrast + 0.5;

  // CRT overlay 
  // SCANLINE EFFECT float scanlineCount = 40.0;
  // SCANLINE EFFECT float flicker = 0.97 + 0.03 * sin(realtime * 8.0);
  // SCANLINE EFFECT float scanline =  0.85 + 0.05 * sin(EffectTexCoords.y * scanlineCount * 6.2831853);
  // SCANLINE EFFECT color *= scanline * flicker;

  FragColor = vec4(color, FragColor.a) * vec4(colorGrade.rgb, 1);


}
