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

uniform bool firstpass;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);; // source learnopengl.com 

bool shouldBlur(){
   float near = 0.1;
   float far = 100;
   float depth  = texture(depthTexture, TexCoords).r;
   float z = depth * 2.0 - 1.0; 
   float depthAmount  = ((2.0 * near * far) / (far + near - z * (far - near))) / far;  // fraction of near/far
    bool blur = !useDepthTexture || (depthAmount < minBlurDistance || depthAmount > maxBlurDistance);
    return blur;
}

void main(){
    bool blur = shouldBlur();
    if (blur){
      vec3 result = texture(framebufferTexture, TexCoords).rgb * weight[0];
      vec2 texSize = 1.0 / textureSize(framebufferTexture, 0);
      if (firstpass){
        for (int i = 1; i < 5; i++){
          result += weight[i] * texture(framebufferTexture, TexCoords + vec2(texSize.x * i, 0.0)).rgb;
          result += weight[i] * texture(framebufferTexture, TexCoords - vec2(texSize.x * i, 0.0)).rgb;
        }
      }else{
        for (int i = 1; i < 5; i++){
          result += weight[i] * texture(framebufferTexture, TexCoords + vec2(0.0, texSize.y * i)).rgb;
          result += weight[i] * texture(framebufferTexture, TexCoords - vec2(0.0, texSize.y * i)).rgb;
        }
      }
      FragColor = vec4(result.rgb, 1.0);
   }else{
      FragColor = vec4(texture(framebufferTexture, TexCoords).rgb, 1.0);
   }
}
