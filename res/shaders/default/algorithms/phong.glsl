
vec3 calculatePhongLight(vec3 normal){
  vec3 ambient = lookupAmbientLight();   
  vec3 totalDiffuse  = vec3(0, 0, 0);     
  vec3 totalSpecular = vec3(0, 0, 0);     

  for (int i = 0; i < getNumLights(); i++){
    vec3 lightPos = lights[i];
    vec3 lightDir = lightsisdir[i] ?  lightsdir[i] : normalize(lightPos - FragPos);

    float angle = dot(lightDir, normalize(-lightsdir[i]));

    float angleFactor = 1;
    float minAngle = lightsmaxangle[i];
    float maxAngle = minAngle + lightsangledelta[i];
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
    float attenuation = calcAttenutation(i);

    totalDiffuse = totalDiffuse + angleFactor * (attenuation * diffuse * lightscolor[i]);
    totalSpecular = totalSpecular + angleFactor * (attenuation * specular * lightscolor[i]);
  }

  vec3 diffuseValue = enableDiffuse ? totalDiffuse : vec3(0, 0, 0);
  vec3 specularValue = enableSpecular ? totalSpecular : vec3(0, 0, 0);
  vec3 color = ambient + diffuseValue + specularValue;
  return color;
}