#version 330 core
out vec4 FragColor;
in vec3 FragPos;
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


    vec3 cameraPosition = vec3(0.0, -1.0, -5.0);
    vec3 lightPosition = vec3(0.0, -1.0, 0.0);

    vec3 lightDir = normalize(lightPosition - FragPos);
    vec3 ambient = vec3(0.2, 0.2, 0.2);
    vec3 diffuse = max(dot(normalize(Normal), lightDir), 0.0) * vec3(1.0, 1.0, 1.0);

    vec4 color = vec4(ambient + diffuse, 1.0) * texColor;

    FragColor = color * vec4(tint.x, tint.y, tint.z, 1.0);
  }
}
