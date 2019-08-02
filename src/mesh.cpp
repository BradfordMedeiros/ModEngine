#include <iostream>
#include <stdexcept>
#include <stb_image.h>
#include "glad/glad.h"
#include <stb_image.h>
#include "./mesh.h"
#include "./loadmodel.h"

Mesh loadMesh(std::string modelPath){
  std::vector<ModelData> models = loadModel(modelPath);
  
  ModelData model = models[0];

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO); 

  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * model.indices.size(), &(model.indices[0]), GL_STATIC_DRAW);

  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * model.vertices.size(), &(model.vertices[0]), GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  glEnableVertexAttribArray(0);
 
  Texture texture = loadTexture(model.texturePaths[0]);
  useTexture(texture);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(1);

  Mesh mesh = {
    .VAOPointer = VAO,
    .texture = texture,
    .numElements = model.indices.size(),
  }; 

  return mesh; 
}

void drawMesh(Mesh mesh){
  glBindVertexArray(mesh.VAOPointer);
  useTexture(mesh.texture);
  glDrawElements(GL_TRIANGLES, mesh.numElements, GL_UNSIGNED_INT, 0);
}

Texture loadTexture(std::string textureFilePath){
  std::cout << "Event: loading texture: " << textureFilePath << std::endl;

  int textureWidth, textureHeight, numChannels;
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  
  unsigned char* data = stbi_load(textureFilePath.c_str(), &textureWidth, &textureHeight, &numChannels, 0); 
  if (!data){
    throw std::runtime_error("failed loading texture " + textureFilePath);
  }  
 
  Texture tex = Texture {
    .textureId = texture,
    .data = data,
    .textureWidth = textureWidth,
    .textureHeight = textureHeight,
  };
  return tex;
}

void useTexture(Texture texture){
  glBindTexture(GL_TEXTURE_2D, texture.textureId);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.textureWidth, texture.textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texture.data);
  glGenerateMipmap(GL_TEXTURE_2D);
}

void freeTextureData(Texture& texture){
   stbi_image_free(texture.data);
}