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
  if (enableDepthVisualization){
    float depth  = texture(depthTexture, TexCoords).r;
    float z = depth * 2.0 - 1.0; 
    float depthAmount  = ((2.0 * near * far) / (far + near - z * (far - near))) / far;  // fraction of near/far
    FragColor = vec4(depthAmount, depthAmount, depthAmount, 1);
    return;
  }


  float depth  = texture(depthTexture, TexCoords).r;
  float z = depth * 2.0 - 1.0; 
  float depthAmount  = ((2.0 * near * far) / (far + near - z * (far - near))) / far;  // fraction of near/far
  
  vec4 fogEffect = vec4(0, 0, 0, 0);
  // in the 0.2 remaining, it should fall off 100%
  if (enableFog){
    calculateFogEffect(depthAmount, fogEffect);
  }

  if (enableBloom){
    FragColor = fogEffect + (texture(framebufferTexture, TexCoords) + (bloomAmount * texture(bloomTexture, TexCoords)));
  }else{
    FragColor = fogEffect + texture(framebufferTexture, TexCoords);
  }

  // should restore after testing a bit more
  //vec3 color = FragColor.rgb;

  //   // chromatic abberation 

  vec2 offset = vec2(cos(realtime) * 0.005, 0.002);
  vec3 color = vec3(
    texture(framebufferTexture, TexCoords + offset).r,
    texture(framebufferTexture, TexCoords).g,
    texture(framebufferTexture, TexCoords - offset).b
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

  color = (color - 0.5) * contrast + 0.5;


  FragColor = vec4(color, FragColor.a) * vec4(colorGrade.rgb, 1);
}
