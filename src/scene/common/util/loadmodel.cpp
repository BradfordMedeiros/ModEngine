#include "./loadmodel.h"

BoundInfo getBounds(std::vector<Vertex>& vertices){
  float xMin, xMax;
  float yMin, yMax;
  float zMin, zMax;

  xMin = vertices[0].position.x;    // @todo zero vertex model --> which is fucking stupid but correct, will error here.
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
    .isNotCentered = false
  };
  return info;
}

void processAnimations(const aiScene* scene){
  int numAnimations = scene -> mNumAnimations;
  std::cout << "num animations: " << numAnimations << std::endl;
  for (int i = 0; i < numAnimations; i++){
    aiAnimation* animation = scene -> mAnimations[i];
    std::cout << "animation name is: " << animation -> mName.C_Str() << std::endl;
    std::cout << "ticks per seconds: " << animation -> mTicksPerSecond << std::endl;
    std::cout << "duration is: " << animation -> mDuration << std::endl;

    for (int j = 0; j < animation -> mNumChannels; j++){
      // http://assimp.sourceforge.net/lib_html/structai_node_anim.html
      // http://ogldev.atspace.co.uk/www/tutorial38/tutorial38.html

      // what about mesh animations ? http://assimp.sourceforge.net/lib_html/structai_mesh_anim.html
      // I think that's not skeletal animation, but rather a bunch of meshes, but im not sure need to check
      aiNodeAnim* aiAnimation = animation-> mChannels[j];  
      std::cout << "affected node name: " << aiAnimation -> mNodeName.C_Str() << std::endl;  // http://assimp.sourceforge.net/lib_html/structai_node_anim.html

    }

  }
}
void processBones(aiMesh* mesh){
  int numBones = mesh -> mNumBones;
  aiBone** bones = mesh -> mBones;
  for (int i = 0; i < numBones; i++){
    aiBone* bone = bones[i];
    std::cout << "bone name is: " << bone -> mName.C_Str() << std::endl;

    int numVerticesInBone = bone -> mNumWeights;
    std::cout << "num verts is: " << numVerticesInBone << std::endl;

    for (int j = 0; j < numVerticesInBone; j++){
      aiVertexWeight weight = bone -> mWeights[j];
      std::cout << "vertex id: " << weight.mVertexId << ", weight: " << weight.mWeight << std::endl;
      auto offsetMatrix = bone -> mOffsetMatrix[j];   // wtf is this i dont get it
    } 
  }


  std::cout << "num bones: " << numBones << std::endl;
}

MeshData processMesh(aiMesh* mesh, const aiScene* scene, std::string modelPath){
   std::vector<Vertex> vertices;
   std::vector<unsigned int> indices;
   
   for (unsigned int i = 0; i < mesh->mNumVertices; i++){
     Vertex vertex;
     vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
     vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);  

     // load one layer of texture coordinates for now
     if (!mesh -> mTextureCoords[0]){
        continue;
     }
     vertex.texCoords = glm::vec2(mesh-> mTextureCoords[0][i].x, mesh -> mTextureCoords[0][i].y);  // Maybe warn here is no texcoords but no materials ? 
     vertices.push_back(vertex);
   } 
 
   for (unsigned int i = 0; i < mesh->mNumFaces; i++){
     aiFace face = mesh->mFaces[i];
     for (unsigned int j = 0; j < face.mNumIndices; j++){
       indices.push_back(face.mIndices[j]);
     }
   }

   // Eventually this should support more than just diffuse maps.
   aiMaterial* material = scene->mMaterials[mesh -> mMaterialIndex];
   
   std::vector<std::string> textureFilepaths;
   for (unsigned int i = 0; i < material->GetTextureCount(aiTextureType_DIFFUSE); i++){
     aiString texturePath;
     material->GetTexture(aiTextureType_DIFFUSE, i, &texturePath);
     std::filesystem::path modellocation = std::filesystem::canonical(modelPath).parent_path();
     std::filesystem::path relativePath = std::filesystem::weakly_canonical(modellocation / texturePath.C_Str()); //  / is append operator 
     textureFilepaths.push_back(relativePath.string());  
   }

   processBones(mesh);

   MeshData model = {
     .vertices = vertices,
     .indices = indices,       
     .texturePaths = textureFilepaths,
     .boundInfo = getBounds(vertices),
   };

   return model;
}

void processNode(aiNode* node, const aiScene* scene, std::string modelPath, std::function<void(MeshData)> onLoadMesh){
   for (unsigned int i = 0; i < node->mNumMeshes; i++){
     MeshData meshData = processMesh(scene->mMeshes[node->mMeshes[i]], scene, modelPath);
     onLoadMesh(meshData);
   }
   for (unsigned int i = 0; i < node ->mNumChildren; i++){
     processNode(node->mChildren[i], scene, modelPath, onLoadMesh);
   }
}

// Currently this just loads all the meshes into the models array. 
// Should have parent/child relations and a hierarchy but todo.
std::vector<MeshData> loadModel(std::string modelPath){
   Assimp::Importer import;
   const aiScene* scene = import.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_GenNormals);
   if (!scene || scene->mFlags && AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
      std::cerr << "error loading model" << std::endl;
      throw std::runtime_error("Error loading model: does the file " + modelPath + " exist?");
   } 

   std::vector<MeshData> models;

   processAnimations(scene);
   processNode(scene->mRootNode, scene, modelPath, [&models](MeshData meshdata) -> void {
     models.push_back(meshdata);
   });

   return models;
}

