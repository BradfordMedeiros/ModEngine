#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;
uniform float opacity;
uniform vec4 tint;

void main(){
   FragColor = (tint * texture(framebufferTexture, TexCoords)) * vec4(1, 1, 1, opacity);
}
