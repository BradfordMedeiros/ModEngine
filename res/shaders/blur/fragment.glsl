#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;

uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.216216);

void main(){
   vec2 texSize = 1.0 / textureSize(framebufferTexture, 0);
   vec3 result = texture(framebufferTexture, TexCoords).rgb * weight[0];

   if (horizontal){
      for (int i = 1; i < 5; i++){
        result += texture(framebufferTexture, TexCoords + vec2(texSize.x * i, 0.0)).rgb * weight[i];
        result += texture(framebufferTexture, TexCoords - vec2(texSize.x * i, 0.0)).rgb * weight[i];
      }
    }else{
      for (int i = 1; i < 5; i++){
        result += texture(framebufferTexture, TexCoords + vec2(0.0, texSize.y * i)).rgb * weight[i];
        result += texture(framebufferTexture, TexCoords - vec2(0.0, texSize.y * i)).rgb * weight[i];
      }
    }

   FragColor = vec4(result.rgb, 1.0);
}
