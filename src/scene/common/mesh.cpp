#include "./mesh.h"

// Generating the VAO per model is probaby not the most efficient, but figure that this is 
// a clean abstraction, and we can optimize this fucker after we get more features in it.
Mesh loadMesh(std::string defaultTexture, MeshData meshData, std::function<Texture(std::string)> ensureLoadTexture){
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
 
  // @TODO texture loading can be optimized when textures are shared between objects, right now places each meshs texture in memory redundantly. right now this is super, super unoptimized.
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
    glVertexAttribIPointer(3 + i, 1, GL_BYTE, sizeof(Vertex), (void*) (offsetof(Vertex, boneIndexes) + (sizeof(short) * i)));    

    glEnableVertexAttribArray(3 + NUM_BONES_PER_VERTEX + i);
    glVertexAttribPointer(3 + NUM_BONES_PER_VERTEX + i, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (offsetof(Vertex, boneWeights) + (sizeof(float) * i)));
  }

  
  Mesh mesh = {
    .VAOPointer = VAO,
    .VBOPointer = VBO,
    .texture = texture,
    .hasEmissionTexture = meshData.hasEmissionTexture,
    .emissionTexture = emission,
    .hasOpacityTexture = meshData.hasOpacityTexture,
    .opacityTexture = opacity,
    .numElements = meshData.indices.size(),
    .boundInfo = meshData.boundInfo,
    .bones = meshData.bones,
  }; 

  return mesh; 
}

Mesh load2DMeshHelper(std::string imagePath, float vertices[], unsigned int indices[], 
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

  glVertexAttribPointer(1, textureWidth, GL_FLOAT, GL_FALSE, sizeof(float) *  bufferWidth, (void*)(sizeof(float) * vertexWidth));
  glEnableVertexAttribArray(1);
 
  BoundInfo boundInfo { };
  std::vector<Bone> bones;
  Mesh mesh = {
    .VAOPointer = VAO,
    .VBOPointer = VBO,
    .texture = texture,
    .hasEmissionTexture = false,
    .hasOpacityTexture = false,
    .numElements = numIndices,
    .boundInfo = boundInfo,
    .bones = bones,
  };

  return mesh;
}
Mesh load2DMesh(std::string imagePath, std::function<Texture(std::string)> ensureLoadTexture){
  float quadVerts[] = {
    -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
    1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
  };
  unsigned int indices[] = {0, 1, 2, 0, 2, 3};
  return load2DMeshHelper(imagePath, quadVerts, indices, 20, 6, 3, 2, ensureLoadTexture);  // @TODO last 4 nums seem derivable in load2dmesh helper
}
Mesh load2DMeshTexCoords(std::string imagePath, float offsetxndi, float offsetyndi, float widthndi, float heightndi, std::function<Texture(std::string)> ensureLoadTexture){
  float quadVerts[] = {
    -1.0f,  1.0f, 0.0f,  offsetxndi, offsetyndi + heightndi,
    -1.0f, -1.0f, 0.0f,  offsetxndi, offsetyndi,
    1.0f, -1.0f, 0.0f,   offsetxndi + widthndi, offsetyndi,
    1.0f,  1.0f, 0.0f,   offsetxndi + widthndi, offsetyndi + heightndi,
  };
  unsigned int indices[] = {0, 1, 2, 0, 2, 3};
  return load2DMeshHelper(imagePath, quadVerts, indices, 20, 6, 3, 2, ensureLoadTexture);
}
Mesh loadMeshFrom3Vert2TexCoords(std::string imagePath, std::vector<float> vertices, std::vector<unsigned int> indicies, std::function<Texture(std::string)> ensureLoadTexture){
  return load2DMeshHelper(imagePath, &(vertices[0]), &indicies[0], vertices.size(), indicies.size(), 3, 2, ensureLoadTexture);
}

Mesh loadSpriteMesh(std::string imagePath, std::function<Texture(std::string)> ensureLoadTexture){
  float uiVerts[] = {
    0.0f,  1.0f, 0.0f,  0.0f, 1.0f,
    0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
  };
  unsigned int indices[] = {0, 1, 2, 0, 2, 3};
  return load2DMeshHelper(imagePath, uiVerts, indices, 20, 6, 3, 2, ensureLoadTexture);  
}

void drawMesh(Mesh mesh, GLint shaderProgram, unsigned int customTextureId){
  glBindVertexArray(mesh.VAOPointer);
 
  glActiveTexture(GL_TEXTURE0); 

  auto diffuseTextureId = customTextureId == -1 ? mesh.texture.textureId : customTextureId;
  glBindTexture(GL_TEXTURE_2D, diffuseTextureId);
 
  glUniform1i(glGetUniformLocation(shaderProgram, "hasEmissionTexture"), mesh.hasEmissionTexture);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, mesh.emissionTexture.textureId);

  glUniform1i(glGetUniformLocation(shaderProgram, "hasOpacityTexture"), mesh.hasOpacityTexture);
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, mesh.opacityTexture.textureId);

  glActiveTexture(GL_TEXTURE0); 
  glDrawElements(GL_TRIANGLES, mesh.numElements, GL_UNSIGNED_INT, 0);
}

// @TODO [memory leak] - this function is okay, but it's usage is bad.  Currently things will call this and just let memory go.
// That's a hardcore memory leak if there is one to talk about.  Ok since this is used in debugging, but maybe just free it after the call 
// or something
void drawLines(std::vector<Line> allLines){
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
  glLineWidth(2);
  glDrawElements(GL_LINES, indicies.size() , GL_UNSIGNED_INT, 0);
}