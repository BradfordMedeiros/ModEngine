#include "./playback.h"

// BRAD -> create an animation with and xyz offset, and make sure the frame is matching what we are using and not flipping x/y/z anywhere

glm::mat4 getInitialPose(Bone& bone, std::function<glm::mat4(std::string, std::string)>& getModelMatrix, std::string rootname){
  auto boneTransform =  getModelMatrix(bone.name, rootname);
  if (!bone.initialPoseSet){
    auto initPose = getTransformationFromMatrix(bone.initialBonePose);
    //printMatrixInformation(bone.initialBonePose, std::string("BONEINFO_INIT") + bone.name);
    bone.initialBonePose = boneTransform;
    bone.initialPoseSet = true;
    //sprintMatrixInformation(bone.initialBonePose, std::string("BONEINFO_ANIM") + bone.name);
  }
  return bone.initialBonePose;
}

// https://gamedev.net/forums/topic/484984-skeletal-animation-non-uniform-scale/4172731/
void updateBonePoses(NameAndMeshObjName meshNameToMeshes, std::function<glm::mat4(std::string, std::string)> getModelMatrix, std::string rootname){
  std::cout << "rootname: " << rootname << std::endl;
  for (int i = 0; i <  meshNameToMeshes.meshes.size(); i++){
    Mesh& mesh = meshNameToMeshes.meshes.at(i);
    std::string meshname = meshNameToMeshes.objnames.at(i);
    for (Bone& bone : mesh.bones){
      bone.offsetMatrix = getModelMatrix(bone.name, rootname) * glm::inverse(getInitialPose(bone, getModelMatrix, rootname));
    }
  }
}

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

  updateBonePoses(meshNameToMeshes, getModelMatrix, rootname); // just for now so we set initial poses as side effect

  for (auto pose : posesForTick){
    printMatrixInformation(pose.pose, std::string("SET_CHANNEL:") + pose.channelName);
    setPose(pose.channelName, pose.pose);
  }
  updateBonePoses(meshNameToMeshes, getModelMatrix, rootname);
}