#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in int aBoneIndex[4];
layout (location = 8) in float aBoneWeight[4];

// per object
uniform mat4 model;
uniform mat4 bones[ $BONES_BUFFER ];
uniform bool hasBones;
uniform float time;

// per frame
uniform mat4 projview;
uniform mat4 lightsprojview;

// global 
uniform bool showBoneWeight;
uniform bool useBoneTransform;


out vec2 TexCoord;
out vec3 Normal;
out mat3 TangentToWorld;
out vec3 FragPos;
out vec4 FragLight;
out vec4 sshadowCoord;
flat out int instanceId;

out vec4 glFragPos;
flat out vec4 overcolor;

uniform bool useInstancing;
uniform vec3 instanceOffsets[100]; // for now...this would be nicer as an array buffer but code should all be the same besides making the buffer


void calculateXYZ(in int index, out int x, out int y, out int z){
  x = 0;
  y = 0;
  z = 0;
}

void main(){
  instanceId = gl_InstanceID;
  vec3 tangent = normalize(vec3(model * vec4(aTangent, 0.0)));
  vec3 norm = normalize(vec3(model * vec4(aNormal, 0.0)));
  tangent = normalize(tangent - dot(tangent, norm) * norm);
  vec3 bitangent = cross(norm, tangent);
  TangentToWorld = transpose(mat3(tangent, bitangent, norm));

  if (hasBones){
    vec4 modelPosition;
    mat4 fullmodel;
    if (!useBoneTransform){
      modelPosition= vec4(aPos.xyz, 1.0);
      overcolor = vec4(0, 0, 1, 1);
    }else{          
      float totalWeight = aBoneWeight[0] + aBoneWeight[1] + aBoneWeight[2] + aBoneWeight[3];  // @TODO -> what if there's no weights?  Probably should just use identity
      float multiplier = 1 / totalWeight;

      mat4 mul = 
      (bones[aBoneIndex[0]] * aBoneWeight[0] * multiplier) + 
      (bones[aBoneIndex[1]] * aBoneWeight[1] * multiplier) + 
      (bones[aBoneIndex[2]] * aBoneWeight[2] * multiplier) + 
      (bones[aBoneIndex[3]] * aBoneWeight[3] * multiplier);

      fullmodel = model  * mul;
      
      if (useInstancing){
        fullmodel = model  * mul;
      }
      modelPosition = fullmodel * vec4(aPos.xyz, 1.0);

      /*if (totalWeight == 1){
        overcolor = vec4(1, 0, 0, 1);
      }else{
        overcolor = vec4(0, 1, 0, 1);
      }*/

    }

    gl_Position = projview *  vec4(modelPosition.x, modelPosition.y, modelPosition.z, 1.0);
    TexCoord = vec2(aTexCoords.x, -aTexCoords.y);
    Normal = mat3(transpose(inverse(fullmodel))) * aNormal;  
    FragPos = modelPosition.xyz;
    sshadowCoord = lightsprojview * vec4(FragPos, 1.0);

    if (true){
      overcolor = vec4(aBoneIndex[0], aBoneIndex[1], aBoneIndex[2], 1.0);
    }else{
      overcolor = vec4(aBoneWeight[0], aBoneWeight[1], aBoneWeight[2], 1.0);
    }

    glFragPos = gl_Position;
  }else{
    vec4 modelPosition = model * vec4(aPos.xyz, 1.0);

    if (useInstancing){
      vec3 offset = instanceOffsets[gl_InstanceID];
      //vec3 offset = vec3(gl_InstanceID * 1.f, gl_InstanceID)
      modelPosition = modelPosition + vec4(offset.xyz, 0);
    }

    gl_Position = projview * modelPosition;
    TexCoord = vec2(aTexCoords.x, -aTexCoords.y);
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    FragPos = modelPosition.xyz;
    sshadowCoord = lightsprojview * vec4(FragPos, 1.0);

    glFragPos = gl_Position;
  }
} 
