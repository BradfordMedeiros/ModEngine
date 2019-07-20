#ifndef MOD_MESH
#define MOD_MESH
#include <iostream>

struct Texture {
   unsigned int textureId;
   unsigned char* data;
};

Texture loadTexture(std::string textureFilePath);
void useTexture(Texture texture);
void freeTextureData(Texture& texture);

struct Mesh {
  unsigned int VAOPointer;
  Texture texture; 
};

Mesh loadMesh(std::string textureFilePath);  // load mesh and put in into the buffers? 
void drawMesh(Mesh);  // call draw arrays

#endif 
