
vec3 calculatePhongLight(vec3 normal){
  vec3 ambient = lookupAmbientLight();   
  vec3 totalDiffuse  = vec3(0, 0, 0);     
  vec3 totalSpecular = vec3(0, 0, 0);     

  int voxelLights[ $LIGHTS_PER_VOXEL ];
  getLights(voxelLights);

  for (int x = 0; x < 5; x++){
    int lightIndex = voxelLights[x];
    if (lightIndex == -1){
      // no lights
      continue;
    }else if (lightIndex == -2){
      totalDiffuse += vec3(0, 1, 0);
      continue;
    }

    vec3 lightPos = lights[lightIndex];
    vec3 lightDir = lightsisdir[lightIndex] ?  lightsdir[lightIndex] : normalize(lightPos - FragPos);

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
    vec3 specular = pow(max(dot(viewDir, reflectDir), 0.0), 32) * vec3(1.0, 1.0, 1.0);  
    float attenuation = calcAttenutation(lightIndex);

    totalDiffuse = totalDiffuse + angleFactor * (attenuation * diffuse * lightscolor[lightIndex]);
    totalSpecular = totalSpecular + angleFactor * (attenuation * specular * lightscolor[lightIndex]);
  }

  vec3 diffuseValue = enableDiffuse ? totalDiffuse : vec3(0, 0, 0);
  vec3 specularValue = enableSpecular ? totalSpecular : vec3(0, 0, 0);
  vec3 color = ambient + diffuseValue + specularValue;
  return color;
}