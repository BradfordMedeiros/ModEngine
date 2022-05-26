#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D maintexture;
uniform sampler2D emissionTexture;
uniform sampler2D opacityTexture;

uniform vec4 tint;
uniform vec3 cameraPosition;

uniform bool enableDiffuse;
uniform bool enableSpecular;
uniform bool hasEmissionTexture;
uniform bool hasOpacityTexture;
uniform vec2 textureOffset;

#define MAX_LIGHTS 32
uniform int numlights;
uniform vec3 lights[MAX_LIGHTS];
uniform vec3 lightscolor[MAX_LIGHTS];
uniform vec3 lightsdir[MAX_LIGHTS];

const float constant = 0.1;
const float linear = 0.1;
const float quadratic = 0.0;

const float emissionAmount = 1;
uniform float discardTexAmount;

void main(){
  if (tint.b > 0.8){
    discard;
  }
    vec2 adjustedTexCoord = TexCoord + textureOffset;
    vec4 diffuseColor = texture(maintexture, vec2(adjustedTexCoord.x, -adjustedTexCoord.y));
   
    FragColor = tint * diffuseColor;
  
}
