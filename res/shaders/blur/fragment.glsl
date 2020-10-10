#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;


uniform bool firstpass;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.216216); // source learnopengl.com 

void main(){
   vec2 texSize = 1.0 / textureSize(framebufferTexture, 0);
   vec3 result = texture(framebufferTexture, TexCoords).rgb * weight[0];

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
}
