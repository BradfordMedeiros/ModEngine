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

std::string generateNodeName(std::string rootname, const char* nodeName){
  return rootname + "/" + nodeName; 
}

std::vector<Animation> processAnimations(std::string rootname, const aiScene* scene){
  std::vector<Animation> animations;

  int numAnimations = scene -> mNumAnimations;
  std::cout << "num animations: " << numAnimations << std::endl;
  for (int i = 0; i < numAnimations; i++){
    aiAnimation* animation = scene -> mAnimations[i];

    std::string animationName = animation -> mName.C_Str();
    if (animationName == ""){
      animationName = "default:" + std::to_string(i); 
    }

    std::vector<AnimationChannel> channels;

    assert(animation -> mNumChannels > 0);
    for (int j = 0; j < animation -> mNumChannels; j++){
      aiNodeAnim* aiAnimation = animation-> mChannels[j];  

      std::vector<aiVectorKey> positionKeys;   
      std::vector<aiVectorKey> scalingKeys;
      std::vector<aiQuatKey> rotationKeys;

      for (int k = 0; k < aiAnimation -> mNumPositionKeys; k++){
        positionKeys.push_back(aiAnimation -> mPositionKeys[k]);
      }
      for (int k = 0; k < aiAnimation -> mNumScalingKeys; k++){
        scalingKeys.push_back(aiAnimation -> mScalingKeys[k]);

      }
      for (int k = 0; k < aiAnimation -> mNumRotationKeys; k++){
        rotationKeys.push_back(aiAnimation -> mRotationKeys[k]);
      }

      AnimationChannel channel {
        .nodeName = generateNodeName(rootname, aiAnimation -> mNodeName.C_Str()),
        .positionKeys = positionKeys,
        .scalingKeys = scalingKeys,
        .rotationKeys = rotationKeys
      };
      channels.push_back(channel);
    }
    
    Animation ani {
      .name = animationName,
      .duration = animation -> mDuration,
      .ticksPerSecond = animation -> mTicksPerSecond,
      .channels = channels
    };

    animations.push_back(ani);
  }

  return animations;
}


aiMatrix4x4 glmMatrixToAi(glm::mat4 mat){
  return aiMatrix4x4(
    mat[0][0],mat[0][1],mat[0][2],mat[0][3],
    mat[1][0],mat[1][1],mat[1][2],mat[1][3],
    mat[2][0],mat[2][1],mat[2][2],mat[2][3],
    mat[3][0],mat[3][1],mat[3][2],mat[3][3]
  );
}

glm::mat4 aiMatrixToGlm(aiMatrix4x4& in_mat){
  glm::mat4 tmp;
  tmp[0][0] = in_mat.a1;
  tmp[1][0] = in_mat.b1;
  tmp[2][0] = in_mat.c1;
  tmp[3][0] = in_mat.d1;

  tmp[0][1] = in_mat.a2;
  tmp[1][1] = in_mat.b2;
  tmp[2][1] = in_mat.c2;
  tmp[3][1] = in_mat.d2;

  tmp[0][2] = in_mat.a3;
  tmp[1][2] = in_mat.b3;
  tmp[2][2] = in_mat.c3;
  tmp[3][2] = in_mat.d3;

  tmp[0][3] = in_mat.a4;
  tmp[1][3] = in_mat.b4;
  tmp[2][3] = in_mat.c4;
  tmp[3][3] = in_mat.d4;
  return tmp;
}


glm::vec3 aiVectorToGlm(aiVector3D& vec){
  return glm::vec3(vec.x, vec.y, vec.z);
}
glm::quat aiQuatToGlm(aiQuaternion& quat){
  auto quaternion = glm::identity<glm::quat>();
  quaternion.x = quat.x;
  quaternion.y = quat.y;
  quaternion.z = quat.z;
  quaternion.w = quat.w;
  return quaternion;
}
glm::mat4 aiKeysToGlm(aiVectorKey& positionKey, aiQuatKey& rotationKey, aiVectorKey& scalingKey){
  auto position = aiVectorToGlm(positionKey.mValue);
  auto rotation = aiQuatToGlm(rotationKey.mValue);
  auto scaling = aiVectorToGlm(scalingKey.mValue);
    
  auto scalingMatrix = glm::scale(glm::mat4(1.f), scaling);                     //http://assimp.sourceforge.net/lib_html/structai_node_anim.html  scaling, then rotation, then translation
  auto rotationMatrix = glm::toMat4(rotation);
  auto positionMatrix = glm::translate(glm::mat4(1.f), position);

  return positionMatrix * rotationMatrix * scalingMatrix;
}


struct BoneWeighting {
  int boneId;
  float weight;
};

struct BoneInfo {
  std::vector<Bone> bones;
  std::map<unsigned int, std::vector<BoneWeighting>>  vertexBoneWeight;
};


BoneInfo processBones(std::string rootname, aiMesh* mesh){
  std::vector<Bone> meshBones;

  aiBone** bones = mesh -> mBones;

  std::map<unsigned int, std::vector<BoneWeighting>> vertexToBones;
  for (int i = 0; i < mesh -> mNumBones; i++){
    aiBone* bone = bones[i];
    Bone meshBone {
      .name = generateNodeName(rootname, bone -> mName.C_Str()),
      .offsetMatrix = glm::mat4(1.f),
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

void setDefaultBoneIndexesAndWeights(std::map<unsigned int, std::vector<BoneWeighting>>&  vertexBoneWeight, int vertexId, int32_t* indices, float* weights, int size){
  std::vector<BoneWeighting> weighting;
  if (vertexBoneWeight.find(vertexId) != vertexBoneWeight.end()){
    weighting = vertexBoneWeight.at(vertexId);
  }

  if (weighting.size() > size || size != 4){
    std::cout << "weighting size: " << weighting.size() << ", size: " << size << std::endl;
    assert(false);
  }

  for (int i = 0; i < size; i++){
    if (i < weighting.size()){
      auto weight = weighting.at(i);
      indices[i] = weight.boneId;
      weights[i] = weight.weight;
     }else{
//    assert(i != 0);
      indices[i] = 0;   // if no associated bone id, just put 0 bone id with 0 weighting so it won't add to the weight
      weights[i] = 0;
    }
  }
}

void dumpVerticesData(std::string modelPath, MeshData& model){
  for (auto vertex : model.vertices){
    std::string vertexInfo = "";
    vertexInfo = vertexInfo + print(vertex.position) + " " + print(vertex.normal) + " " + print(vertex.normal);

    vertexInfo = vertexInfo + " (";
    for (int i = 0; i < NUM_BONES_PER_VERTEX; i++){
      vertexInfo = vertexInfo + std::to_string(vertex.boneIndexes[i]) + (i < NUM_BONES_PER_VERTEX - 1 ? ", " : "");
    }
    vertexInfo = vertexInfo + ")";

    vertexInfo = vertexInfo + " (";
    for (int i = 0; i < NUM_BONES_PER_VERTEX; i++){
      vertexInfo = vertexInfo + std::to_string(vertex.boneWeights[i]) + (i < NUM_BONES_PER_VERTEX - 1 ? ", " : "");
    }
    vertexInfo = vertexInfo + ") ";

    std::cout << modelPath << " v: " << vertexInfo << std::endl;
  }
} 

std::string getTexturePath(aiTextureType type, std::string modelPath,  aiMaterial* material){
  aiString texturePath;
  material -> GetTexture(type, 0, &texturePath);
  std::filesystem::path modellocation = std::filesystem::canonical(modelPath).parent_path();
  std::filesystem::path relativePath = std::filesystem::weakly_canonical(modellocation / texturePath.C_Str()); //  / is append operator 
  return relativePath.string();
}

MeshData processMesh(std::string rootname, aiMesh* mesh, const aiScene* scene, std::string modelPath){
   std::vector<Vertex> vertices;
   std::vector<unsigned int> indices;
   
   BoneInfo boneInfo = processBones(rootname, mesh);

   std::cout << "loading modelPath: " << modelPath << std::endl;
   for (unsigned int i = 0; i < mesh -> mNumVertices; i++){
     Vertex vertex;
     vertex.position = glm::vec3(mesh -> mVertices[i].x, mesh -> mVertices[i].y, mesh -> mVertices[i].z);
     vertex.normal = glm::vec3(mesh -> mNormals[i].x, mesh -> mNormals[i].y, mesh -> mNormals[i].z); 
     setDefaultBoneIndexesAndWeights(boneInfo.vertexBoneWeight, i, vertex.boneIndexes, vertex.boneWeights, NUM_BONES_PER_VERTEX);

     // load one layer of texture coordinates for now
     if (!mesh -> mTextureCoords[0]){
        assert(false);
        continue;
     }
     vertex.texCoords = glm::vec2(mesh -> mTextureCoords[0][i].x, mesh -> mTextureCoords[0][i].y);  // Maybe warn here is no texcoords but no materials ? 
     vertices.push_back(vertex);
   } 
 
   for (unsigned int i = 0; i < mesh -> mNumFaces; i++){
     aiFace face = mesh -> mFaces[i];
     for (unsigned int j = 0; j < face.mNumIndices; j++){
       indices.push_back(face.mIndices[j]);
     }
   }

   aiMaterial* material = scene -> mMaterials[mesh -> mMaterialIndex];
   
   int diffuseTextureCount = material -> GetTextureCount(aiTextureType_DIFFUSE);
   assert(diffuseTextureCount == 0 || diffuseTextureCount == 1);
   std::string diffuseTexturePath;
   if (diffuseTextureCount == 1){
     diffuseTexturePath = getTexturePath(aiTextureType_DIFFUSE, modelPath, material);
   }

   int emissionTextureCount = material -> GetTextureCount(aiTextureType_EMISSIVE);
   assert(emissionTextureCount == 0 || emissionTextureCount == 1);
   std::string emissionTexturePath;
   if (emissionTextureCount == 1){
     emissionTexturePath = getTexturePath(aiTextureType_EMISSIVE, modelPath, material);
   }

   int opacityTextureCount = material -> GetTextureCount(aiTextureType_OPACITY);
   assert(opacityTextureCount == 0 || opacityTextureCount == 1);
   std::string opacityTexturePath;
   if (opacityTextureCount == 1){
     opacityTexturePath = getTexturePath(aiTextureType_OPACITY, modelPath, material);
   }

   MeshData model = {
     .vertices = vertices,
     .indices = indices,       
     .diffuseTexturePath = diffuseTexturePath,
     .hasDiffuseTexture = diffuseTextureCount == 1,
     .emissionTexturePath = emissionTexturePath,
     .hasEmissionTexture = emissionTextureCount == 1,
     .opacityTexturePath = opacityTexturePath,
     .hasOpacityTexture = opacityTextureCount == 1,
     .boundInfo = getBounds(vertices),
     .bones = boneInfo.bones,
   };

   //dumpVerticesData(modelPath, model); 

   return model;
}

void processNode(
  std::string rootname,
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

   auto nodeName = generateNodeName(rootname, node -> mName.C_Str());
   onAddNode(nodeName, currentNodeId, node -> mTransformation);
   for (unsigned int i = 0; i < (node -> mNumMeshes); i++){
     onLoadMesh(currentNodeId, node -> mMeshes[i]);
   }
   for (unsigned int i = 0; i < node -> mNumChildren; i++){
     processNode(rootname, node -> mChildren[i], currentNodeId, localNodeId, onLoadMesh, onAddNode, addParent);
   }
}

void assertAllNamesUnique(std::map<int32_t, std::string>& idToName){
  bool foundDuplicate = false;
  std::map<std::string, int> names;
  for (auto [val, name] : idToName){
    if (names.find(name) != names.end()){
      foundDuplicate = true;
    }
    names[name] = val;
  }
  assert(!foundDuplicate);
}

// Currently this just loads all the meshes into the models array. 
// Should have parent/child relations and a hierarchy but todo.
ModelData loadModel(std::string rootname, std::string modelPath){
   Assimp::Importer import;
   const aiScene* scene = import.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_GenNormals);
   if (!scene || scene -> mFlags && AI_SCENE_FLAGS_INCOMPLETE || !scene -> mRootNode){
      std::cerr << "error loading model" << std::endl;
      throw std::runtime_error("Error loading model: does the file " + modelPath + " exist and is valid?");
   } 

   std::map<int32_t, MeshData> meshIdToMeshData;
   std::map<int32_t, std::vector<int>> nodeToMeshId;
   std::map<int32_t, int32_t> childToParent;
   std::map<int32_t, Transformation> nodeTransform;
   std::map<int32_t, std::string> names;

   auto animations = processAnimations(rootname, scene);

   int localNodeId = -1;
   processNode(rootname, scene -> mRootNode, localNodeId, &localNodeId, 
    [&scene, modelPath, &meshIdToMeshData, &nodeToMeshId, &rootname](int nodeId, int meshId) -> void {
      // load mesh
      MeshData meshData = processMesh(rootname, scene -> mMeshes[meshId], scene, modelPath);
      nodeToMeshId[nodeId].push_back(meshId);
      meshIdToMeshData[meshId] = meshData;
    },
    [&nodeTransform, &names, &nodeToMeshId](std::string name, int nodeId, aiMatrix4x4 transform) -> void {
      // add node
      names[nodeId] = name;  
      
      aiVector3t<float> scaling;
      aiQuaterniont<float> rotation;
      aiVector3t<float> position;
      transform.Decompose(scaling, rotation, position);
      Transformation trans = {
        .position = glm::vec3(position.x, position.y, position.z),
        .scale = glm::vec3(scaling.x, scaling.y, scaling.z),
        .rotation = aiQuatToGlm(rotation),
      };

      nodeTransform[nodeId] = trans;
      if (nodeToMeshId.find(nodeId) == nodeToMeshId.end()){
        std::vector<int> emptyMeshList;
        nodeToMeshId[nodeId] = emptyMeshList;
      }
    },
    [&childToParent](int parentId, int nodeId) -> void {
      // add parent
      childToParent[parentId] = nodeId;
    }
  );

   assert(nodeToMeshId.size() == nodeTransform.size());
   assert(names.size() ==  nodeToMeshId.size());
   assertAllNamesUnique(names);

   ModelData data = {
     .meshIdToMeshData = meshIdToMeshData,
     .nodeToMeshId = nodeToMeshId,
     .childToParent = childToParent,
     .nodeTransform = nodeTransform,
     .names = names,
     .animations = animations,
   };
   return data;
}

