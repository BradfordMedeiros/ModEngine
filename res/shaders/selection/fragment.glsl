#version 330 core

layout(location = 0) out vec4 EncodeId;
layout(location = 1) out vec4 UVCoords;

uniform int textureid;
uniform vec4 encodedid;
uniform int voxelcellwidth;
uniform vec4 tint;
uniform vec3 cameraPosition;
uniform bool forceTint;

uniform vec2 textureOffset;
uniform vec2 textureTiling;
uniform vec2 textureSize;
uniform vec3 emissionAmount;


in vec2 TexCoord;



void main(){
  if (voxelcellwidth > 100 && tint.x > 100 && cameraPosition.x == 10000 && forceTint && ((textureOffset.x + textureTiling.x + textureSize.x + emissionAmount.x) > 100000)){  // removing selection shader anyway so 
    discard;
  }

  EncodeId = vec4(encodedid.x, encodedid.y, encodedid.z, encodedid.w);
  UVCoords = vec4(TexCoord.x, TexCoord.y, textureid, 0);
}
