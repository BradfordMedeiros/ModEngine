#include <iostream>
#include "glad/glad.h"
#include "./mesh.h"

float vertices[] = {
  0.5f,  0.5f, 0.0f,  
  0.5f, -0.5f, 0.0f,  
  -0.5f, -0.5f, 0.0f, 
  -0.5f,  0.5f, 0.0f, 
};
unsigned int indicies[] = {
  0, 1, 3, 
  1, 2, 3,
}; 


VAOPointer loadMesh(){
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO); 

  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  
  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  
  return VAO;
}

void drawMesh(VAOPointer vao){
  glBindVertexArray(vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); 
}


