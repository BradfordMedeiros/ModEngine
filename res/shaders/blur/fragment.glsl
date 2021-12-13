#version 330 core 

// Would be use algorithm that can specify blur radius w/ constant time... does that work...
// or at least just parameterize radius

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;
uniform sampler2D depthTexture;
uniform bool useDepthTexture;
uniform float minBlurDistance;
uniform float maxBlurDistance;
uniform float near;
uniform float far;
uniform int amount;
uniform bool firstpass;

bool shouldBlur(){
  float depth  = texture(depthTexture, TexCoords).r;
  float z = depth * 2.0 - 1.0; 
  float depthAmount  = ((2.0 * near * far) / (far + near - z * (far - near)));  // fraction of near/far
  bool blur = !useDepthTexture || (depthAmount < minBlurDistance || depthAmount > maxBlurDistance);
  return blur;
}

int numSamples = 4;
float weight = 1.0 / ((2 * numSamples) -1);

// box filter blur for now
// guassian would look better
void main(){
    bool blur = shouldBlur();
    if (blur){
      vec3 result = vec3(0, 0, 0);
      vec2 texSize = 1.0 / textureSize(framebufferTexture, 0);
      float offsetPerSample = amount / numSamples;
      if (firstpass){
        for (int i = (-numSamples + 1); i < numSamples; i++){
          result += weight * texture(framebufferTexture, TexCoords + vec2(texSize.x * i * offsetPerSample, 0.0)).rgb;
        }
      }else{
        for (int i = (-numSamples + 1); i < numSamples; i++){
          result += weight * texture(framebufferTexture, TexCoords + vec2(0.0, texSize.y * i * offsetPerSample)).rgb;
        }
      }
      FragColor = vec4(result.rgb, 1.0);
   }else{
      FragColor = vec4(texture(framebufferTexture, TexCoords).rgb, 1.0);
   }
}
