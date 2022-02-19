#include "./mesh.h"

void setVertexPosition(Mesh& mesh, unsigned int vertexIndex, glm::vec3 pos, glm::vec3 normal){
  glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOPointer);
  glBufferSubData(GL_ARRAY_BUFFER, (sizeof(Vertex) * vertexIndex) + offsetof(Vertex, position), sizeof(pos), &pos);
  glBufferSubData(GL_ARRAY_BUFFER, (sizeof(Vertex) * vertexIndex) + offsetof(Vertex, normal), sizeof(normal), &normal);
}

Mesh loadMesh(std::string defaultTexture, MeshData meshData, std::function<Texture(std::string)> ensureLoadTexture){
  assert((meshData.indices.size() % 3) == 0);
  auto numTriangles = meshData.indices.size() / 3;

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO); 

  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * meshData.indices.size(), &(meshData.indices[0]), GL_STATIC_DRAW);

  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * meshData.vertices.size(), &(meshData.vertices[0]), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, position));
 
  Texture texture;
  if (meshData.hasDiffuseTexture){
    texture = ensureLoadTexture(meshData.diffuseTexturePath); 
  }else{
    texture = ensureLoadTexture(defaultTexture); 
  }


  Texture emission;
  if (meshData.hasEmissionTexture){
    emission = ensureLoadTexture(meshData.emissionTexturePath);
  }

  Texture opacity;
  if (meshData.hasOpacityTexture){
    opacity = ensureLoadTexture(meshData.opacityTexturePath);
  }

  Texture roughness;
  if (meshData.hasRoughnessTexture){
    roughness = ensureLoadTexture(meshData.roughnessTexturePath);
  }

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoords));

  for (auto vertex : meshData.vertices){
    for (int i = 0; i < NUM_BONES_PER_VERTEX; i++){
      if (vertex.boneIndexes[i] > 50){
        std::cout << "value is: " << vertex.boneIndexes[i] << std::endl;
        assert(false);
      }
     }
  }

  // this is directly coupled to :  ./util/loadmodel.h Vertex struct definition
  for (int i = 0; i < NUM_BONES_PER_VERTEX; i++){
    glEnableVertexAttribArray(3 + i);
    glVertexAttribIPointer(3 + i, 1, GL_BYTE, sizeof(Vertex), (void*) (offsetof(Vertex, boneIndexes) + (sizeof(int32_t) * i)));    

    glEnableVertexAttribArray(3 + NUM_BONES_PER_VERTEX + i);
    glVertexAttribPointer(3 + NUM_BONES_PER_VERTEX + i, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (offsetof(Vertex, boneWeights) + (sizeof(float) * i)));
  }

  
  Mesh mesh = {
    .VAOPointer = VAO,
    .EBOPointer = EBO,
    .VBOPointer = VBO,
    .hasDiffuseTexture = meshData.hasDiffuseTexture,
    .texture = texture,
    .hasEmissionTexture = meshData.hasEmissionTexture,
    .emissionTexture = emission,
    .hasOpacityTexture = meshData.hasOpacityTexture,
    .opacityTexture = opacity,
    .hasCubemapTexture = false,
    .hasRoughnessTexture = meshData.hasRoughnessTexture,
    .roughnessTexture = roughness,
    .numElements = meshData.indices.size(),
    .boundInfo = meshData.boundInfo,
    .bones = meshData.bones,
    .numTriangles = numTriangles,
  }; 

  return mesh; 
}

Mesh load2DMesh(std::string imagePath, float vertices[], unsigned int indices[], 
  unsigned int dataSize, unsigned int numIndices, unsigned int vertexWidth, unsigned int textureWidth, std::function<Texture(std::string)> ensureLoadTexture){

  unsigned int bufferWidth = vertexWidth + textureWidth;
  Texture texture = ensureLoadTexture(imagePath);

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

  glVertexAttribPointer(2, textureWidth, GL_FLOAT, GL_FALSE, sizeof(float) *  bufferWidth, (void*)(sizeof(float) * vertexWidth));
  glEnableVertexAttribArray(2);
 

  BoundInfo boundInfo { };
  std::vector<Bone> bones;
  Mesh mesh = {
    .VAOPointer = VAO,
    .EBOPointer = EBO,
    .VBOPointer = VBO,
    .hasDiffuseTexture = true,
    .texture = texture,
    .hasEmissionTexture = false,
    .hasOpacityTexture = false,
    .hasCubemapTexture = false,
    .hasRoughnessTexture = false,
    .numElements = numIndices,
    .boundInfo = boundInfo,
    .bones = bones,
    .numTriangles = numIndices / 3,
  };

  return mesh;
}

// This orients the text correctly, but the 2d text rendering isnt yet setup for this
Mesh loadSpriteMeshSubimage(std::string imagePath, float offsetxndi, float offsetyndi, float widthndi, float heightndi, std::function<Texture(std::string)> ensureLoadTexture){
  float verts[] = {
    // -1 in the tex coords since the image loaded are flipped on the vertical + reversed in the shader
    // Implementation detail of data loaded for texture loading. 
    -1.0f,  1.0f, 0.0f,  offsetxndi, -1 * offsetyndi,
    -1.0f, -1.0f, 0.0f,  offsetxndi, -1 * (offsetyndi + heightndi), 
    1.0f, -1.0f, 0.0f,   offsetxndi + widthndi, -1 * (offsetyndi + heightndi),
    1.0f,  1.0f, 0.0f,   offsetxndi + widthndi, -1 * offsetyndi,
  };
  unsigned int indices[] = {0, 1, 2, 0, 2, 3};
  return load2DMesh(imagePath, verts, indices, 20, 6, 3, 2, ensureLoadTexture);
}

Mesh loadSpriteMesh(std::string imagePath, std::function<Texture(std::string)> ensureLoadTexture){
  float verts[] = {
    // See above note to explain the -1 in y
    0.0f,  1.0f, 0.0f,  0.0f, -1.0f,
    0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,  1.0f, -1.0f,
  };
  unsigned int indices[] = {2, 1, 0, 3, 2, 0};
  return load2DMesh(imagePath, verts, indices, 20, 6, 3, 2, ensureLoadTexture);  
}

void drawMesh(Mesh mesh, GLint shaderProgram, unsigned int customTextureId, unsigned int customOpacityTextureId,  bool drawPoints){
  glBindVertexArray(mesh.VAOPointer);
 
  glActiveTexture(GL_TEXTURE0); 

  auto diffuseTextureId = customTextureId == -1 ? mesh.texture.textureId : customTextureId;
  auto hasDiffuseTexture = customTextureId != -1 || mesh.hasDiffuseTexture; 
  glUniform1i(glGetUniformLocation(shaderProgram, "hasDiffuseTexture"), hasDiffuseTexture);
  glBindTexture(GL_TEXTURE_2D, diffuseTextureId);
 
  glUniform1i(glGetUniformLocation(shaderProgram, "hasEmissionTexture"), mesh.hasEmissionTexture);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, mesh.emissionTexture.textureId);

  auto opacityTextureId = customOpacityTextureId == -1 ? mesh.opacityTexture.textureId : customOpacityTextureId;
  bool hasOpacityTexture = mesh.hasOpacityTexture || (customOpacityTextureId != -1);
  glUniform1i(glGetUniformLocation(shaderProgram, "hasOpacityTexture"), hasOpacityTexture);
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, opacityTextureId);

  glUniform1i(glGetUniformLocation(shaderProgram, "hasCubemapTexture"), mesh.hasCubemapTexture);
  glActiveTexture(GL_TEXTURE0 + 4);
  glBindTexture(GL_TEXTURE_CUBE_MAP, mesh.cubemapTexture.textureId);

  glUniform1i(glGetUniformLocation(shaderProgram, "hasRoughnessTexture"), mesh.hasRoughnessTexture);
  glActiveTexture(GL_TEXTURE0 + 5);
  glBindTexture(GL_TEXTURE_2D, mesh.roughnessTexture.textureId);

  glActiveTexture(GL_TEXTURE0); 
  glDrawElements(GL_TRIANGLES, mesh.numElements, GL_UNSIGNED_INT, 0);

  if (drawPoints){
    glDrawElements(GL_POINTS, mesh.numElements, GL_UNSIGNED_INT, 0);   
  }
}

// returns # of verts drawn
int drawLines(std::vector<Line> allLines){
  std::vector<glm::vec3> lines;
  for (Line line : allLines){
    lines.push_back(line.fromPos);
    lines.push_back(line.toPos);
  }

  std::vector<unsigned int> indicies;
  for (unsigned int i = 0; i < lines.size(); i++){
    indicies.push_back(i);
  }

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indicies.size(), &(indicies[0]), GL_STATIC_DRAW);

  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * lines.size(), &(lines[0]), GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
  glEnableVertexAttribArray(0);
  glLineWidth(5);
  glDrawElements(GL_LINES, indicies.size() , GL_UNSIGNED_INT, 0);

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &EBO);
  glDeleteBuffers(1, &VBO);
  return indicies.size();
}

Mesh loadSkybox(std::string defaultTexture, std::string skyboxPath, std::string skyboxTexture, std::function<Texture(std::string)> ensureLoadTexture,  std::function<Texture(std::string)> ensureLoadCubemapTexture){
  ModelData data = loadModel("", skyboxPath);
  assert(data.meshIdToMeshData.size() == 1);
  MeshData meshData = data.meshIdToMeshData.begin() -> second;
  Mesh mesh = loadMesh(defaultTexture, meshData, ensureLoadTexture);
  mesh.hasCubemapTexture = skyboxTexture != "";
  if (skyboxTexture != ""){
    mesh.cubemapTexture = ensureLoadCubemapTexture(skyboxTexture);
  }
  return mesh;
}

void freeMesh(Mesh& mesh){
  std::cout << "FREEING MESH" << std::endl;
  glDeleteVertexArrays(1, &mesh.VAOPointer);
  GLuint buffersToDelete[2] = { mesh.EBOPointer, mesh.VBOPointer };
  glDeleteBuffers(2, buffersToDelete);
}

unsigned int loadFullscreenQuadVAO(){
  static float quadVertices[] = {
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f,  0.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f
  };
  unsigned int quadVAO;
  unsigned int quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0); 
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  return quadVAO;
}