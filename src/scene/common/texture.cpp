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

Texture loadCubemapTextureData(std::vector<unsigned char*> data, std::vector<int> textureWidth, std::vector<int> textureHeight, std::vector<int> numChannels){
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
  
  for (int i = 0; i < data.size(); i++){
    GLint format = GL_RGB;
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, textureWidth.at(i), textureHeight.at(i), 0, format, GL_UNSIGNED_BYTE, data.at(i));
  } 

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  Texture tex {
    .textureId = texture,
  };
  return tex;
}

Texture loadCubemapTexture(std::string textureFilePathRoot){
  std::cout << "Event: loading texture: " << textureFilePathRoot << std::endl;

  std::vector<std::string> filepaths = {
    textureFilePathRoot + "right.jpg",
    textureFilePathRoot + "left.jpg",
    textureFilePathRoot + "top.jpg",
    textureFilePathRoot + "bottom.jpg",
    textureFilePathRoot + "front.jpg",
    textureFilePathRoot + "back.jpg",
  };

  std::vector<unsigned char*> datas;
  std::vector<int> textureWidths;
  std::vector<int> textureHeights;
  std::vector<int> channels;

  std::cout << "start loading raw jpg data" << std::endl;
  for (auto filepath : filepaths){
    int textureWidth, textureHeight, numChannels;
    std::cout << "loading file: " << filepath << std::endl;
    unsigned char* data = stbi_load(filepath.c_str(), &textureWidth, &textureHeight, &numChannels, 0); 
    if (!data){
      throw std::runtime_error("failed loading texture " + textureFilePathRoot + ", reason: " + stbi_failure_reason());
    }
    datas.push_back(data);
    textureWidths.push_back(textureWidth);
    textureHeights.push_back(textureHeight);
    channels.push_back(numChannels);
  }
  std::cout << "finishing loading raw jpg data" << std::endl;

  std::cout << "start loading cubemap data" << std::endl;
  auto texture = loadCubemapTextureData(datas, textureWidths, textureHeights, channels);
  std::cout << "finishing loading cubemap data" << std::endl;

  for (int i = 0; i < datas.size(); i++){
    std::cout << "start freeing data: " << i << std::endl;
    stbi_image_free(datas.at(i));
    std::cout << "finish freeing data: " << i << std::endl;
  }
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