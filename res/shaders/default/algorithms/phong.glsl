
const float PI = 3.14159265359;

vec3 calculatePhongLight(vec3 normal, out vec3 lightPos, out bool hasLight, bool visualizeLights, out mat4 lightRot){
  vec3 ambient = lookupAmbientLight();   
  vec3 totalDiffuse  = vec3(0, 0, 0);     
  vec3 totalSpecular = vec3(0, 0, 0);     

  int voxelLights[ $LIGHTS_PER_VOXEL ];
  getLights(voxelLights);

  hasLight = false;
  for (int x = 0; x < $LIGHTS_PER_VOXEL; x++){
    int lightIndex = voxelLights[x];
    if (lightIndex == -1){
      // no lights
      continue;
    }else if (lightIndex == -2){
      totalDiffuse += vec3(1, 0, 0);
      continue;
    }

    hasLight = true;

    lightPos = lights[lightIndex];
    vec3 lightDir = lightsisdir[lightIndex] ?  lightsdir[lightIndex] : normalize(lightPos - FragPos);
    lightRot = lightsdirmat[lightIndex];

    float angle = dot(lightDir, normalize(-lightsdir[lightIndex]));

    float angleFactor = 1;
    float minAngle = lightsmaxangle[lightIndex];
    float maxAngle = minAngle + lightsangledelta[lightIndex];
    float angleAmount = mix(minAngle, maxAngle, angle);
    if (angle < maxAngle){
      if (angle < minAngle){
        continue;
      }
      angleFactor = (angle - minAngle) / (maxAngle - minAngle);
    }

    vec3 diffuse = max(dot(normal, lightDir), 0.0) * vec3(1.0, 1.0, 1.0);
    vec3 viewDir = normalize(cameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);  
    vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), 32) * vec3(0.0, 0.0, 0.00);  
    float attenuation = calcAttenutation(lightIndex);

    totalDiffuse = totalDiffuse + angleFactor * (attenuation * diffuse * lightscolor[lightIndex]);
    totalSpecular = totalSpecular + angleFactor * (attenuation * specular * lightscolor[lightIndex]);
  }

  {
    // TODO cleanup this code with above, it's duplicate. 
    int lightIndex = defaultVoxelLight;
    if (lightIndex == -1){
      // no lights
    }else if (lightIndex == -2){
      totalDiffuse += vec3(1, 0, 0);
    }else{
      hasLight = true;

      lightPos = lights[lightIndex];

      vec3 lightDir = lightsisdir[lightIndex] ?  lightsdir[lightIndex] : normalize(lightPos - FragPos);
      lightRot = lightsdirmat[lightIndex];

      float angle = dot(lightDir, normalize(-lightsdir[lightIndex]));

      float angleFactor = 1;
      float minAngle = lightsmaxangle[lightIndex];
      float maxAngle = minAngle + lightsangledelta[lightIndex];
      float angleAmount = mix(minAngle, maxAngle, angle);
      if (angle < maxAngle){
        if (angle < minAngle){
          //continue;
        }
        angleFactor = (angle - minAngle) / (maxAngle - minAngle);
      }

      vec3 diffuse = max(dot(normal, lightDir), 0.0) * vec3(0.4, 0.4, 0.4);
      vec3 viewDir = normalize(cameraPosition - FragPos);
      vec3 reflectDir = reflect(-lightDir, normal);  
      vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), 32) * vec3(0.3, 0.3, 0.3);  
      float attenuation = calcAttenutation(lightIndex);

      totalDiffuse = totalDiffuse + angleFactor * (attenuation * diffuse * lightscolor[lightIndex]);
      totalSpecular = totalSpecular + angleFactor * (attenuation * specular * lightscolor[lightIndex]);      
    }
  }

  vec3 diffuseValue = enableDiffuse ? totalDiffuse : vec3(0, 0, 0);
  vec3 specularValue = enableSpecular ? totalSpecular : vec3(0, 0, 0);
  vec3 color = ambient + diffuseValue + specularValue;

  if (visualizeLights){
    bool outOfRange = false;
    ivec3 indexs = calcLightIndexValues(outOfRange);
    int sum = indexs.x + indexs.y + indexs.z;

    if (hasLight){
      return vec3(1, 0, 0);
    }
    if ((sum & 1) == 0) {
        return vec3(color.r, color.g + 0.5, color.b + 0.5);
    } else {
        return vec3(color.r + 0.5, color.g + 0.5, color.b);
    }
  }
  return color;
}