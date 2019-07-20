#version 330 core 

out vec4 FragColor;
in vec2 TexCoords;
uniform sampler2D framebufferTexture;

void main(){
   vec4 color = texture(framebufferTexture, TexCoords);
   float avgColor = (color.r + color.g + color.b) / 3;
   FragColor = vec4(avgColor, avgColor, avgColor, color.a);
}
