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
#include "../../perf/benchstats.h"

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
  int numVertices;  // if this is < 0, invalid don't depend on
  int numIndices;   // if this is < 0, invalid don't depend on
  //std::vector<Vertex> debugVertexs;
};

struct MeshRef {
  std::set<objid> owners;
  std::set<std::string> textureRefs;
  Mesh mesh;
};

struct NameAndMeshObjName {
  std::string objname;
  std::string* meshname;
  Mesh* mesh;
};

void setVertexPosition(Mesh& mesh, unsigned int vertexIndex, glm::vec3 pos, glm::vec3 normal);
Mesh loadMesh(std::string defaultTexture, MeshData modelData, std::function<Texture(std::string)> ensureLoadTexture);		 // loads model and returns mesh/bound texture data
Mesh loadSpriteMeshSubimage(std::string imagePath, float offsetx, float offsety, float width, float height, std::function<Texture(std::string)> ensureLoadTexture, bool flipVerticalTexCoords); // 2DMesh with subimage selection
Mesh loadSpriteMesh(std::string imagePath, std::function<Texture(std::string)> ensureLoadTexture);  // loads a 2d mesh with vertex centered around 0 to 1 x/y

struct MeshUniforms {
  glm::mat4 model;
};
void drawMesh(Mesh mesh, GLint shaderProgram, unsigned int customTextureId, unsigned int customOpacityTextureId, bool drawPoints, unsigned int customNormalTextureId, MeshUniforms meshUniforms);

struct LineRenderData {
  unsigned int VAO;
  unsigned int EBO;
  unsigned int VBO;
  unsigned int numIndices;
};
LineRenderData createLineRenderData(std::vector<Line>& allLines);
int drawLines(GLint shaderProgram, std::vector<Line> allLines, int linewidth, glm::mat4& model); // returns # of verts drawn
void freeLineRenderData(LineRenderData& lineData);

Mesh loadSkybox(std::string defaultTexture, std::string skyboxPath, std::string skyboxTexture, std::function<Texture(std::string)> ensureLoadTexture,  std::function<Texture(std::string)> ensureLoadCubemapTexture);
void freeMesh(Mesh& mesh);

// 6 vertices, eg drawarray(6)
unsigned int loadFullscreenQuadVAO();  // TODO - maybe make an unload...but in practice we just load it and keep it so whatever
unsigned int loadFullscreenQuadVAO3D();

std::vector<Vertex> readVertsFromMeshVao(Mesh& mesh);

std::string print(Mesh& mesh);

#endif 
