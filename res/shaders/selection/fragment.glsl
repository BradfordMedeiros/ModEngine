#version 330 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 UVCoords;

uniform int textureid;
uniform vec4 encodedid;
in vec2 TexCoord;

void main(){
  FragColor = vec4(encodedid.x, encodedid.y, encodedid.z, encodedid.w);
  UVCoords = vec4(TexCoord.x, TexCoord.y, textureid, 0);
}
