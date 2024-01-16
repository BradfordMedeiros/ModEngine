#include "./obj_octree.h"

std::vector<AutoSerialize> octreeAutoserializer {
 
};

GameObjectOctree createOctree(GameobjAttributes& attr, ObjectTypeUtil& util){
  GameObjectOctree obj {
  };
  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeOctree(GameObjectOctree& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
//  auto serializedData = serializeVoxelState(obj.voxel, util.textureName);
//
//  pairs.push_back(std::pair<std::string, std::string>("from", serializedData.voxelState));
//  if (serializedData.textureState != ""){
//    pairs.push_back(std::pair<std::string, std::string>("fromtextures", serializedData.textureState));
//  }
  return pairs;
}  //