#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in int aBoneIndex[4];
layout (location = 7) in float aBoneWeight[4];

uniform mat4 model;
uniform mat4 bones[100];
uniform bool hasBones;


//uniform mat4 worldtolight;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 FragLight;

// temporary
out vec4 glFragPos;
flat out vec4 overcolor;
uniform bool showBoneWeight;
uniform bool useBoneTransform;


////

void main(){
  if (hasBones){
    vec4 modelPosition;
    if (!useBoneTransform){
      modelPosition= vec4(aPos.xyz, 1.0);
      overcolor = vec4(0, 0, 1, 1);
    }else{          
      float totalWeight = aBoneWeight[0] + aBoneWeight[1] + aBoneWeight[2] + aBoneWeight[3];  // @TODO -> what if there's no weights?  Probably should just use identity
      float multiplier = 1 / totalWeight;
      bool useIdentity = totalWeight < 0.1;

      mat4 mul = 
      (bones[aBoneIndex[0]] * aBoneWeight[0] * multiplier) + 
      (bones[aBoneIndex[1]] * aBoneWeight[1] * multiplier) + 
      (bones[aBoneIndex[2]] * aBoneWeight[2] * multiplier) + 
      (bones[aBoneIndex[3]] * aBoneWeight[3] * multiplier);

      if (useIdentity){
        mul = mat4(1);
      }

      modelPosition = mul * vec4(aPos.xyz, 1.0);

      /*if (totalWeight == 1){
        overcolor = vec4(1, 0, 0, 1);
      }else{
        overcolor = vec4(0, 1, 0, 1);
      }*/

    }

    gl_Position = projview *  model * vec4(modelPosition.x, modelPosition.y, modelPosition.z, 1.0);
    TexCoord = aTexCoords;
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    FragPos = modelPosition.xyz;
    

    if (!showBoneWeight){
      overcolor = vec4(aBoneIndex[0], aBoneIndex[1], aBoneIndex[2], 1.0);
    }else{
      overcolor = vec4(aBoneWeight[0], aBoneWeight[1], aBoneWeight[2], 1.0);
    }

    glFragPos = gl_Position;
  }else{
    vec4 modelPosition = model * vec4(aPos.xyz, 1.0);

    gl_Position = projview * modelPosition;
    TexCoord = aTexCoords;
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    FragPos = modelPosition.xyz;
    glFragPos = gl_Position;
  }

  //FragLight = worldtolight * vec4(FragPos.xyz, 1.0);
} 
