#include "./texture.h"

Texture loadTextureData(unsigned char* data, int textureWidth, int textureHeight, int numChannels){
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  
  GLint format = GL_RGB;
  if (numChannels == 4) {
    format = GL_RGBA;
  }
  
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // https://stackoverflow.com/questions/15983607/opengl-texture-tilted
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, format, textureWidth, textureHeight, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  
  Texture tex {
    .textureId = texture,
  };
  return tex;
}

Texture loadTexture(std::string textureFilePath){
  std::cout << "Event: loading texture: " << textureFilePath << std::endl;

  int textureWidth, textureHeight, numChannels;
  unsigned char* data = stbi_load(textureFilePath.c_str(), &textureWidth, &textureHeight, &numChannels, 0); 
  if (!data){
    throw std::runtime_error("failed loading texture " + textureFilePath + ", reason: " + stbi_failure_reason());
  }
  auto texture = loadTextureData(data, textureWidth, textureHeight, numChannels);
  stbi_image_free(data);
  return texture;
}

void updateTextureData(Texture& texture, unsigned char* data, int textureWidth, int textureHeight){
  glBindTexture(GL_TEXTURE_2D, texture.textureId);
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, textureWidth, textureHeight, GL_RGB, GL_UNSIGNED_BYTE, data);
}

void saveTextureData(std::string filepath, char* data, int width, int height){
  stbi_flip_vertically_on_write(1);
  stbi_write_png(filepath.c_str(), width, height, 3, data, 0); 
}