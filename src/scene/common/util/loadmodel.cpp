#include "./loadmodel.h"

std::string readFileOrPackage(std::string filepath);
ModelDataCore loadModelCoreBrush(std::string modelPath);

std::vector<Animation> processAnimations( const aiScene* scene){
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
        .nodeName = aiAnimation -> mNodeName.C_Str(),
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
  glm::mat4 temp;
  temp[0][0] = in_mat.a1; 
  temp[0][1] = in_mat.b1;  
  temp[0][2] = in_mat.c1; 
  temp[0][3] = in_mat.d1;
  temp[1][0] = in_mat.a2; 
  temp[1][1] = in_mat.b2;  
  temp[1][2] = in_mat.c2; 
  temp[1][3] = in_mat.d2;
  temp[2][0] = in_mat.a3; 
  temp[2][1] = in_mat.b3;  
  temp[2][2] = in_mat.c3; 
  temp[2][3] = in_mat.d3;
  temp[3][0] = in_mat.a4; 
  temp[3][1] = in_mat.b4;  
  temp[3][2] = in_mat.c4; 
  temp[3][3] = in_mat.d4;
  return temp;
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

Transformation aiKeysToTransform(aiVectorKey& positionKey, aiQuatKey& rotationKey, aiVectorKey& scalingKey){
  Transformation transform {
    .position = aiVectorToGlm(positionKey.mValue),
    .scale = aiVectorToGlm(scalingKey.mValue),
    .rotation = aiQuatToGlm(rotationKey.mValue),
  };
  return transform;
}
glm::mat4 transformToGlm(Transformation transform){
  //http://assimp.sourceforge.net/lib_html/structai_node_anim.html  scaling, then rotation, then translation
  auto positionMatrix = glm::translate(glm::mat4(1.f), transform.position);
  auto rotationMatrix = glm::toMat4(transform.rotation);
  auto scalingMatrix = glm::scale(glm::mat4(1.f), transform.scale);

  return positionMatrix * rotationMatrix * scalingMatrix;
}


struct BoneWeighting {
  int boneId;
  float weight;
};

struct BoneInfo {
  std::vector<Bone> bones;
  std::unordered_map<unsigned int, std::vector<BoneWeighting>>  vertexBoneWeight;
};

void printMatrix(std::string bonename, aiMatrix4x4& matrix, glm::mat4 glmMatrix){
  std::cout << "process bones: " << bonename << std::endl;
  auto transform = getTransformationFromMatrix(glmMatrix);
  aiVector3t<float> scaling;
  aiQuaterniont<float> rotation;
  aiVector3t<float> position;
  matrix.Decompose(scaling, rotation, position);

  std::cout << "BONEINFO_MODEL: " << bonename << " " << " position: " << print(transform.position) << " | " << print(aiVectorToGlm(position)) << std::endl;
  std::cout << "BONEINFO_MODEL: " << bonename << " " << " scale: " << print(transform.scale) << " | " << print(aiVectorToGlm(scaling)) << std::endl;
  std::cout << "BONEINFO_MODEL: " << bonename << " " << " rotation: " << print(transform.rotation) << std::endl << std::endl;
}

BoneInfo processBones(aiMesh* mesh){
  std::vector<Bone> meshBones;

  aiBone** bones = mesh -> mBones;

  std::unordered_map<unsigned int, std::vector<BoneWeighting>> vertexToBones;
  for (int i = 0; i < mesh -> mNumBones; i++){
    aiBone* bone = bones[i];
    Bone meshBone {
      .name = bone -> mName.C_Str(),
      .offsetMatrix = glm::mat4(1.f),
      .initialBonePoseInverse = glm::mat4(1.f),  // this gets populated in setInitialBonePoses since needs lookup
      .initialLocalTransform = getTransformationFromMatrix(glm::mat4(1.f)),  // same here
    };

    printMatrix(meshBone.name, bone -> mOffsetMatrix, meshBone.initialBonePoseInverse);

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

void setDefaultBoneIndexesAndWeights(std::unordered_map<unsigned int, std::vector<BoneWeighting>>&  vertexBoneWeight, int vertexId, int32_t* indices, float* weights, int size){
  std::vector<BoneWeighting> weighting;
  if (vertexBoneWeight.find(vertexId) != vertexBoneWeight.end()){
    weighting = vertexBoneWeight.at(vertexId);
  }

  if (weighting.size() > size || size != 4){
    std::cout << "printBoneWeighting actual weighting size: " << weighting.size() << ", max target size: " << size << std::endl;
    //std::cout << "actual weighting size: " << weighting.size() << ", max target size: " << size << std::endl;
    //assert(false);
  }

  for (int i = 0; i < size; i++){
    if (i < weighting.size()){
      modlog("Animation weighting size", std::to_string(weighting.size()));
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

std::optional<int32_t> getNodeId(ModelData& data, std::string nodename){
  for (auto &[id, name] : data.names){
    if (name == nodename){
      return id;
    }
  }
  std::cout << "no node named: [" << nodename << "]" << std::endl;
  std::cout << "existing nodes: [ ";
  for (auto &[_, name] : data.names){
    std::cout << name << " ";
  }
  std::cout << "]" << std::endl;
  return std::nullopt;
}

void setInitialBonePoses(ModelData& data, std::unordered_map<int32_t, glm::mat4>& fullnodeTransform, std::unordered_map<int32_t, Transformation>& localTransforms){
  for (auto &[id, transform] : fullnodeTransform){
    printMatrixInformation(transform, std::string("initialbone - ") + std::to_string(id));
  }
  for (auto &[id, meshdata] : data.meshIdToMeshData){
    for (auto &bone : meshdata.bones){
      bone.initialBonePoseInverse = glm::inverse(fullnodeTransform.at(getNodeId(data, bone.name).value()));
      bone.initialLocalTransform = localTransforms.at(getNodeId(data, bone.name).value());
      std::cout << "set bone initial: " << print(bone.initialLocalTransform) << std::endl;
      printMatrixInformation(bone.initialBonePoseInverse, std::string("offsetmatrix - " + bone.name));
    }
  }
}

std::string generateNodeName(std::string rootname, const char* nodeName){
  return rootname + "/" + nodeName; 
}
void renameRootNode(ModelData& data, std::string rootname, std::string realrootname){
  for (auto &[_, meshdata] : data.meshIdToMeshData){
    for (auto &bone : meshdata.bones){
      // bone.name
      bone.shortName = bone.name;
      if (bone.name == realrootname){
        bone.name = rootname;
        assert(false);   // figure out when this happens
      }else{
        bone.name = generateNodeName(rootname, bone.name.c_str());
      }
    }
  }
  for (auto &idToName : data.names){
    if (idToName.second == realrootname){
      idToName.second = rootname;
    }else{
      idToName.second = generateNodeName(rootname, idToName.second.c_str());
    }
  }

  for (auto &animation : data.animations){
    for (auto &channel : animation.channels){
      // chnnael.nodeName
      if (channel.nodeName == realrootname){
        channel.nodeName = rootname;
      }else{
        channel.nodeName = generateNodeName(rootname, channel.nodeName.c_str());
      }
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

  std::filesystem::path modellocation = std::filesystem::path(modelPath).parent_path();
  std::filesystem::path texturelocation = std::filesystem::path(texturePath.C_Str());

  std::filesystem::path relativePath = (modellocation / texturelocation).lexically_normal(); //  / is append operator

  bool shouldAddLeadingDot = false;
  std::string finalPath = relativePath.string();
  if (finalPath.size() >= 2){
    bool isUpDir = finalPath.at(0) == '.' && finalPath.at(1) == '.';
    bool isAbsPath = finalPath.at(0) == '/';
    if (!isUpDir && !isAbsPath){
      shouldAddLeadingDot = true;
    }
  }

  std::cout << "test diffuse modelPath: " << modelPath << std::endl;
  std::cout << "test diffuse get final path: " << finalPath << std::endl;
  std::cout << "test diffuse get texture path: " << relativePath.string() << std::endl;
  std::cout << "test diffuse raw texture path: " << texturePath.C_Str() << std::endl;
  if (shouldAddLeadingDot){
    finalPath = "./" + finalPath;
  }

  std::cout << "test diffuse texture getTexturePath: " << finalPath << std::endl;

  return finalPath;
}

std::string print(std::vector<BoneWeighting>& bones, BoneInfo& boneInfo){
  std::string val;
  for (auto &bone : bones){
    val = val + " " + boneInfo.bones.at(bone.boneId).name;
  }
  return val;
}

void printBoneWeighting(BoneInfo& boneInfo){
  for (auto &[vertexId, bones] : boneInfo.vertexBoneWeight){
    if (bones.size() > 4){
      std::cout << "printBoneWeighting: too many bones, vertex = " << vertexId << ", num_bones = " << std::to_string(bones.size()) << ", bones = " << print(bones, boneInfo) << std::endl;
      //modassert(false, "too many bones");
    }
  }
}


const bool DUMP_VERTEX_DATA = false;
MeshData processMesh(aiMesh* mesh, const aiScene* scene, std::string modelPath){
   std::vector<Vertex> vertices;
   std::vector<unsigned int> indices;
   
   BoneInfo boneInfo = processBones(mesh);
   modlog("Animation bone size", std::to_string(boneInfo.bones.size()));

   printBoneWeighting(boneInfo);

   std::cout << "loading modelPath: " << modelPath << std::endl;
   for (unsigned int i = 0; i < mesh -> mNumVertices; i++){
     Vertex vertex;
     vertex.position = glm::vec3(mesh -> mVertices[i].x, mesh -> mVertices[i].y, mesh -> mVertices[i].z);
     vertex.normal = glm::vec3(mesh -> mNormals[i].x, mesh -> mNormals[i].y, mesh -> mNormals[i].z); 
     vertex.tangent = glm::vec3(mesh -> mTangents[i].x, mesh -> mTangents[i].y, mesh -> mTangents[i].z);
     vertex.color = glm::vec3(0.f, 0.f, 0.f);
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
     std::cout << "test diffuse texture core: " << diffuseTexturePath << std::endl;
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

   // This is weird in assimp... this is the roughness/metallic map....
   // See https://github.com/assimp/assimp/blob/master/include/assimp/pbrmaterial.h#L57
   // #define AI_MATKEY_GLTF_PBRMETALLICROUGHNESS_METALLICROUGHNESS_TEXTURE aiTextureType_UNKNOWN, 0
   int roughnessTextureCount = material -> GetTextureCount(aiTextureType_UNKNOWN);
   assert(roughnessTextureCount == 0 || roughnessTextureCount == 1);
   std::string roughnessTexturePath;
   if (roughnessTextureCount == 1){
     roughnessTexturePath = getTexturePath(aiTextureType_UNKNOWN, modelPath, material);
   }

   int normalTextureCount = material -> GetTextureCount(aiTextureType_NORMALS);
   assert(normalTextureCount == 0 || normalTextureCount == 1);
   std::string normalTexturePath;
   if (normalTextureCount == 1){
     normalTexturePath = getTexturePath(aiTextureType_NORMALS, modelPath, material);
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
     .roughnessTexturePath = roughnessTexturePath,
     .hasRoughnessTexture = roughnessTextureCount == 1,
     .normalTexturePath = normalTexturePath,
     .hasNormalTexture = normalTextureCount == 1,
     .boundInfo = getBounds(vertices),
     .bones = boneInfo.bones,
   };

   if (DUMP_VERTEX_DATA){
     dumpVerticesData(modelPath, model); 
   }
   return model;
}

Transformation aiMatrixToTransform(aiMatrix4x4& matrix){
  aiVector3t<float> scaling;
  aiQuaterniont<float> rotation;
  aiVector3t<float> position;  
  matrix.Decompose(scaling, rotation, position);
  Transformation trans = {
    .position = glm::vec3(position.x, position.y, position.z),
    .scale = glm::vec3(scaling.x, scaling.y, scaling.z),
    .rotation = aiQuatToGlm(rotation),
  };
  return trans;
}

void processNode(
  aiNode* node, 
  int parentNodeId,
  int* localNodeId, 
  std::function<void(int, int)> onLoadMesh,
  std::function<void(std::string, int, Transformation& transform, glm::mat4, int depth)> onAddNode,
  std::function<void(int, int)> addParent,
  int depth,
  glm::mat4 fullTransform
){
   *localNodeId = *localNodeId + 1;
   int currentNodeId =  *localNodeId;
  
   if (parentNodeId != -1){
     addParent(currentNodeId, parentNodeId);
   }

   auto nodeName = node -> mName.C_Str();
   auto trans = aiMatrixToTransform(node -> mTransformation); 

   // Root node uses position specified, not what the model says
   auto transformMatrix = parentNodeId == -1 ? fullTransform : matrixFromComponents(fullTransform, trans.position, trans.scale, trans.rotation);

   onAddNode(nodeName, currentNodeId, trans, transformMatrix, depth);
   for (unsigned int i = 0; i < (node -> mNumMeshes); i++){
     onLoadMesh(currentNodeId, node -> mMeshes[i]);
   }
   for (unsigned int i = 0; i < node -> mNumChildren; i++){
     processNode(node -> mChildren[i], currentNodeId, localNodeId, onLoadMesh, onAddNode, addParent, depth + 1, transformMatrix);
   }
}

void assertAllNamesUnique(std::unordered_map<int32_t, std::string>& idToName){
  bool foundDuplicate = false;
  std::unordered_map<std::string, int> names;
  for (auto [val, name] : idToName){
    if (names.find(name) != names.end()){
      foundDuplicate = true;
    }
    names[name] = val;
  }
  assert(!foundDuplicate);
}

void printTransformDebug(Transformation& transform){
  std::cout << "pos: " << print(transform.position) << " | ";
  std::cout << "scale: " << print(transform.scale) << " | ";
  std::cout << "rot: " << print(transform.rotation);
}

void printDebugModelData(ModelData& data, std::string modelPath){
  std::cout << "DEBUG: Model Data: " << modelPath << std::endl;

  std::cout << "bone data: " << std::endl;
  for (auto &[meshid, meshData] : data.meshIdToMeshData){
    std::cout << "(" << meshid << ", [" << std::endl;
    for (auto bone : meshData.bones){
      std::cout << "  (" << bone.name << " " << std::endl;
      auto initialBonePoseInverse = getTransformationFromMatrix(bone.initialBonePoseInverse);
      printMatrixInformation(bone.initialBonePoseInverse, "    bone");
      std::cout << "  )" << std::endl;
    }
    std::cout << "])" << std::endl;
  }
  std::cout << std::endl;


  std::cout << "nodeToMeshId ids: " << std::endl;
  for (auto &[id, meshids] : data.nodeToMeshId){
    std::cout << id << " - [ ";
    for (auto meshid : meshids){
      std::cout << meshid << " ";
    }
    std::cout << "]" << std::endl;
  }
  std::cout << std::endl;


  std::cout << "childid to parentid: " << std::endl;
  for (auto &[childid, parentid] : data.childToParent){
    std::cout << "(" << childid << ", " << parentid << ")" << std::endl;
  }
  std::cout << std::endl;


  std::cout << "nodeid to transform: " << std::endl;
  for (auto &[nodeid, transform] : data.nodeTransform){
    std::cout << "(" << nodeid << ", " << print(transform) << ")" << std::endl;
  }
  std::cout << std::endl;


  std::cout << "id to name: " << std::endl;
  for (auto &[id, name] : data.names){
    std::cout << "(" << id << ", " << name << ")" << std::endl;
  }
  std::cout << std::endl;


  std::cout << "animations: " << std::endl;
  for (auto &animation : data.animations){
    std::cout << "(" << animation.name << "[" << std::endl;
    for (auto &channel : animation.channels){
      std::cout << channel.nodeName << ", "; 
    }
    std::cout  << "]) " << std::endl;
  }
  std::cout << std::endl;

}



// This is some horrible shit to make the file IO able to read from in memory that I specify
class CustomIOStream : public Assimp::IOStream {
public:
  std::string file;
  int32_t seekOffset = 0;
  int32_t fileSize = 0;
  CustomIOStream(std::string file){
    this -> file = file;
    this -> fileSize = readFileOrPackage(file).size();
  };
  ~CustomIOStream(){};

  size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override {
    auto fileContent = readFileOrPackage(this -> file);
    if (this -> seekOffset >= fileContent.size()) {
      return 0;  // End of file
    }
    auto numBytes = pSize * pCount;
    numBytes = std::min(numBytes, fileContent.size() - this -> seekOffset);
    memcpy(pvBuffer, fileContent.data() + this -> seekOffset, numBytes);
    this -> seekOffset += numBytes;
    return numBytes / pSize;
  }
  size_t Write( const void* pvBuffer, size_t pSize, size_t pCount) override{
    modassert(false, std::string("write not supported, wanted to write: ") + std::string(this->file));
    return 0;
  }
  aiReturn Seek( size_t pOffset, aiOrigin pOrigin) override{
    if (pOrigin == aiOrigin_SET){
      this -> seekOffset = pOffset;
      //std::cout << "seek set: " << std::to_string(pOrigin) << std::endl;
    }else if (pOrigin == aiOrigin_CUR){
      this -> seekOffset = this -> seekOffset + pOffset;
      //std::cout << "seek curr: " << std::to_string(pOrigin) << std::endl;
    }else if (pOrigin == aiOrigin_END){
      this -> seekOffset = this -> fileSize + pOffset;
      //std::cout << "seek end: " << std::to_string(pOrigin) << std::endl;
    }else{
      modassert(false, "unexpected seek pOrigin");
    }
    return aiReturn_SUCCESS;
  }
  size_t Tell() const override{
    modassert(false, std::string("Tell not supported, wanted to Tell: ") + std::string(this->file));
    return 0;
  }
  size_t FileSize() const override {
    return this -> fileSize;
  }
  void Flush () {}
};

// Fisher Price - My First Filesystem
class MyIOSystem : public Assimp::IOSystem {
public:
  MyIOSystem() {}
  ~MyIOSystem() {}

  bool Exists(const char* file) const override {
    //std::cout << "checking if file exists:  " << file << std::endl;
    return true;
  }
  char getOsSeparator() const override {
    return '/';
  }
  Assimp::IOStream* Open(const char* file, const char* mode) override {
    std::cout << "iostream open file: " << file << std::endl;
    ::CustomIOStream* ptr = new ::CustomIOStream(std::string(file));
    return ptr;
  }
  void Close(Assimp::IOStream* pFile) override{ 
    std::cout << "iostream closefile" << std::endl;
    delete pFile; 
  }
};


ModelDataCore loadModelCoreAssimp(std::string modelPath){
   Assimp::Importer import;

   MyIOSystem ioSystem ;
   import.SetIOHandler(&ioSystem);

   modlog("load file loadModelCore", modelPath);
   std::string fileContent = readFileOrPackage(modelPath);

   const aiScene* scene = import.ReadFile(modelPath, aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
   if (!scene || scene -> mFlags && AI_SCENE_FLAGS_INCOMPLETE || !scene -> mRootNode){
      std::cerr << "error loading model" << std::endl;
      modassert(false, std::string("Error loading model: does the file ") + modelPath + " "  + std::string(import.GetErrorString()));
   } 
   import.SetIOHandler(NULL); // otherwise Assimp::Importer will try to free this on destruct...ok bro 
   std::cout << "loading file" << std::endl;

   std::unordered_map<int32_t, MeshData> meshIdToMeshData;
   std::unordered_map<int32_t, std::vector<int>> nodeToMeshId;
   std::unordered_map<int32_t, int32_t> childToParent;
   std::unordered_map<int32_t, Transformation> nodeTransform;
   std::unordered_map<int32_t, glm::mat4> fullnodeTransform;
   std::unordered_map<int32_t, std::string> names;

   auto animations = processAnimations(scene);

   int localNodeId = -1;
   std::set<std::string> boneNames;
   processNode(scene -> mRootNode, localNodeId, &localNodeId, 
    [&scene, modelPath, &meshIdToMeshData, &nodeToMeshId, &boneNames](int nodeId, int meshId) -> void {
      // load mesh
      MeshData meshData = processMesh(scene -> mMeshes[meshId], scene, modelPath);
      nodeToMeshId[nodeId].push_back(meshId);
      meshIdToMeshData[meshId] = meshData;
      
      for (auto &bone : meshData.bones){
        boneNames.insert(bone.name);
      }
    },
    [&nodeTransform, &fullnodeTransform, &names, &nodeToMeshId](std::string name, int nodeId, Transformation& trans, glm::mat4 fullTransform, int depth) -> void {
      // add node
      names[nodeId] = name;  
      nodeTransform[nodeId] = trans;
      fullnodeTransform[nodeId] = fullTransform;
      if (nodeToMeshId.find(nodeId) == nodeToMeshId.end()){
        std::vector<int> emptyMeshList;
        nodeToMeshId[nodeId] = emptyMeshList;
      }
    },
    [&childToParent](int parentId, int nodeId) -> void {
      // add parent
      childToParent[parentId] = nodeId;
    },
    0,
    glm::mat4(1.f)
  );
 
  assert(nodeToMeshId.size() == nodeTransform.size());
  assert(names.size() ==  nodeToMeshId.size());
  assertAllNamesUnique(names);

  modlog("process bones: ", print(boneNames));

  std::set<int32_t> boneIds;
  for (auto &[id, name] : names){
    if (boneNames.count(name) > 0){
      boneIds.insert(id);
    }else if (name.find("mixamorig") != std::string::npos || name.find("Armature") != std::string::npos){ // obviously not good
      boneIds.insert(id);
    }
  }

   modlog("found bone ids: ", print(boneIds));
   ModelDataCore coreModelData {
      .modelData = ModelData {
        .meshIdToMeshData = meshIdToMeshData,
        .nodeToMeshId = nodeToMeshId,
        .childToParent = childToParent,
        .nodeTransform = nodeTransform,
        .names = names,
        .bones = boneIds,
        .animations = animations,
      },
      .loadedRoot = scene -> mRootNode -> mName.C_Str(),
   };

   // pass in full transforms, and bones, then set initialoffset to full transform of bone
   setInitialBonePoses(coreModelData.modelData, fullnodeTransform, nodeTransform); 
   printDebugModelData(coreModelData.modelData, modelPath);

   return coreModelData;
}

ModelDataCore loadModelCore(std::string modelPath){
  auto extension = getExtension(modelPath);
  if (extension.has_value() && extension.value() == "brush"){
    return loadModelCoreBrush(modelPath);
  }
  return loadModelCoreAssimp(modelPath);
}


ModelData extractModel(ModelDataCore& modelCore, std::string rootname){
  auto modelData = modelCore.modelData;
  renameRootNode(modelData, rootname, modelCore.loadedRoot);
  return modelData;
}

ModelData loadModel(std::string rootname, std::string modelPath){
  auto data = loadModelCore(modelPath);
  renameRootNode(data.modelData, rootname, data.loadedRoot);
  return data.modelData;
}

std::vector<glm::vec3> getVertexsFromModelData(ModelData& data){
  std::vector<glm::vec3> vertexs;
  for (auto [id, meshData] : data.meshIdToMeshData){
    for (auto index : meshData.indices){
      vertexs.push_back(meshData.vertices.at(index).position);
    }
  }
  return vertexs;
}

int32_t nodeIdFromName(ModelData& modelData, std::string targetName){
  for (auto &[nodeId, name] : modelData.names){
    if (name == targetName){
      return nodeId;
    }
  }
  std::cout << "no node named: " << targetName << std::endl;
  assert(false);
  return -1;
}

std::string nameForMeshId(std::string& rootmesh, int32_t meshId){
  return rootmesh + "=" + std::to_string(meshId);
}
bool isRootMeshName(std::string& meshname){
  return !stringContains(meshname, '=');
}
std::string rootMesh(std::string& meshname){
  auto rootAndRest = carAndRest(meshname, '=');
  return rootAndRest.first;
}
std::vector<std::string> meshNamesForNode(ModelData& modelData, std::string& rootmesh, std::string nodeName){
  std::vector<std::string> meshnames;
  auto meshIds = modelData.nodeToMeshId.at(nodeIdFromName(modelData, nodeName));
  for (auto meshId : meshIds){
    meshnames.push_back(nameForMeshId(rootmesh, meshId));
  }
  return meshnames;
}