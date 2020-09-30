#ifndef MOD_MESH
#define MOD_MESH

#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <stdexcept>
#include <stb_image.h>
#include "glad/glad.h"
#include "./util/loadmodel.h"
#include "./util/types.h"
#include "./texture.h"

struct Mesh {
  unsigned int VAOPointer;
  unsigned int EBOPointer;
  unsigned int VBOPointer;
  Texture texture; 
  bool hasEmissionTexture;
  Texture emissionTexture;
  bool hasOpacityTexture;
  Texture opacityTexture;
  long unsigned int numElements;
  BoundInfo boundInfo;
  std::vector<Bone> bones;
};

struct NameAndMesh {
  std::vector<std::reference_wrapper<std::string>> meshNames;
  std::vector<std::reference_wrapper<Mesh>> meshes;
};

void setVertexPosition(Mesh& mesh, unsigned int vertexIndex, glm::vec3 pos);
Mesh loadMesh(std::string defaultTexture, MeshData modelData, std::function<Texture(std::string)> ensureLoadTexture);		 // loads model and returns mesh/bound texture data
Mesh load2DMesh(std::string imagePath, std::function<Texture(std::string)> ensureLoadTexture);		 // loads single quad mesh with texture centered around -1 to 1 x/y
Mesh load2DMeshTexCoords(std::string imagePath, float offsetx, float offsety, float width, float height, std::function<Texture(std::string)> ensureLoadTexture); // 2DMesh with subimage selection
Mesh loadMeshFrom3Vert2TexCoords(std::string imagePath, std::vector<float> vertices, std::vector<unsigned int> indicies, std::function<Texture(std::string)> ensureLoadTexture);
Mesh loadSpriteMesh(std::string imagePath, std::function<Texture(std::string)> ensureLoadTexture);  // loads a 2d mesh with vertex centered around 0 to 1 x/y
void drawMesh(Mesh mesh, GLint shaderProgram, unsigned int customTextureId = -1);  						 // draws mesh and related texture info (no shader data supplied)
void drawLines(std::vector<Line> allLines);

#endif 
