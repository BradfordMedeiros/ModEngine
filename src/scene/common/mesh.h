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
  bool hasDiffuseTexture;
  Texture texture; 
  bool hasEmissionTexture;
  Texture emissionTexture;
  bool hasOpacityTexture;
  Texture opacityTexture;
  bool hasCubemapTexture;
  Texture cubemapTexture;
  bool hasRoughnessTexture;
  Texture roughnessTexture;
  bool hasNormalTexture;
  Texture normalTexture;
  long unsigned int numElements;
  BoundInfo boundInfo;
  std::vector<Bone> bones;
  int numTriangles;
};

struct MeshRef {
  std::set<objid> owners;
  Mesh mesh;
};

struct NameAndMesh {
  std::vector<std::reference_wrapper<std::string>> meshNames;
  std::vector<std::reference_wrapper<Mesh>> meshes;
};

struct NameAndMeshObjName {
  std::vector<std::string> objnames;
  std::vector<std::reference_wrapper<std::string>> meshNames;
  std::vector<std::reference_wrapper<Mesh>> meshes;
};

void setVertexPosition(Mesh& mesh, unsigned int vertexIndex, glm::vec3 pos, glm::vec3 normal);
Mesh loadMesh(std::string defaultTexture, MeshData modelData, std::function<Texture(std::string)> ensureLoadTexture);		 // loads model and returns mesh/bound texture data
Mesh loadSpriteMeshSubimage(std::string imagePath, float offsetx, float offsety, float width, float height, std::function<Texture(std::string)> ensureLoadTexture, bool flipVerticalTexCoords); // 2DMesh with subimage selection
Mesh loadSpriteMesh(std::string imagePath, std::function<Texture(std::string)> ensureLoadTexture);  // loads a 2d mesh with vertex centered around 0 to 1 x/y
void drawMesh(Mesh mesh, GLint shaderProgram, unsigned int customTextureId = -1, unsigned int customOpacityTextureId = -1,  bool drawPoints = false);  						 // draws mesh and related texture info (no shader data supplied)

struct LineRenderData {
  unsigned int VAO;
  unsigned int EBO;
  unsigned int VBO;
  unsigned int numIndices;
};
LineRenderData createLineRenderData(std::vector<Line>& allLines);
int drawLines(std::vector<Line> allLines, int linewidth = 5); // returns # of verts drawn
void freeLineRenderData(LineRenderData& lineData);

Mesh loadSkybox(std::string defaultTexture, std::string skyboxPath, std::string skyboxTexture, std::function<Texture(std::string)> ensureLoadTexture,  std::function<Texture(std::string)> ensureLoadCubemapTexture);
void freeMesh(Mesh& mesh);

// 6 vertices, eg drawarray(6)
unsigned int loadFullscreenQuadVAO();  // TODO - maybe make an unload...but in practice we just load it and keep it so whatever

#endif 
