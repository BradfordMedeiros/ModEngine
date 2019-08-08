#include <iostream>
#include <stdexcept>
#include <stb_image.h>
#include <stb_image.h>
#include "glad/glad.h"
#include "./mesh.h"
#include "./loadmodel.h"

// Generating the VAO per model is probaby not the most efficient, but figure that this is 
// a clean abstraction, and we can optimize this fucker after we get more features in it.
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

Mesh load2DMeshHelper(std::string imagePath, float vertices[], unsigned int indices[], 
  unsigned int dataSize, unsigned int numIndices, unsigned int vertexWidth, unsigned int textureWidth){
  unsigned int bufferWidth = vertexWidth + textureWidth;

  Texture texture = loadTexture(imagePath);

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO); 

  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numIndices, &(indices[0]), GL_STATIC_DRAW);

  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * dataSize, &(vertices[0]), GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, vertexWidth, GL_FLOAT, GL_FALSE, sizeof(float) * bufferWidth, (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, textureWidth, GL_FLOAT, GL_FALSE, sizeof(float) *  bufferWidth, (void*)(sizeof(float) * vertexWidth));
  glEnableVertexAttribArray(1);
 
  Mesh mesh = {
    .VAOPointer = VAO,
    .texture = texture,
    .numElements = 6,
  };
  return mesh;
}
Mesh load2DMesh(std::string imagePath){
  float quadVerts[] = {
    -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
    1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
  };
  unsigned int indices[] = {0, 1, 2, 0, 2, 3};
  return load2DMeshHelper(imagePath, quadVerts, indices, 20, 6, 3, 2);
}
Mesh loadSpriteMesh(std::string imagePath){
  float uiVerts[] = {
    0.0f,  1.0f, 0.0f,  0.0f, 1.0f,
    0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
  };
  unsigned int indices[] = {0, 1, 2, 0, 2, 3};
  return load2DMeshHelper(imagePath, uiVerts, indices, 20, 6, 3, 2);  
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
  
  unsigned char* data = stbi_load(textureFilePath.c_str(), &textureWidth, &textureHeight, &numChannels, 0); 

  GLint format = GL_RGB;
  if (numChannels == 4) {
    format = GL_RGBA;
  }
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, format, textureWidth, textureHeight, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  if (!data){
    throw std::runtime_error("failed loading texture " + textureFilePath);
  }  
 
  Texture tex = Texture {
    .textureId = texture,
    .data = data,
  };

  return tex;
}

void useTexture(Texture texture){
  glBindTexture(GL_TEXTURE_2D, texture.textureId);
}

void freeTextureData(Texture& texture){
   stbi_image_free(texture.data);
}


