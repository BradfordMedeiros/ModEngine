#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform bool flipCoords;

void main(){
  if (flipCoords){
    gl_Position = vec4(aPos.x, -1 * aPos.y, 0.0, 1.0);
  }else{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
  }
  TexCoords = aTexCoords;
}
