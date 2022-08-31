#version 430 

in vec3 FragPos;
in vec2 TexCoord;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

uniform sampler2D maintexture;

void main(){
    vec4 diffuseColor = texture(maintexture, TexCoord);
    FragColor = diffuseColor;
    float brightness = FragColor.r + FragColor.g + FragColor.b;
    if(brightness > 2.7){
      BloomColor = vec4(FragColor.rgb, 1.0);
    }else{
      BloomColor = vec4(0.0, 0.0, 0.0, 0.0);    
    }       
}
