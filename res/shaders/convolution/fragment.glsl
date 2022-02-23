#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;

uniform float amount;
uniform float kernel[9];

void main(){
  vec3 result = vec3(0, 0, 0);
  vec2 texSize = 1.0 / textureSize(framebufferTexture, 0);
  float offsetPerSample = 1;
  for (int x = -1; x <= 1; x++){
   for (int y = -1; y <= 1; y++){
      int index = (x + 1) * 3 + (y + 1);
      float weight = kernel[index];
      result += weight * (texture(
        framebufferTexture, 
        TexCoords + vec2(texSize.x * x * offsetPerSample * amount , texSize.y * y * offsetPerSample * amount)
      ).rgb);
    }
  }
  FragColor = vec4(result.r, result.g, result.b, 1);
}

/*  if (result.r > 0.1 || result.g > 0.1 || result.b > 0.1){
    FragColor = vec4(1, 0, 0, 1);
  }else{
    discard;
  }*/