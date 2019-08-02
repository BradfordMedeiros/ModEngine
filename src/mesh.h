#ifndef MOD_MESH
#define MOD_MESH
#include <iostream>
#include <glm/glm.hpp>


struct Texture {
   unsigned int textureId;
   unsigned char* data;
   int textureWidth;
   int textureHeight;
};

Texture loadTexture(std::string textureFilePath);
void useTexture(Texture texture);
void freeTextureData(Texture& texture);

struct Mesh {
  unsigned int VAOPointer;
  Texture texture; 
  unsigned int numElements;
};

Mesh loadMesh(std::string modelPath);
Mesh load2DMesh(std::string imagePath);
void drawMesh(Mesh);  

#endif 
