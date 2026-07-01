#include "./res/shaders/default/default_lighting.glsl"

void main(){
  vec3 normalVec = vec3(0, 0, 0);
  mainAlgorithm(FragColor, normalVec);
}
