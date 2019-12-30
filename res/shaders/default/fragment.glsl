#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
uniform sampler2D textureData;
uniform vec3 tint;

void main(){
  if (tint.r < 0.1){
    FragColor = vec4(tint.r, tint.g, tint.b, 1.0);
  }else{
    vec4 texColor = texture(textureData, TexCoord);
    if (texColor.a < 0.1){
      discard;
    }
    FragColor = texColor * vec4(tint.x, tint.y, tint.z, 1.0);
  }
}
