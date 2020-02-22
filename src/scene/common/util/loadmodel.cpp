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

std::vector<Animation> processAnimations(const aiScene* scene){
  std::vector<Animation> animations;

  int numAnimations = scene -> mNumAnimations;
  std::cout << "num animations: " << numAnimations << std::endl;
  for (int i = 0; i < numAnimations; i++){
    aiAnimation* animation = scene -> mAnimations[i];

    Animation ani {
      .name = animation -> mName.C_Str(),
      .duration = animation -> mDuration,
      .ticksPerSecond = animation -> mTicksPerSecond
    };

    animations.push_back(ani);

    for (int j = 0; j < animation -> mNumChannels; j++){
      aiNodeAnim* aiAnimation = animation-> mChannels[j];  
      std::cout << "affected node name: " << aiAnimation -> mNodeName.C_Str() << std::endl;  // http://assimp.sourceforge.net/lib_html/structai_node_anim.html
    }
  }

  return animations;
}

// https://stackoverflow.com/questions/29184311/how-to-rotate-a-skinned-models-bones-in-c-using-assimp
glm::mat4 aiMatrixToGlm(aiMatrix4x4 from){
  glm::mat4 to;
  to[0][0] = (GLfloat)from.a1; to[0][1] = (GLfloat)from.b1;  to[0][2] = (GLfloat)from.c1; to[0][3] = (GLfloat)from.d1;
  to[1][0] = (GLfloat)from.a2; to[1][1] = (GLfloat)from.b2;  to[1][2] = (GLfloat)from.c2; to[1][3] = (GLfloat)from.d2;
  to[2][0] = (GLfloat)from.a3; to[2][1] = (GLfloat)from.b3;  to[2][2] = (GLfloat)from.c3; to[2][3] = (GLfloat)from.d3;
  to[3][0] = (GLfloat)from.a4; to[3][1] = (GLfloat)from.b4;  to[3][2] = (GLfloat)from.c4; to[3][3] = (GLfloat)from.d4;
  return to;
}

struct BoneWeighting {
  int boneId;
  float weight;
};

struct BoneInfo {
  std::vector<Bone> bones;
  std::map<unsigned int, std::vector<BoneWeighting>>  vertexBoneWeight;
};

BoneInfo processBones(aiMesh* mesh){
  std::vector<Bone> meshBones;

  aiBone** bones = mesh -> mBones;

  std::map<unsigned int, std::vector<BoneWeighting>> vertexToBones;
  for (int i = 0; i < mesh -> mNumBones; i++){
    aiBone* bone = bones[i];

    Bone meshBone {
      .name = bone -> mName.C_Str(),
      .offsetMatrix = aiMatrixToGlm(bone -> mOffsetMatrix)
    };
    meshBones.push_back(meshBone);

    for (int j = 0; j < bone -> mNumWeights; j++){
      aiVertexWeight boneVertexWeight = bone -> mWeights[j];
      BoneWeighting boneWeight {
        .boneId = i,
        .weight = boneVertexWeight.mWeight
      };
      vertexToBones[boneVertexWeight.mVertexId].push_back(boneWeight);
    }
  }

  BoneInfo info {
    .bones = meshBones,
    .vertexBoneWeight = vertexToBones
  };
  
  return info;
}

void setDefaultBoneIndexesAndWeights(std::map<unsigned int, std::vector<BoneWeighting>>&  vertexBoneWeight, int vertexId, short* indices, float* weights, int size){
  std::vector<BoneWeighting> weighting;
  if (vertexBoneWeight.find(vertexId) != vertexBoneWeight.end()){
    weighting = vertexBoneWeight.at(vertexId);
  }
  assert(weighting.size() <= size);

  for (int i = 0; i < size; i++){
    if (i < weighting.size()){
      auto weight = weighting.at(i);
      indices[i] = weight.boneId;
      weights[i] = weight.weight;
    }else{
      indices[i] = 0;   // if no associated bone id, just put 0 bone id with 0 weighting so it won't add to the weight
      weights[i] = 0;
    }
  }
}



MeshData processMesh(aiMesh* mesh, const aiScene* scene, std::string modelPath){
   std::vector<Vertex> vertices;
   std::vector<unsigned int> indices;
   
   BoneInfo boneInfo = processBones(mesh);

   for (unsigned int i = 0; i < mesh->mNumVertices; i++){
     Vertex vertex;
     vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
     vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z); 
     setDefaultBoneIndexesAndWeights(boneInfo.vertexBoneWeight, i, vertex.boneIndexes, vertex.boneWeights, NUM_BONES_PER_VERTEX);

     // load one layer of texture coordinates for now
     if (!mesh -> mTextureCoords[0]){
        continue;
     }
     vertex.texCoords = glm::vec2(mesh -> mTextureCoords[0][i].x, mesh -> mTextureCoords[0][i].y);  // Maybe warn here is no texcoords but no materials ? 
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
     material -> GetTexture(aiTextureType_DIFFUSE, i, &texturePath);
     std::filesystem::path modellocation = std::filesystem::canonical(modelPath).parent_path();
     std::filesystem::path relativePath = std::filesystem::weakly_canonical(modellocation / texturePath.C_Str()); //  / is append operator 
     textureFilepaths.push_back(relativePath.string());  
   }


   MeshData model = {
     .vertices = vertices,
     .indices = indices,       
     .texturePaths = textureFilepaths,
     .boundInfo = getBounds(vertices),
     .bones = boneInfo.bones
   };

   return model;
}

void processNode(
  aiNode* node, 
  int parentNodeId,
  int* localNodeId, 
  std::function<void(int, int)> onLoadMesh,
  std::function<void(std::string, int, aiMatrix4x4)> onAddNode,
  std::function<void(int, int)> addParent
){
   *localNodeId = *localNodeId + 1;
   int currentNodeId =  *localNodeId;
  
   if (parentNodeId != -1){
     addParent(currentNodeId, parentNodeId);
   }

   onAddNode(node -> mName.C_Str(), currentNodeId, node -> mTransformation);
   for (unsigned int i = 0; i < (node -> mNumMeshes); i++){
     onLoadMesh(currentNodeId, node -> mMeshes[i]);
   }
   for (unsigned int i = 0; i < node -> mNumChildren; i++){
     processNode(node -> mChildren[i], currentNodeId, localNodeId, onLoadMesh, onAddNode, addParent);
   }
}

// Currently this just loads all the meshes into the models array. 
// Should have parent/child relations and a hierarchy but todo.
ModelData loadModel(std::string modelPath){
   Assimp::Importer import;
   const aiScene* scene = import.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_GenNormals);
   if (!scene || scene->mFlags && AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode){
      std::cerr << "error loading model" << std::endl;
      throw std::runtime_error("Error loading model: does the file " + modelPath + " exist?");
   } 

   std::map<short, MeshData> meshIdToMeshData;
   std::map<short, std::vector<int>> nodeToMeshId;
   std::map<short, short> childToParent;
   std::map<short, Transformation> nodeTransform;
   std::map<short, std::string> names;

   auto animations = processAnimations(scene);

   int localNodeId = -1;
   processNode(scene -> mRootNode, localNodeId, &localNodeId, 
    [&scene, modelPath, &meshIdToMeshData, &nodeToMeshId](int nodeId, int meshId) -> void {
      MeshData meshData = processMesh(scene -> mMeshes[meshId], scene, modelPath);
      nodeToMeshId[nodeId].push_back(meshId);
      meshIdToMeshData[meshId] = meshData;
    },
    [&nodeTransform, &names, &nodeToMeshId](std::string name, int nodeId, aiMatrix4x4 transform) -> void {
      names[nodeId] = name;
      aiVector3t<float> scaling;
      aiQuaterniont<float> rotation;
      aiVector3t<float> position;
      transform.Decompose(scaling, rotation, position);
      Transformation trans = {
        .position = glm::vec3(position.x, position.y, position.z),
        .scale = glm::vec3(scaling.x, scaling.y, scaling.z),
        .rotation = glm::quat(1.f, 0, 0, 0)
      };
      nodeTransform[nodeId] = trans;
      if (nodeToMeshId.find(nodeId) == nodeToMeshId.end()){
        std::vector<int> emptyMeshList;
        nodeToMeshId[nodeId] = emptyMeshList;
      }
    },
    [&childToParent](int parentId, int nodeId) -> void {
      childToParent[parentId] = nodeId;
    }
  );

   assert(nodeToMeshId.size() == nodeTransform.size());
   assert(names.size() ==  nodeToMeshId.size());

   ModelData data = {
     .meshIdToMeshData = meshIdToMeshData,
     .nodeToMeshId = nodeToMeshId,
     .childToParent = childToParent,
     .nodeTransform = nodeTransform,
     .names = names,
     .animations = animations
   };
   return data;
}

