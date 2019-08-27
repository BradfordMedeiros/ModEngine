#ifndef MOD_MESH
#define MOD_MESH

#include <iostream>
#include <glm/glm.hpp>
#include <iostream>
#include <stdexcept>
#include <stb_image.h>
#include <stb_image.h>
#include "glad/glad.h"
#include "./util/loadmodel.h"

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
Mesh load2DMeshTexCoords(std::string imagePath, float offsetx, float offsety, float width, float height); // 2DMesh with subimage selection
Mesh loadSpriteMesh(std::string imagePath);  // loads a 2d mesh with vertex centered around 0 to 1 x/y
void drawMesh(Mesh);  						 // draws mesh and related texture info (no shader data supplied)

#endif 
