#include "./loadmodel.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <utility>

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
  auto start = std::chrono::steady_clock::now();
  auto extension = getExtension(modelPath);
  ModelDataCore modelCore{};
  if (extension.has_value() && extension.value() == "map"){
    modelCore = loadModelCoreBrush(modelPath);
  }else if (extension.has_value() && extension.value() == "model"){
    modelCore.modelData = loadModelData(modelPath);
    std::optional<int32_t> rootId;
    for (auto& [id, _] : modelCore.modelData.nodeTransform){
      if (modelCore.modelData.childToParent.find(id) == modelCore.modelData.childToParent.end()){
        modassert(!rootId.has_value(), "model data has multiple root nodes");
        rootId = id;
      }
    }
    modassert(rootId.has_value(), "model data has no root node");
    modelCore.loadedRoot = modelCore.modelData.names.at(rootId.value());
  }else{
    modelCore = loadModelCoreAssimp(modelPath);
  }
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
  modlog("loadModelCore duration", modelPath + " duration " + std::to_string(duration.count()) + " ms");
  return modelCore;
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

rapidjson::Value vec3ToJson(glm::vec3 vec, rapidjson::Document::AllocatorType& allocator){
  modassert(std::isfinite(vec.x) && std::isfinite(vec.y) && std::isfinite(vec.z), "saveModelData found non-finite vec3 component");
  rapidjson::Value values(rapidjson::kArrayType);
  values.PushBack(vec.x, allocator);
  values.PushBack(vec.y, allocator);
  values.PushBack(vec.z, allocator);
  return values;
}

rapidjson::Value vec2ToJson(glm::vec2 vec, rapidjson::Document::AllocatorType& allocator){
  modassert(std::isfinite(vec.x) && std::isfinite(vec.y), "saveModelData found non-finite vec2 component");
  rapidjson::Value values(rapidjson::kArrayType);
  values.PushBack(vec.x, allocator);
  values.PushBack(vec.y, allocator);
  return values;
}

rapidjson::Value vec4ToJson(glm::vec4 vec, rapidjson::Document::AllocatorType& allocator){
  modassert(std::isfinite(vec.x) && std::isfinite(vec.y) && std::isfinite(vec.z) && std::isfinite(vec.w), "saveModelData found non-finite vec4 component");
  rapidjson::Value values(rapidjson::kArrayType);
  values.PushBack(vec.x, allocator);
  values.PushBack(vec.y, allocator);
  values.PushBack(vec.z, allocator);
  values.PushBack(vec.w, allocator);
  return values;
}

rapidjson::Value mat4ToJson(glm::mat4 matrix, rapidjson::Document::AllocatorType& allocator){
  rapidjson::Value columns(rapidjson::kArrayType);
  for (int column = 0; column < 4; column++){
    columns.PushBack(vec4ToJson(matrix[column], allocator), allocator);
  }
  return columns;
}

void saveModelData(ModelData& modelData, std::string filepath){
  rapidjson::Document doc;
  doc.SetObject();
  auto& allocator = doc.GetAllocator();

  {
   rapidjson::Value meshes(rapidjson::kArrayType);
   std::vector<int32_t> meshIds;
   meshIds.reserve(modelData.meshIdToMeshData.size());
   for (auto& [meshId, _] : modelData.meshIdToMeshData){
     meshIds.push_back(meshId);
   }
   std::sort(meshIds.begin(), meshIds.end());
   for (int32_t meshId : meshIds){
     auto& meshData = modelData.meshIdToMeshData.at(meshId);
     rapidjson::Value mesh(rapidjson::kArrayType);
     mesh.PushBack(meshId, allocator);

     rapidjson::Value data(rapidjson::kObjectType);
     rapidjson::Value vertices(rapidjson::kArrayType);
     for (auto& vertex : meshData.vertices){
       rapidjson::Value vertexData(rapidjson::kArrayType);
       vertexData.PushBack(vec3ToJson(vertex.position, allocator), allocator);
       vertexData.PushBack(vec3ToJson(vertex.normal, allocator), allocator);
       vertexData.PushBack(vec3ToJson(vertex.tangent, allocator), allocator);
       vertexData.PushBack(vec3ToJson(vertex.color, allocator), allocator);
       vertexData.PushBack(vec2ToJson(vertex.texCoords, allocator), allocator);

       rapidjson::Value boneIndexes(rapidjson::kArrayType);
       rapidjson::Value boneWeights(rapidjson::kArrayType);
       for (int i = 0; i < NUM_BONES_PER_VERTEX; i++){
         boneIndexes.PushBack(vertex.boneIndexes[i], allocator);
         modassert(std::isfinite(vertex.boneWeights[i]), "saveModelData found non-finite vertex bone weight");
         boneWeights.PushBack(vertex.boneWeights[i], allocator);
       }
       vertexData.PushBack(boneIndexes, allocator);
       vertexData.PushBack(boneWeights, allocator);
       vertices.PushBack(vertexData, allocator);
     }
     data.AddMember("vertices", vertices, allocator);

     rapidjson::Value indices(rapidjson::kArrayType);
     for (auto index : meshData.indices){
       indices.PushBack(index, allocator);
     }
     data.AddMember("indices", indices, allocator);

     rapidjson::Value bones(rapidjson::kArrayType);
     for (auto& bone : meshData.bones){
       rapidjson::Value boneData(rapidjson::kObjectType);
       boneData.AddMember("name", rapidjson::Value(bone.name, allocator), allocator);
       boneData.AddMember("shortName", rapidjson::Value(bone.shortName, allocator), allocator);
       boneData.AddMember("offsetMatrix", mat4ToJson(bone.offsetMatrix, allocator), allocator);
       boneData.AddMember("initialBonePoseInverse", mat4ToJson(bone.initialBonePoseInverse, allocator), allocator);

       rapidjson::Value initialTransform(rapidjson::kArrayType);
       initialTransform.PushBack(vec3ToJson(bone.initialLocalTransform.position, allocator), allocator);
       initialTransform.PushBack(vec3ToJson(bone.initialLocalTransform.scale, allocator), allocator);
       initialTransform.PushBack(vec4ToJson(serializeQuatToVec4(bone.initialLocalTransform.rotation), allocator), allocator);
       boneData.AddMember("initialLocalTransform", initialTransform, allocator);
       bones.PushBack(boneData, allocator);
     }
     data.AddMember("bones", bones, allocator);

     data.AddMember("diffuseTexturePath", rapidjson::Value(meshData.diffuseTexturePath, allocator), allocator);
     data.AddMember("hasDiffuseTexture", meshData.hasDiffuseTexture, allocator);
     data.AddMember("emissionTexturePath", rapidjson::Value(meshData.emissionTexturePath, allocator), allocator);
     data.AddMember("hasEmissionTexture", meshData.hasEmissionTexture, allocator);
     data.AddMember("opacityTexturePath", rapidjson::Value(meshData.opacityTexturePath, allocator), allocator);
     data.AddMember("hasOpacityTexture", meshData.hasOpacityTexture, allocator);
     data.AddMember("roughnessTexturePath", rapidjson::Value(meshData.roughnessTexturePath, allocator), allocator);
     data.AddMember("hasRoughnessTexture", meshData.hasRoughnessTexture, allocator);
     data.AddMember("normalTexturePath", rapidjson::Value(meshData.normalTexturePath, allocator), allocator);
     data.AddMember("hasNormalTexture", meshData.hasNormalTexture, allocator);

     rapidjson::Value bounds(rapidjson::kArrayType);
     bounds.PushBack(meshData.boundInfo.xMin, allocator);
     bounds.PushBack(meshData.boundInfo.xMax, allocator);
     bounds.PushBack(meshData.boundInfo.yMin, allocator);
     bounds.PushBack(meshData.boundInfo.yMax, allocator);
     bounds.PushBack(meshData.boundInfo.zMin, allocator);
     bounds.PushBack(meshData.boundInfo.zMax, allocator);
     data.AddMember("bounds", bounds, allocator);
     data.AddMember("isSky", meshData.isSky, allocator);
     data.AddMember("isWater", meshData.isWater, allocator);
     data.AddMember("isHidden", meshData.isHidden, allocator);

     mesh.PushBack(data, allocator);
     meshes.PushBack(mesh, allocator);
   }
   doc.AddMember("meshes", meshes, allocator);
  }

  {
   rapidjson::Value ids(rapidjson::kArrayType);
   rapidjson::Value meshIdsForNodes(rapidjson::kArrayType);
   rapidjson::Value parentIds(rapidjson::kArrayType);
   rapidjson::Value transforms(rapidjson::kArrayType);
   rapidjson::Value names(rapidjson::kArrayType);

    std::vector<int32_t> nodeIds;
    nodeIds.reserve(modelData.nodeTransform.size());
    for (auto& [id, _] : modelData.nodeTransform){
      nodeIds.push_back(id);
    }
    std::sort(nodeIds.begin(), nodeIds.end());
    for (int32_t id : nodeIds){
      auto& transform = modelData.nodeTransform.at(id);
      ids.PushBack(id, allocator);

      rapidjson::Value meshIdsForNode(rapidjson::kArrayType);
      for (auto meshId : modelData.nodeToMeshId.at(id)){
        meshIdsForNode.PushBack(meshId, allocator);
      }
      meshIdsForNodes.PushBack(meshIdsForNode, allocator);

      auto parent = modelData.childToParent.find(id);
      if (parent == modelData.childToParent.end()){
        parentIds.PushBack(rapidjson::Value(), allocator);
      }else{
        parentIds.PushBack(parent->second, allocator);
      }

      rapidjson::Value transformValues(rapidjson::kArrayType);
      transformValues.PushBack(vec3ToJson(transform.position, allocator), allocator);
      transformValues.PushBack(vec3ToJson(transform.scale, allocator), allocator);
      transformValues.PushBack(vec4ToJson(serializeQuatToVec4(transform.rotation), allocator), allocator);
      transforms.PushBack(transformValues, allocator);

      names.PushBack(rapidjson::Value(modelData.names.at(id), allocator), allocator);
    }

    doc.AddMember("id", ids, allocator);
    doc.AddMember("meshids", meshIdsForNodes, allocator);
    doc.AddMember("parent", parentIds, allocator);
    doc.AddMember("transform", transforms, allocator);
    doc.AddMember("name", names, allocator);
  }

  // animation 
  {
    rapidjson::Value animations(rapidjson::kArrayType);
    for (auto& animation : modelData.animations){
      rapidjson::Value values(rapidjson::kArrayType);

      values.PushBack(rapidjson::Value(animation.name, allocator), allocator);
      values.PushBack(animation.duration, allocator);
      values.PushBack(animation.ticksPerSecond, allocator);

      rapidjson::Value channels(rapidjson::kArrayType);
      for(auto& animationChannel : animation.channels){
        rapidjson::Value channel(rapidjson::kArrayType);
        channel.PushBack(rapidjson::Value(animationChannel.nodeName, allocator), allocator);
        {
          rapidjson::Value positionKeys(rapidjson::kArrayType);
          for (auto& positionKey : animationChannel.positionKeys){
            rapidjson::Value key(rapidjson::kArrayType);
            key.PushBack(positionKey.mTime, allocator);
            key.PushBack(vec3ToJson(aiVectorToGlm(positionKey.mValue), allocator), allocator);
            positionKeys.PushBack(key, allocator);
          }
          channel.PushBack(positionKeys, allocator);
        }
        {
          rapidjson::Value scalingKeys(rapidjson::kArrayType);
          for (auto& scalingKey : animationChannel.scalingKeys){
            rapidjson::Value key(rapidjson::kArrayType);
            key.PushBack(scalingKey.mTime, allocator);
            key.PushBack(vec3ToJson(aiVectorToGlm(scalingKey.mValue), allocator), allocator);
            scalingKeys.PushBack(key, allocator);
          }
          channel.PushBack(scalingKeys, allocator);
        }

        {
          rapidjson::Value rotationKeys(rapidjson::kArrayType);
          for (auto& rotationKey : animationChannel.rotationKeys){
            rapidjson::Value key(rapidjson::kArrayType);
            key.PushBack(rotationKey.mTime, allocator);
            key.PushBack(vec4ToJson(serializeQuatToVec4(aiQuatToGlm(rotationKey.mValue)), allocator), allocator);
            rotationKeys.PushBack(key, allocator);
          }
          channel.PushBack(rotationKeys, allocator);    
        }

        channels.PushBack(channel, allocator);
      }
      values.PushBack(channels, allocator);

      animations.PushBack(values, allocator);
    }
    doc.AddMember("animation", animations, allocator);
  }
  doc.AddMember("sponsorRootPosition", modelData.sponsorRootPosition, allocator);

  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  modassert(doc.Accept(writer), "saveModelData could not serialize model data as valid JSON");
  realfiles::saveFile(filepath, buffer.GetString());
}

double readNumber(const rapidjson::Value& value, std::string context){
  modassert(value.IsNumber(), context + " must be numeric");
  double number = value.GetDouble();
  modassert(std::isfinite(number), context + " must be finite");
  return number;
}

int32_t readInt32(const rapidjson::Value& value, std::string context){
  modassert(value.IsInt(), context + " must be a 32-bit integer");
  return value.GetInt();
}

std::string readString(const rapidjson::Value& value, std::string context){
  modassert(value.IsString(), context + " must be a string");
  return value.GetString();
}

bool readBool(const rapidjson::Value& value, std::string context){
  modassert(value.IsBool(), context + " must be a boolean");
  return value.GetBool();
}

void requireArraySize(const rapidjson::Value& value, rapidjson::SizeType size, std::string context){
  modassert(value.IsArray() && value.Size() == size, context + " must be an array of size " + std::to_string(size));
}

float readFloat(const rapidjson::Value& value, std::string context){
  double number = readNumber(value, context);
  modassert(number >= -std::numeric_limits<float>::max() && number <= std::numeric_limits<float>::max(),
      context + " is outside the float range");
  return static_cast<float>(number);
}

glm::vec2 readVec2(const rapidjson::Value& value, std::string context){
  requireArraySize(value, 2, context);
  return glm::vec2(readFloat(value[0], context), readFloat(value[1], context));
}

glm::vec3 readVec3(const rapidjson::Value& value, std::string context){
  requireArraySize(value, 3, context);
  return glm::vec3(
      readFloat(value[0], context),
      readFloat(value[1], context),
      readFloat(value[2], context));
}

glm::vec4 readVec4(const rapidjson::Value& value, std::string context){
  requireArraySize(value, 4, context);
  return glm::vec4(
      readFloat(value[0], context),
      readFloat(value[1], context),
      readFloat(value[2], context),
      readFloat(value[3], context));
}

glm::quat readQuaternion(const rapidjson::Value& value, std::string context){
  glm::vec4 encoded = readVec4(value, context);
  float directionLength = glm::length(glm::vec3(encoded));
  modassert(std::isfinite(directionLength) && directionLength > 1e-6f,
      context + " has an invalid direction");
  return parseQuat(encoded);
}

glm::mat4 readMat4(const rapidjson::Value& value, std::string context){
  requireArraySize(value, 4, context);
  glm::mat4 matrix(1.0f);
  for (rapidjson::SizeType column = 0; column < 4; column++){
    glm::vec4 values = readVec4(value[column], context + " column " + std::to_string(column));
    for (rapidjson::SizeType row = 0; row < 4; row++){
      matrix[column][row] = values[row];
    }
  }
  return matrix;
}

Transformation readTransformation(const rapidjson::Value& value, std::string context){
  requireArraySize(value, 3, context);
  Transformation transform {
    .position = readVec3(value[0], context + " position"),
    .scale = readVec3(value[1], context + " scale"),
    .rotation = readQuaternion(value[2], context + " rotation"),
  };
  return transform;
}

ModelData loadModelData(std::string filepath){
  ModelData modelData{};

  std::string fileContent = realfiles::doLoadFile(filepath);
  rapidjson::Document doc;
  doc.Parse(fileContent.c_str());
  modassert(!doc.HasParseError(), "could not parse " + filepath);
  modassert(doc.IsObject(), "root must be an object");

  modassert(doc.HasMember("meshes"), "missing member meshes");
  auto& meshes = doc["meshes"];
  modassert(meshes.IsArray(), "meshes must be an array");
  for (rapidjson::SizeType meshIndex = 0; meshIndex < meshes.Size(); meshIndex++){
    MeshData meshData{};

    auto& meshEntry = meshes[meshIndex];
    requireArraySize(meshEntry, 2, "mesh entry");
    int32_t meshId = readInt32(meshEntry[0], "mesh id");
    auto& data = meshEntry[1];
    modassert(data.IsObject(), "mesh data must be an object");
    modassert(data.HasMember("vertices"), "missing member vertices");
    modassert(data.HasMember("indices"), "missing member indices");
    modassert(data.HasMember("bones"), "missing member bones");
    modassert(data.HasMember("diffuseTexturePath"), "missing member diffuseTexturePath");
    modassert(data.HasMember("hasDiffuseTexture"), "missing member hasDiffuseTexture");
    modassert(data.HasMember("emissionTexturePath"), "missing member emissionTexturePath");
    modassert(data.HasMember("hasEmissionTexture"), "missing member hasEmissionTexture");
    modassert(data.HasMember("opacityTexturePath"), "missing member opacityTexturePath");
    modassert(data.HasMember("hasOpacityTexture"), "missing member hasOpacityTexture");
    modassert(data.HasMember("roughnessTexturePath"), "missing member roughnessTexturePath");
    modassert(data.HasMember("hasRoughnessTexture"), "missing member hasRoughnessTexture");
    modassert(data.HasMember("normalTexturePath"), "missing member normalTexturePath");
    modassert(data.HasMember("hasNormalTexture"), "missing member hasNormalTexture");
    modassert(data.HasMember("bounds"), "missing member bounds");
    modassert(data.HasMember("isSky"), "missing member isSky");
    modassert(data.HasMember("isWater"), "missing member isWater");
    modassert(data.HasMember("isHidden"), "missing member isHidden");

    {
      auto& vertices = data["vertices"];
      modassert(vertices.IsArray(), "mesh vertices must be an array");
      for (rapidjson::SizeType vertexIndex = 0; vertexIndex < vertices.Size(); vertexIndex++){
        auto& vertexData = vertices[vertexIndex];
        requireArraySize(vertexData, 7, "vertex");
        Vertex vertex{};
        vertex.position = readVec3(vertexData[0], "vertex position");
        vertex.normal = readVec3(vertexData[1], "vertex normal");
        vertex.tangent = readVec3(vertexData[2], "vertex tangent");
        vertex.color = readVec3(vertexData[3], "vertex color");
        vertex.texCoords = readVec2(vertexData[4], "vertex texture coordinates");
        requireArraySize(vertexData[5], NUM_BONES_PER_VERTEX, "vertex bone indices");
        requireArraySize(vertexData[6], NUM_BONES_PER_VERTEX, "vertex bone weights");
        for (int boneIndex = 0; boneIndex < NUM_BONES_PER_VERTEX; boneIndex++){
          vertex.boneIndexes[boneIndex] = readInt32(vertexData[5][boneIndex], "vertex bone index");
          vertex.boneWeights[boneIndex] = readFloat(vertexData[6][boneIndex], "vertex bone weight");
        }
        meshData.vertices.push_back(vertex);
      }
    }
    {
      auto& indices = data["indices"];
      modassert(indices.IsArray(), "mesh indices must be an array");
      for (rapidjson::SizeType index = 0; index < indices.Size(); index++){
        auto& value = indices[index];
        modassert(value.IsUint(), "mesh index must be an unsigned integer");
        unsigned int meshVertexIndex = value.GetUint();
        modassert(meshVertexIndex < meshData.vertices.size(), "mesh index references a missing vertex");
        meshData.indices.push_back(meshVertexIndex);
      }
    }

    {
      auto& bones = data["bones"];
      modassert(bones.IsArray(), "mesh bones must be an array");
      for (rapidjson::SizeType boneIndex = 0; boneIndex < bones.Size(); boneIndex++){
        auto& boneData = bones[boneIndex];
        modassert(boneData.IsObject(), "bone data must be an object");
        modassert(boneData.HasMember("name"), "missing member name");
        modassert(boneData.HasMember("shortName"), "missing member shortName");
        modassert(boneData.HasMember("offsetMatrix"), "missing member offsetMatrix");
        modassert(boneData.HasMember("initialBonePoseInverse"), "missing member initialBonePoseInverse");
        modassert(boneData.HasMember("initialLocalTransform"), "missing member initialLocalTransform");
        Bone bone{};
        bone.name = readString(boneData["name"], "bone name");
        bone.shortName = readString(boneData["shortName"], "bone short name");
        bone.offsetMatrix = readMat4(boneData["offsetMatrix"], "bone offset matrix");
        bone.initialBonePoseInverse = readMat4(boneData["initialBonePoseInverse"], "bone initial pose inverse");
        bone.initialLocalTransform = readTransformation(boneData["initialLocalTransform"], "bone initial local transform");
        meshData.bones.push_back(bone);
      }
    }

    meshData.diffuseTexturePath = readString(data["diffuseTexturePath"], "diffuse texture path");
    meshData.hasDiffuseTexture = readBool(data["hasDiffuseTexture"], "has diffuse texture");
    meshData.emissionTexturePath = readString(data["emissionTexturePath"], "emission texture path");
    meshData.hasEmissionTexture = readBool(data["hasEmissionTexture"], "has emission texture");
    meshData.opacityTexturePath = readString(data["opacityTexturePath"], "opacity texture path");
    meshData.hasOpacityTexture = readBool(data["hasOpacityTexture"], "has opacity texture");
    meshData.roughnessTexturePath = readString(data["roughnessTexturePath"], "roughness texture path");
    meshData.hasRoughnessTexture = readBool(data["hasRoughnessTexture"], "has roughness texture");
    meshData.normalTexturePath = readString(data["normalTexturePath"], "normal texture path");
    meshData.hasNormalTexture = readBool(data["hasNormalTexture"], "has normal texture");

    auto& bounds = data["bounds"];
    requireArraySize(bounds, 6, "mesh bounds");
    meshData.boundInfo = {
      .xMin = readFloat(bounds[0], "bounds xMin"),
      .xMax = readFloat(bounds[1], "bounds xMax"),
      .yMin = readFloat(bounds[2], "bounds yMin"),
      .yMax = readFloat(bounds[3], "bounds yMax"),
      .zMin = readFloat(bounds[4], "bounds zMin"),
      .zMax = readFloat(bounds[5], "bounds zMax"),
    };
    meshData.isSky = readBool(data["isSky"], "mesh isSky");
    meshData.isWater = readBool(data["isWater"], "mesh isWater");
    meshData.isHidden = readBool(data["isHidden"], "mesh isHidden");

    modelData.meshIdToMeshData[meshId] = std::move(meshData);
  }

  modassert(doc.HasMember("id"), "missing member id");
  auto& ids = doc["id"];
  modassert(ids.IsArray(), "id must be an array");
  auto nodeCount = ids.Size();

  modassert(doc.HasMember("meshids"), "missing member meshids");
  auto& meshIdsForNodes = doc["meshids"];
  modassert(meshIdsForNodes.IsArray() && meshIdsForNodes.Size() == nodeCount, "meshids must align with id");

  modassert(doc.HasMember("parent"), "missing member parent");
  auto& parentIds = doc["parent"];
  modassert(parentIds.IsArray() && parentIds.Size() == nodeCount, "parent must align with id");

  modassert(doc.HasMember("transform"), "missing member transform");
  auto& transforms = doc["transform"];
  modassert(transforms.IsArray() && transforms.Size() == nodeCount, "transform must align with id");

  modassert(doc.HasMember("name"), "missing member name");
  auto& names = doc["name"];
  modassert(names.IsArray() && names.Size() == nodeCount, "name must align with id");

  for (rapidjson::SizeType nodeIndex = 0; nodeIndex < nodeCount; nodeIndex++){
    int32_t id = readInt32(ids[nodeIndex], "node id");
    Transformation transform = readTransformation(transforms[nodeIndex], "node transform");
    std::string name = readString(names[nodeIndex], "node name");

    modassert(meshIdsForNodes[nodeIndex].IsArray(), "node mesh ids must be an array");
    std::vector<int> nodeMeshIds;
    for (rapidjson::SizeType meshIndex = 0; meshIndex < meshIdsForNodes[nodeIndex].Size(); meshIndex++){
      int32_t meshId = readInt32(meshIdsForNodes[nodeIndex][meshIndex], "node mesh id");
      modassert(modelData.meshIdToMeshData.find(meshId) != modelData.meshIdToMeshData.end(), "node references missing mesh id " + std::to_string(meshId));
      nodeMeshIds.push_back(meshId);
    }

    modassert(modelData.nodeTransform.emplace(id, transform).second, "duplicate node id " + std::to_string(id));
    modelData.nodeToMeshId.emplace(id, std::move(nodeMeshIds));
    modelData.names.emplace(id, std::move(name));

    auto& parent = parentIds[nodeIndex];
    if (!parent.IsNull()){
      modelData.childToParent.emplace(id, readInt32(parent, "node parent id"));
    }
  }
  for (auto& [nodeId, parentId] : modelData.childToParent){
    modassert(modelData.nodeTransform.find(parentId) != modelData.nodeTransform.end(), "node " + std::to_string(nodeId) + " references missing parent " + std::to_string(parentId));
  }

  modassert(doc.HasMember("animation"), "missing member animation");
  auto& animations = doc["animation"];
  modassert(animations.IsArray(), "animation must be an array");
  for (rapidjson::SizeType animationIndex = 0; animationIndex < animations.Size(); animationIndex++){
    auto& animationData = animations[animationIndex];
    requireArraySize(animationData, 4, "animation");
    Animation animation {
      .name = readString(animationData[0], "animation name"),
      .duration = readNumber(animationData[1], "animation duration"),
      .ticksPerSecond = readNumber(animationData[2], "animation ticks per second"),
    };

    auto& channels = animationData[3];
    modassert(channels.IsArray(), "animation channels must be an array");
    for (rapidjson::SizeType channelIndex = 0; channelIndex < channels.Size(); channelIndex++){
      auto& channelData = channels[channelIndex];
      requireArraySize(channelData, 4, "animation channel");
      AnimationChannel channel;
      channel.nodeName = readString(channelData[0], "animation channel node name");

      auto& positionKeys = channelData[1];
      modassert(positionKeys.IsArray(), "position keys must be an array");
      for (rapidjson::SizeType keyIndex = 0; keyIndex < positionKeys.Size(); keyIndex++){
        auto& key = positionKeys[keyIndex];
        requireArraySize(key, 2, "position key");
        double time = readNumber(key[0], "position key time");
        glm::vec3 value = readVec3(key[1], "position key value");
        channel.positionKeys.emplace_back(time, aiVector3D(value.x, value.y, value.z));
      }

      auto& scalingKeys = channelData[2];
      modassert(scalingKeys.IsArray(), "scale keys must be an array");
      for (rapidjson::SizeType keyIndex = 0; keyIndex < scalingKeys.Size(); keyIndex++){
        auto& key = scalingKeys[keyIndex];
        requireArraySize(key, 2, "scale key");
        double time = readNumber(key[0], "scale key time");
        glm::vec3 value = readVec3(key[1], "scale key value");
        channel.scalingKeys.emplace_back(time, aiVector3D(value.x, value.y, value.z));
      }

      auto& rotationKeys = channelData[3];
      modassert(rotationKeys.IsArray(), "rotation keys must be an array");
      for (rapidjson::SizeType keyIndex = 0; keyIndex < rotationKeys.Size(); keyIndex++){
        auto& key = rotationKeys[keyIndex];
        requireArraySize(key, 2, "rotation key");
        double time = readNumber(key[0], "rotation key time");
        glm::quat value = readQuaternion(key[1], "rotation key value");
        channel.rotationKeys.emplace_back(time, aiQuaternion(value.w, value.x, value.y, value.z));
      }

      animation.channels.push_back(std::move(channel));
    }
    modelData.animations.push_back(std::move(animation));
  }

  std::set<std::string> boneNames;
  for (auto& [_, meshData] : modelData.meshIdToMeshData){
    for (auto& bone : meshData.bones){
      boneNames.insert(bone.name);
    }
  }
  for (auto& [id, name] : modelData.names){
    if (boneNames.count(name) > 0 || name.find("mixamorig") != std::string::npos || name.find("Armature") != std::string::npos){
      modelData.bones.insert(id);
    }
  }

  modassert(doc.HasMember("sponsorRootPosition"), "missing member sponsorRootPosition");
  modelData.sponsorRootPosition = readBool(doc["sponsorRootPosition"], "sponsorRootPosition");
  return modelData;
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
std::optional<int> meshIdFromName(std::string& meshname){
  auto rootAndRest = carAndRest(meshname, '=');
  auto second = rootAndRest.second;
  if (second == ""){
    return std::nullopt;
  }
  return std::atoi(second.c_str());
}

std::vector<std::string> meshNamesForNode(ModelData& modelData, std::string& rootmesh, std::string nodeName){
  std::vector<std::string> meshnames;
  auto meshIds = modelData.nodeToMeshId.at(nodeIdFromName(modelData, nodeName));
  for (auto meshId : meshIds){
    meshnames.push_back(nameForMeshId(rootmesh, meshId));
  }
  return meshnames;
}