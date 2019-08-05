#ifndef MOD_MESH
#define MOD_MESH
#include <iostream>
#include <glm/glm.hpp>


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
  unsigned int numElements;
};

Mesh loadMesh(std::string modelPath);		 // loads model and returns mesh/bound texture data
Mesh load2DMesh(std::string imagePath);		 // loads single quad mesh with texture centered around -1 to 1 x/y
Mesh loadSpriteMesh(std::string imagePath);  // loads a 2d mesh with vertex centered around 0 to 1 x/y
void drawMesh(Mesh);  						 // draws mesh and related texture info (no shader data supplied)

#endif 
