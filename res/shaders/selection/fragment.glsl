#version 330 core
out vec4 FragColor;

uniform vec4 encodedid;

void main(){
  FragColor = vec4(encodedid.x, encodedid.y, encodedid.z, encodedid.w);
  //FragColor = vec4(0.1, 0.2, 0.3, 0.8);
}
