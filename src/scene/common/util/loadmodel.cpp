#include "./loadmodel.h"

BoundInfo getBounds(std::vector<Vertex>& vertices){
  float xMin, xMax;
  float yMin, yMax;
  float zMin, zMax;

  xMin = vertices[0].position.x;
  xMax = vertices[0].position.x;
  yMin = vertices[0].position.y;
  yMax = vertices[0].position.y;
  zMin = vertices[0].position.z;
  zMax = vertices[0].position.z;

  for (Vertex vert: vertices){
    if (vert.position.x > xMax){
      xMax = vert.position.x;
    }
    if (vert.position.x < xMin){
      xMin = vert.position.x;
    }
    if (vert.position.y > yMax){
      yMax = vert.position.y;
    }
    if (vert.position.y < yMin){
      yMin = vert.position.y;
    }
    if (vert.position.z > zMax){
      zMax = vert.position.z;
    }
    if (vert.position.z < zMin){
      zMin = vert.position.z;
    }
  }

  BoundInfo info = {
    .xMin = xMin, .xMax = xMax,
    .yMin = yMin, .yMax = yMax,
    .zMin = zMin, .zMax = zMax,
  };
  return info;
}

ModelData processMesh(aiMesh* mesh, const aiScene* scene, std::string modelPath){
   std::vector<Vertex> vertices;
   std::vector<unsigned int> indices;
   
   for (unsigned int i = 0; i < mesh->mNumVertices; i++){
     Vertex vertex;
     vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
     vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

     // load one layer of texture coordinates for now
     if (!mesh->mTextureCoords[0]){
        continue;
     }
     vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);  // Maybe warn here is no texcoords but no materials ? 
     vertices.push_back(vertex);
   } 
 
   for (unsigned int i = 0; i < mesh->mNumFaces; i++){
     aiFace face = mesh->mFaces[i];
     for (unsigned int j = 0; j < face.mNumIndices; j++){
       indices.push_back(face.mIndices[j]);
     }
   }

   // Eventually this should support more than just diffuse maps.
   aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
   
   std::vector<std::string> textureFilepaths;
   for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++){
     aiString texturePath;
     material->GetTexture(aiTextureType_DIFFUSE, i, &texturePath);
     std::filesystem::path modellocation = std::filesystem::canonical(modelPath).parent_path();
     std::filesystem::path relativePath = std::filesystem::weakly_canonical(modellocation / texturePath.C_Str()); //  / is append operator 
     textureFilepaths.push_back(relativePath.string());  
   }

   ModelData model = {
     .vertices = vertices,
     .indices = indices,       
     .texturePaths = textureFilepaths,
     .boundInfo = getBounds(vertices),
   };

   return model;
}

void processNode(aiNode* node, const aiScene* scene, std::string modelPath, std::function<void(ModelData)> onLoadMesh){
   for (unsigned int i = 0; i < node->mNumMeshes; i++){
     ModelData meshData = processMesh(scene->mMeshes[node->mMeshes[i]], scene, modelPath);
     onLoadMesh(meshData);
   }
   for (unsigned int i = 0; i < node ->mNumChildren; i++){
     processNode(node->mChildren[i], scene, modelPath, onLoadMesh);
   }
}

// Currently this just loads all the meshes into the models array. 
// Should have parent/child relations and a hierarchy but todo.
std::vector<ModelData> loadModel(std::string modelPath){
   Assimp::Importer import;
   const aiScene* scene = import.ReadFile(modelPath, aiProcess_Triangulate);
   if (!scene || scene->mFlags && AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
      std::cerr << "error loading model" << std::endl;
      throw std::runtime_error("Error loading mode: does the file " + modelPath + " exist?");
   } 

   std::vector<ModelData> models;
   processNode(scene->mRootNode, scene, modelPath, [&models](ModelData meshdata) -> void {
     models.push_back(meshdata);
   });

   return models;
}
