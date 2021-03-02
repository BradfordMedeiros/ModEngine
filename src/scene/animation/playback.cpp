#include "./playback.h"

bool boneExistsInMesh(std::string boneName, NameAndMesh& meshData){
  for (int i = 0; i < meshData.meshes.size(); i++){
    Mesh& mesh = meshData.meshes.at(i);
    for (Bone& bone : mesh.bones){
      if (bone.name == boneName){    
        return true;
      }
    }
  }
  return false;
}

glm::mat4 getOffsetMatrix(std::string bonename, NameAndMesh meshNameToMeshes){
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    std::string meshName = meshNameToMeshes.meshNames.at(i);
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    for (Bone& bone : mesh.bones){
      if (bone.name == bonename){
        return bone.initialOffsetMatrix;
      }
    }
  }
  assert(false);
  return glm::mat4(1.f);
}


std::map<std::string, glm::mat4> initialPose; 
bool shouldSetPose = false;
void playbackAnimation(
  Animation animation,  
  std::map<std::string, std::map<std::string, std::string>>& meshnameToBoneToParent, 
  NameAndMesh meshNameToMeshes,  
  float currentTime, 
  float elapsedTime, 
  std::function<glm::mat4(std::string, bool)> getModelMatrix,
  std::function<void(std::string, glm::mat4)> setPose
){  
  std::map<std::string, glm::mat4> nodeToTransformedPose;
  auto posesForTick = animationPosesAtTime(animation, currentTime, elapsedTime);
  for (auto pose : posesForTick){
    if (shouldSetPose){
      setPose(pose.channelName, pose.pose);
    }
  }
  shouldSetPose = true;

  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    std::string meshName = meshNameToMeshes.meshNames.at(i);
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    auto boneToParent = meshnameToBoneToParent.at(meshName);


    bool notMatching = false;
    std::cout << "=====================" << std::endl;

    auto topModel =  getModelMatrix("onenodewithanimation", true);
    auto armatureModel =  getModelMatrix("Armature", true);
    auto cubeModel =  getModelMatrix("Cube", true);

    std::cout << "top model: " << print(topModel) << std::endl;
    std::cout << "armature model: " << print(armatureModel) << std::endl;
    std::cout << "cube model: " << print(cubeModel) << std::endl;

    for (Bone& bone : mesh.bones){
      auto boneTransform =  getModelMatrix(bone.name, true);
      if (initialPose.find(bone.name) == initialPose.end()){
        initialPose[bone.name] = boneTransform;
      }
      if (bone.offsetMatrix != boneTransform){
        std::cout << "bone name: " << bone.name << std::endl;
        std::cout << "Offset matrix: " << print(bone.initialOffsetMatrix) << std::endl;
        std::cout << "inverse offset matrix: " << print(glm::inverse(bone.initialOffsetMatrix)) << std::endl << std::endl;
        std::cout << "bone transform: " << bone.name << "-- " << print(boneTransform) << std::endl << std::endl;
        //std::cout << "Value * inverse: " << print(boneTransform * glm::inverse(bone.initialOffsetMatrix)) << std::endl << std::endl;
        //std::cout << "Value * reg: " << print(boneTransform * bone.initialOffsetMatrix) << std::endl << std::endl;

        notMatching = true;
      }


      auto hasParent = boneToParent.find(bone.name) != boneToParent.end();
      //auto parentTransform = hasParent ? getOffsetMatrix(boneToParent.at(bone.name), meshNameToMeshes) : glm::mat4(1.f);
      
      auto parentTransform = getModelMatrix("Armature", true);

      std::cout << "parent transform: " << print(parentTransform) << std::endl << std::endl;


      //auto parentTransform2 = hasParent ? getModelMatrix(boneToParent.at(bone.name), false) : glm::mat4(1.f);
      //auto transform = getModelMatrix(bone.name, false);
      // So think about it like this
      // You have the actual position of the bone.  That's the final position, which takes into account parenting and whatnot.  It's actual 
      // object space position right
      // Now that's relative to the bind pose.  Even if the hands are up and shit, you still need this result to be glm::mat4(1.f) to be able 
      // to get the bind pose
      // This means that we need this matrix, relative to whatever the bind pose is.  
      // So effectively whatever the nodes position is you multiply times this offset matrix that cancels out the effect of being relative to the bind pose

      bone.offsetMatrix =  glm::inverse(parentTransform) * boneTransform;
    }
    //assert(!notMatching);
  }
}
