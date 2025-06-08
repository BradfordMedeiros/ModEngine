#include "./mesh.h"

extern Stats statistics;
extern std::unordered_map<unsigned int, std::vector<ShaderTextureBinding>> textureBindings; 

void shaderSetUniform(unsigned int shaderToUse, const char* name, glm::mat4&& value);
void shaderSetUniform(unsigned int shaderToUse, const char* name, glm::mat4& value);
void shaderSetUniform(unsigned int shaderToUse, const char* name, glm::vec3& value);
void shaderSetUniform(unsigned int shaderToUse, const char* name, glm::vec3&& value);
void shaderSetUniform(unsigned int shaderToUse, const char* name, glm::vec2& value);
void shaderSetUniform(unsigned int shaderToUse, const char* name, glm::vec4& value);
void shaderSetUniform(unsigned int shaderToUse, const char* name, bool value);
glm::vec4 getColorFromGameobject(objid id);


int numberOfDrawCallsThisFrame = 0;  // static-state

void setVertexPosition(Mesh& mesh, unsigned int vertexIndex, glm::vec3 pos, glm::vec3 normal){
  glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOPointer);
  glBufferSubData(GL_ARRAY_BUFFER, (sizeof(Vertex) * vertexIndex) + offsetof(Vertex, position), sizeof(pos), &pos);
  glBufferSubData(GL_ARRAY_BUFFER, (sizeof(Vertex) * vertexIndex) + offsetof(Vertex, normal), sizeof(normal), &normal);
}

Mesh loadMesh(std::string defaultTexture, MeshData meshData, std::function<Texture(std::string)> ensureLoadTexture){
  registerStat(statistics.loadMeshStat, 1);

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
    std::cout << "ensure load texture dif: " << meshData.diffuseTexturePath << std::endl;
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

  Texture normal;
  if (meshData.hasNormalTexture){
    normal = ensureLoadTexture(meshData.normalTexturePath);
  }

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, normal));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, texCoords));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, tangent));

  for (auto vertex : meshData.vertices){
    for (int i = 0; i < NUM_BONES_PER_VERTEX; i++){
      if (vertex.boneIndexes[i] >= 100){
        std::cout << "value is: " << vertex.boneIndexes[i] << std::endl;
        assert(false);
      }
     }
  }

  // this is directly coupled to :  ./util/loadmodel.h Vertex struct definition
  for (int i = 0; i < NUM_BONES_PER_VERTEX; i++){
    glEnableVertexAttribArray(4 + i);
    glVertexAttribIPointer(4 + i, 1, GL_BYTE, sizeof(Vertex), (void*) (offsetof(Vertex, boneIndexes) + (sizeof(int32_t) * i)));    

    glEnableVertexAttribArray(4 + NUM_BONES_PER_VERTEX + i);
    glVertexAttribPointer(4 + NUM_BONES_PER_VERTEX + i, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) (offsetof(Vertex, boneWeights) + (sizeof(float) * i)));
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
    .hasNormalTexture = meshData.hasNormalTexture,
    .normalTexture = normal,
    .numElements = meshData.indices.size(),
    .boundInfo = meshData.boundInfo,
    .bones = meshData.bones,
    .numTriangles = numTriangles,
    .numVertices = meshData.vertices.size(),
    .numIndices = meshData.indices.size(),
    //.debugVertexs = meshData.vertices,
  }; 

  return mesh; 
}

Mesh load2DMesh(std::string imagePath, float vertices[], unsigned int indices[], 
  unsigned int dataSize, unsigned int numIndices, unsigned int vertexWidth, unsigned int textureWidth, std::function<Texture(std::string)> ensureLoadTexture, bool flipVerticalTexCoords){

  Texture texture = ensureLoadTexture(imagePath);
  unsigned int bufferWidth = vertexWidth + textureWidth;

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
    .hasNormalTexture = false,
    .numElements = numIndices,
    .boundInfo = boundInfo,
    .bones = bones,
    .numTriangles = numIndices / 3,
    .numVertices = -1,
    .numIndices = -1,
    //.debugVertexs = {},
  };

  return mesh;
}

// This orients the text correctly, but the 2d text rendering isnt yet setup for this
Mesh loadSpriteMeshSubimage(std::string imagePath, float offsetxndi, float offsetyndi, float widthndi, float heightndi, std::function<Texture(std::string)> ensureLoadTexture, bool flipVerticalTexCoords){
  float verticalMulitiplier = flipVerticalTexCoords ? -1.f : 1.f;
  float verts[] = {
    -1.0f,  1.0f, 0.0f,  offsetxndi, verticalMulitiplier * offsetyndi,
    -1.0f, -1.0f, 0.0f,  offsetxndi, verticalMulitiplier * (offsetyndi + heightndi), 
    1.0f, -1.0f, 0.0f,   offsetxndi + widthndi, verticalMulitiplier * (offsetyndi + heightndi),
    1.0f,  1.0f, 0.0f,   offsetxndi + widthndi, verticalMulitiplier * offsetyndi,
  };
  unsigned int indices[] = {0, 1, 2, 0, 2, 3};
  return load2DMesh(imagePath, verts, indices, 20, 6, 3, 2, ensureLoadTexture, flipVerticalTexCoords);
}

Mesh loadSpriteMesh(std::string imagePath, std::function<Texture(std::string)> ensureLoadTexture){
  return loadSpriteMeshSubimage(imagePath, 0, 0, 1, 1, ensureLoadTexture, false);
}


// TODO This is intended for the default shader
// in practice this gets called for other shaders too 
// should just create another functon to handle the ui shader

void drawMesh(Mesh& mesh, GLint shaderProgram, bool drawPoints, MeshUniforms& meshUniforms){
  //meshUniforms.useInstancing = true;

  shaderSetUniform(shaderProgram, "model", meshUniforms.model);
  glProgramUniform3fv(shaderProgram, glGetUniformLocation(shaderProgram, "emissionAmount"), 1, glm::value_ptr(meshUniforms.emissionAmount));
  glProgramUniform2fv(shaderProgram, glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(meshUniforms.textureSize));
  glProgramUniform2fv(shaderProgram, glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(meshUniforms.textureTiling));
  glProgramUniform2fv(shaderProgram, glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(meshUniforms.textureOffset));
  shaderSetUniform(shaderProgram, "tint", meshUniforms.tint);
  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "forceTint"), false);

  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "useInstancing"), meshUniforms.useInstancing);


  glProgramUniform4fv(shaderProgram, glGetUniformLocation(shaderProgram, "encodedid"), 1, glm::value_ptr(getColorFromGameobject(meshUniforms.id)));


  /*
            auto color = getColorFromGameobject(selectionId);
          Color colorTypeColor {
            .r = color.x,
            .g = color.y, 
            .b = color.z,
            .a = color.w,
          };
          auto restoredId = getIdFromColor(colorTypeColor);
          */

  bool hasBones = !((meshUniforms.bones == NULL) || (meshUniforms.bones -> size() == 0));
  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "hasBones"), hasBones);

  if (hasBones){
    for (int i = 0; i < meshUniforms.bones -> size(); i++){
      auto boneUniformLocation = glGetUniformLocation(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str());
      shaderSetUniform(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str(), meshUniforms.bones -> at(i).offsetMatrix);
    }
    shaderSetUniform(shaderProgram, "groupToModel", meshUniforms.bonesGroupModel);
  }


  glBindVertexArray(mesh.VAOPointer);
 
  glActiveTexture(GL_TEXTURE0); 

  auto diffuseTextureId = meshUniforms.customTextureId == -1 ? mesh.texture.textureId : meshUniforms.customTextureId;
  auto hasDiffuseTexture = meshUniforms.customTextureId != -1 || mesh.hasDiffuseTexture; 
  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "hasDiffuseTexture"), hasDiffuseTexture);
  glBindTexture(GL_TEXTURE_2D, diffuseTextureId);
 
  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "textureid"), diffuseTextureId);

  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "hasEmissionTexture"), mesh.hasEmissionTexture);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, mesh.emissionTexture.textureId);

  auto opacityTextureId = meshUniforms.customOpacityTextureId == -1 ? mesh.opacityTexture.textureId : meshUniforms.customOpacityTextureId;
  bool hasOpacityTexture = mesh.hasOpacityTexture || (meshUniforms.customOpacityTextureId != -1);
  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "hasOpacityTexture"), hasOpacityTexture);
  glActiveTexture(GL_TEXTURE0 + 2);
  glBindTexture(GL_TEXTURE_2D, opacityTextureId);

  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "hasCubemapTexture"), mesh.hasCubemapTexture);
  glActiveTexture(GL_TEXTURE0 + 4);
  glBindTexture(GL_TEXTURE_CUBE_MAP, mesh.cubemapTexture.textureId);

  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "hasRoughnessTexture"), mesh.hasRoughnessTexture);
  glActiveTexture(GL_TEXTURE0 + 5);
  glBindTexture(GL_TEXTURE_2D, mesh.roughnessTexture.textureId);

  auto normalTextureId = meshUniforms.customNormalTextureId == -1 ? mesh.normalTexture.textureId : meshUniforms.customNormalTextureId;
  bool hasNormalTexture = mesh.hasNormalTexture || (meshUniforms.customNormalTextureId != -1);
  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "hasNormalTexture"), hasNormalTexture);
  glActiveTexture(GL_TEXTURE0 + 6);
  glBindTexture(GL_TEXTURE_2D, normalTextureId);

  glActiveTexture(GL_TEXTURE0); 

  if (textureBindings.find(shaderProgram) != textureBindings.end()){
    auto& texInfo = textureBindings.at(shaderProgram);
    for (auto &texBinding : texInfo){
      glActiveTexture(GL_TEXTURE0 + texBinding.textureUnit);
      glBindTexture(GL_TEXTURE_2D, texBinding.textureId);
    }
  }


  if (meshUniforms.useInstancing){
    int numInstances = 5;
    for (int i = 0; i < numInstances; i++){
      glProgramUniform3fv(shaderProgram, glGetUniformLocation(shaderProgram, ("instanceOffsets[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(glm::vec3(i * 5.f, i * 5.f, i * 5.f)));
    }    

    glDrawElementsInstanced(GL_TRIANGLES, mesh.numElements, GL_UNSIGNED_INT, 0, 1000);
  }else{
    glDrawElements(GL_TRIANGLES, mesh.numElements, GL_UNSIGNED_INT, 0);
  }
  numberOfDrawCallsThisFrame++;

  if (drawPoints){
    glDrawElements(GL_POINTS, mesh.numElements, GL_UNSIGNED_INT, 0);
    numberOfDrawCallsThisFrame++;
  }
}

LineRenderData createLineRenderData(std::vector<Line>& allLines){
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

  return LineRenderData {
    .VAO = VAO,
    .EBO = EBO,
    .VBO = VBO,
    .numIndices = indicies.size(),
  };
}
void freeLineRenderData(LineRenderData& lineData){
  glDeleteVertexArrays(1, &lineData.VAO);
  glDeleteBuffers(1, &lineData.EBO);
  glDeleteBuffers(1, &lineData.VBO); 
}

// returns # of verts drawn
int drawLines(GLint shaderProgram, std::vector<Line> allLines, int linewidth, glm::mat4& model, glm::vec4 tint){
  shaderSetUniform(shaderProgram, "model", model);
  shaderSetUniform(shaderProgram, "tint", tint);
  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "forceTint"), true);
  glProgramUniform4fv(shaderProgram, glGetUniformLocation(shaderProgram, "encodedid"), 1, glm::value_ptr(getColorFromGameobject(0)));
  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "hasBones"), false);
  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "hasCubemapTexture"), false);
  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "hasDiffuseTexture"), false);
  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "hasEmissionTexture"), false);
  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "hasNormalTexture"), false);
  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "hasRoughnessTexture"), false);
  glProgramUniform3fv(shaderProgram, glGetUniformLocation(shaderProgram, "emissionAmount"), 1, glm::value_ptr(glm::vec3(0.f, 0.f, 0.f)));
  glProgramUniform2fv(shaderProgram, glGetUniformLocation(shaderProgram, "textureSize"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
  glProgramUniform2fv(shaderProgram, glGetUniformLocation(shaderProgram, "textureTiling"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));
  glProgramUniform2fv(shaderProgram, glGetUniformLocation(shaderProgram, "textureOffset"), 1, glm::value_ptr(glm::vec2(1.f, 1.f)));

  glProgramUniform1i(shaderProgram, glGetUniformLocation(shaderProgram, "useInstancing"), false);

  modassert(allLines.size() > 0, "draw lines - all lines must be non-zero size");
  auto lineData = createLineRenderData(allLines);
  glLineWidth(linewidth);
  glDrawElements(GL_LINES, lineData.numIndices , GL_UNSIGNED_INT, 0);


  numberOfDrawCallsThisFrame++;
  freeLineRenderData(lineData);
  return lineData.numIndices;
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

unsigned int loadFullscreenQuadVAO3D(){
  static float quadVertices[] = {
    -1.0f,  1.0f,  0.f, 0.0f, 1.0f,
    -1.0f, -1.0f,  0.f, 0.0f, 0.0f,
     1.0f, -1.0f,  0.f, 1.0f, 0.0f,
    -1.0f,  1.0f,  0.f, 0.0f, 1.0f,
    1.0f, -1.0f,  0.f, 1.0f, 0.0f,
    1.0f,  1.0f,  0.f, 1.0f, 1.0f
  };
  unsigned int quadVAO;
  unsigned int quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0); 
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  
  /// Todo this should be normals, not a repeat of the texcoords
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));


  return quadVAO;
}

std::vector<Vertex> readVertsFromMeshVao(Mesh& mesh){
  modlog("read verts", std::string("num verts = ") + std::to_string(mesh.numVertices) + ", num indicies = " + std::to_string(mesh.numIndices));
  if (mesh.numVertices <= 0 || mesh.numIndices <= 0){
    modassert(false, "no vertices in mesh vao");
    return {};
  }

  glBindVertexArray(mesh.VAOPointer); 

  std::vector<Vertex> vertices;
  vertices.resize(mesh.numVertices);

  //std::cout << "num vertices read: " << mesh.numVertices << std::endl;

  glBindBuffer(GL_ARRAY_BUFFER, mesh.VBOPointer);
  glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertex) * mesh.numVertices, &(vertices.at(0)));

  std::vector<unsigned int> indices;
  indices.resize(mesh.numIndices);

  std::cout << "read verts debug: " << print(mesh) << std::endl;
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBOPointer);
  glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int) * mesh.numIndices, &(indices.at(0)));

  std::vector<Vertex> vertexs;
  for (auto index : indices){
    vertexs.push_back(vertices.at(index));
  }

  //std::cout << "readVertsFromMeshVao  - " << vertices.size() << ", " << indices.size() << " [" << std::endl;
  //for (auto index : indices){
  //  std::cout << "index: " << index << " -- " << print(vertices.at(index)) << std::endl;
  //}
  //std::cout << "]" << std::endl;
  modassert(vertexs.size() % 3 == 0, "vertices must be a multiple of 3");
  return vertexs;
}

std::string print(Mesh& mesh){
  std::string value = "";
  value += "vbo = " + std::to_string(mesh.VBOPointer) + "\n";
  value += "ebo = " + std::to_string(mesh.EBOPointer) + "\n";
  value += "numVertices = " + std::to_string(mesh.numVertices) + "\n";
  value += "numIndices = " + std::to_string(mesh.numIndices) + "\n";
  value += "numElements = " + std::to_string(mesh.numElements) + "\n";
  value += "bones.size = " + std::to_string(mesh.bones.size()) + "\n";
  value += "hasDiffuseTexture = " + print(mesh.hasDiffuseTexture) + "\n";
  value += "hasEmissionTexture = " + print(mesh.hasEmissionTexture) + "\n";
  value += "hasOpacityTexture = " + print(mesh.hasOpacityTexture) + "\n";
  value += "hasCubemapTexture = " + print(mesh.hasCubemapTexture) + "\n";
  value += "hasRoughnessTexture = " + print(mesh.hasRoughnessTexture) + "\n";
  value += "hasNormalTexture = " + print(mesh.hasNormalTexture) + "\n";

  return value;
}