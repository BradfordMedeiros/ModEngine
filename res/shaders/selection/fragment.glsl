#version 330 core

layout(location = 0) out vec4 EncodeId;
layout(location = 1) out vec4 UVCoords;

uniform int textureid;
uniform vec4 encodedid;
uniform int voxelcellwidth;
uniform vec4 tint;
uniform vec3 cameraPosition;
uniform bool forceTint;

in vec2 TexCoord;



void main(){
  if (voxelcellwidth > 100 || tint.x > 100 || cameraPosition.x == 10000 || forceTint){  // removing selection shader anyway so 
    discard;
  }

  EncodeId = vec4(encodedid.x, encodedid.y, encodedid.z, encodedid.w);
  UVCoords = vec4(TexCoord.x, TexCoord.y, textureid, 0);
}
