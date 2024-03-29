#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoord;
out vec3 FragPos;
uniform mat4 projview;

void main(){
    vec4 modelPosition =  vec4(aPos.xyz, 1.0);
    gl_Position = projview * modelPosition;
    TexCoord = aTexCoords;
} 
