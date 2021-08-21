#include "./playback.h"

// BRAD -> create an animation with and xyz offset, and make sure the frame is matching what we are using and not flipping x/y/z anywhere

glm::mat4 getInitialPose(Bone& bone, std::function<glm::mat4(std::string, std::string)>& getModelMatrix){
  auto boneTransform =  getModelMatrix(bone.name, bone.skeletonBase);
  if (!bone.initialPoseSet){
    auto initPose = getTransformationFromMatrix(bone.initialBonePose);
    printMatrixInformation(bone.initialBonePose, std::string("BONEINFO_INIT") + bone.name);
    bone.initialBonePose = boneTransform;
    bone.initialPoseSet = true;
    printMatrixInformation(bone.initialBonePose, std::string("BONEINFO_ANIM") + bone.name);
  }
  return bone.initialBonePose;
}

void updateBonePoses(NameAndMeshObjName meshNameToMeshes, std::function<glm::mat4(std::string, std::string)> getModelMatrix, std::string rootname){
  std::cout << "rootname: " << rootname << std::endl;
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    std::string meshname = meshNameToMeshes.objnames.at(i);
    for (Bone& bone : mesh.bones){
      std::cout << "skeletonBase is: " << bone.skeletonBase << std::endl;
      auto meshTransform = getModelMatrix(meshname, rootname);
      //auto meshTransform = getModelMatrix("targetmodel/sentinel", "targetmodel");

      //auto boneOffsetMatrix = getModelMatrix(bone.name, bone.skeletonBase) * glm::inverse(getInitialPose(bone, getModelMatrix));
      auto boneOffsetMatrix =  glm::inverse(glm::inverse(getInitialPose(bone, getModelMatrix)) * getModelMatrix(bone.name, bone.skeletonBase));
      printMatrixInformation(boneOffsetMatrix, std::string("BONEINFO_OFFSET") + bone.name);

      std::cout << "mesh name is : " << meshname << std::endl;

      bone.offsetMatrix = boneOffsetMatrix;
    }
  }
}

/*void updateBonePoses(NameAndMesh meshNameToMeshes, std::function<glm::mat4(std::string, std::string)> getModelMatrix, std::function<glm::mat4(Bone&)> getInitialPose){
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    auto meshName = meshNameToMeshes.meshNames.at(i);
    for (Bone& bone : mesh.bones){
      bone.offsetMatrix =  glm::inverse(getModelMatrix("move/Cube", "move")) * (
        getModelMatrix(bone.name, bone.skeletonBase) * 
        glm::inverse(
          getInitialPose(bone)
        )
      );
    }
  }
}*/

void playbackAnimation(
  Animation animation,  
  NameAndMeshObjName meshNameToMeshes,  
  float currentTime, 
  float elapsedTime, 
  std::function<glm::mat4(std::string, std::string)> getModelMatrix,
  std::function<void(std::string, glm::mat4)> setPose,
  std::string rootname
){  
  auto posesForTick = animationPosesAtTime(animation, currentTime, elapsedTime);
  for (auto pose : posesForTick){
    setPose(pose.channelName, pose.pose);
  }
  updateBonePoses(meshNameToMeshes, getModelMatrix, rootname);
}