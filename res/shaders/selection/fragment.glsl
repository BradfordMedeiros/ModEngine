#version 330 core
out vec4 FragColor;

uniform vec3 encodedid;

void main(){
  FragColor = vec4(encodedid.x, encodedid.y, encodedid.z, 1.0);
}
