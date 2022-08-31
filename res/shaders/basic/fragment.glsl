#version 430 

in vec3 FragPos;
in vec2 TexCoord;
layout (location = 0) out vec4 FragColor;

uniform sampler2D maintexture;

void main(){
    FragColor = texture(maintexture, TexCoord);
}
