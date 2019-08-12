#version 330 core
out vec4 FragColor;

uniform vec3 tint;

void main(){
  FragColor = vec4(tint.x, tint.y, tint.z, 1.0);
}
