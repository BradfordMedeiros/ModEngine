#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;

float opacity = 0.5;
void main(){
   FragColor = texture(framebufferTexture, TexCoords) * vec4(1, 1, 1, opacity);
}
