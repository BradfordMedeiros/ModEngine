
const float PI = 3.14159265359;

//////////
// Background: https://www.scitepress.org/Papers/2021/102527/102527.pdf
 //  https://www.scitepress.org/Papers/2021/102527/102527.pdf

// approximation for fraction of light that gets reflected (not refracted)
vec3 fresnelSchlick(float angle, vec3 F0){
    return F0 + (1.0 - F0) * pow(clamp(1.0 - angle, 0.0, 1.0), 5);
} 

// Approximation for the fraction the microfacets aligned to halfway vector 
float trowbridgereitzGGX(vec3 N, vec3 H, float roughness){
    float a = roughness * roughness;
    float asquared = a * a;
    float nOnH  = max(dot(N, H), 0.0);
    return asquared / (PI * pow(((nOnH * nOnH) * (asquared - 1.0) + 1.0), 2));
}

//  Approximation for fraction of not occluded geometry (do to one microfacet overlapping the ray of another)
float schlickGGX(float nOnV, float roughness){
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return nOnV / (nOnV * (1.0 - k) + k);
}
float smith(vec3 N, vec3 V, vec3 L, float roughness){
    float nOnV = max(dot(N, V), 0.0);
    float nOnL = max(dot(N, L), 0.0);
    float ggx2  = schlickGGX(nOnV, roughness);
    float ggx1  = schlickGGX(nOnL, roughness);
    return ggx1 * ggx2;
}

vec4 calculateCookTorrence(
  vec3 N,
  vec3 albedo, 
  float metallic, 
  float roughness // 1 is rough, disperes light, 0 is metallic, darker with highlights. perfectly smooth is just dark so pick something small
){ 
    vec3 V = normalize(cameraPosition - FragPos);  
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < getNumLights(); ++i){
        vec3 L = normalize(lights[i] - FragPos);
        vec3 H = normalize(V + L);
        float attenuation = calcAttenutation(i);
        vec3 radiance = lightscolor[i] * attenuation;        
        
        float D = trowbridgereitzGGX(N, H, roughness);        
        float G = smith(N, V, L, roughness);      
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);       
        
        vec3 kS = F;
        vec3 kD = (1.0 - metallic) * (vec3(1.0) - kS);
        
        vec3 specular = (D * G * F) / (4 * max(dot(N, V), 0) * max(dot(N, L), 0) + 0.0001);  
            
        float nOnL = max(dot(N, L), 0.0);                
        Lo += ((kD * albedo / PI) + specular) * radiance * nOnL; 
    }

    float ao = 3.2;
    vec3 ambient = vec3(0.53) * albedo * ao;
    vec4 color = vec4(ambient + Lo, 1);
    return color;
}