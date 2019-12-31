#version 330 core
out vec4 FragColor;
in vec3 FragPos;
in vec2 TexCoord;
in vec3 Normal;
uniform sampler2D textureData;
uniform vec3 tint;
uniform vec3 cameraPosition;
uniform vec3 lightPosition;

uniform bool enableDiffuse;
uniform bool enableSpecular;

void main(){
  if (tint.r < 0.1){
    FragColor = vec4(tint.r, tint.g, tint.b, 1.0);
  }else{
    vec4 texColor = texture(textureData, TexCoord);
    if (texColor.a < 0.1){
      discard;
    }

    vec3 lightDir = normalize(lightPosition - FragPos);
    vec3 normal = normalize(Normal);

    // Ambient lighting
    vec3 ambient = vec3(0.2, 0.2, 0.2);

    // Diffuse lighting
    vec3 diffuse = max(dot(normal, lightDir), 0.0) * vec3(1.0, 1.0, 1.0);

    // Specular lighting
    vec3 viewDir = normalize(cameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);  
    vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), 32) * vec3(1.0, 1.0, 1.0);

    vec3 diffuseValue = enableDiffuse ? diffuse : vec3(0, 0, 0);
    vec3 specularValue = enableSpecular ? specular : vec3(0, 0, 0);
    vec4 color = vec4(ambient + diffuseValue + specularValue, 1.0) * texColor;
    FragColor = color * vec4(tint.x, tint.y, tint.z, 1.0);
  }
}
