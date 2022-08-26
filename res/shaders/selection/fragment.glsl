#version 330 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec2 UVCoords;
layout(location = 2) out vec4 FragColor2;

uniform vec4 encodedid;
uniform vec4 encodedid2;
in vec2 TexCoord;

void main(){
  FragColor = vec4(encodedid.x, encodedid.y, encodedid.z, encodedid.w);
  FragColor2 = vec4(encodedid2.x, encodedid2.y, encodedid2.z, encodedid2.w);
  UVCoords = TexCoord;
}
