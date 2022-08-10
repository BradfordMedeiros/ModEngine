#include "./obj_voxel.h"

std::vector<AutoSerialize> voxelAutoserializer {
 
};

GameObjectVoxel createVoxel(GameobjAttributes& attr, ObjectTypeUtil& util){
  auto defaultVoxelTexture = util.ensureTextureLoaded("./res/textures/wood.jpg").textureId;
  auto textureString = attr.stringAttributes.find("fromtextures") == attr.stringAttributes.end() ? "" : attr.stringAttributes.at("fromtextures");
  auto voxel = createVoxels(parseVoxelState(attr.stringAttributes.at("from"), textureString, defaultVoxelTexture, util.ensureTextureLoaded), util.onCollisionChange, defaultVoxelTexture);
  GameObjectVoxel obj {
    .voxel = voxel,
  };
  createAutoSerialize((char*)&obj, voxelAutoserializer, attr, util);
  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeVoxel(GameObjectVoxel& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  auto serializedData = serializeVoxelState(obj.voxel, util.textureName);

  pairs.push_back(std::pair<std::string, std::string>("from", serializedData.voxelState));
  if (serializedData.textureState != ""){
    pairs.push_back(std::pair<std::string, std::string>("fromtextures", serializedData.textureState));
  }
  return pairs;
}  