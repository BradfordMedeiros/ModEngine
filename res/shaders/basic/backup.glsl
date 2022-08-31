#version 430 

in vec3 FragPos;
in vec2 TexCoord;
layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec4 BloomColor;

uniform sampler2D maintexture;
uniform bool hasDiffuseTexture;

uniform vec2 textureOffset;
uniform vec2 textureTiling;
uniform vec2 textureSize;


void main(){
    vec2 offsetTexCoord = vec2(TexCoord.x, TexCoord.y); 
    vec2 adjustedTexCoord = mod(offsetTexCoord * textureTiling, 1) * textureSize + textureOffset;
    vec4 diffuseColor = hasDiffuseTexture ? texture(maintexture, adjustedTexCoord) : vec4(1, 1, 1, 1);
    FragColor = diffuseColor;

    // TODO -> what would be a better thesholding function? 
    float brightness = FragColor.r + FragColor.g + FragColor.b;
    if(brightness > 2.7){
      BloomColor = vec4(FragColor.rgb, 1.0);
    }else{
      BloomColor = vec4(0.0, 0.0, 0.0, 0.0);    
    }       
}
