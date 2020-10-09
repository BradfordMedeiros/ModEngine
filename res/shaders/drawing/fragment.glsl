#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;
uniform float opacity;
uniform vec3 tint;

void main(){
   FragColor = (vec4(tint.rgb, 1.0) * texture(framebufferTexture, TexCoords)) * vec4(1, 1, 1, opacity);
}
