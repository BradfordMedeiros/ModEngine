#include "./main_input.h"

extern World world;
extern engineState state;
extern SchemeBindingCallbacks schemeBindings;
extern bool disableInput;
extern GameObjectVoxel* voxelPtr;
extern KeyRemapper keyMapper;
extern bool useYAxis;

float maskValues[] { 10.f };
HeightmapMask mask {
  .values = maskValues,
  .width = 1,
  .height = 1,
};

void processManipulator(){
  if (state.enableManipulator && state.selectedIndex != -1 && idExists(world, state.selectedIndex)){
    auto selectObject = getGameObject(world, state.selectedIndex); 
    if (state.manipulatorMode == TRANSLATE){
      applyPhysicsTranslation(world, state.selectedIndex, selectObject.transformation.position, state.offsetX, state.offsetY, state.manipulatorAxis);
    }else if (state.manipulatorMode == SCALE){
      applyPhysicsScaling(world, state.selectedIndex, selectObject.transformation.position, selectObject.transformation.scale, state.lastX, state.lastY, state.offsetX, state.offsetY, state.manipulatorAxis);
    }else if (state.manipulatorMode == ROTATE){
      applyPhysicsRotation(world, state.selectedIndex, selectObject.transformation.rotation, state.offsetX, state.offsetY, state.manipulatorAxis);
    }
  }
}

void onMouseEvents(GLFWwindow* window, double xpos, double ypos){
  onMouse(disableInput, window, state, xpos, ypos, rotateCamera);  
  schemeBindings.onMouseMoveCallback(state.offsetX, state.offsetY); 
  processManipulator();
}

void onArrowKey(int key){}

void maybeApplyTextureOffset(int index, glm::vec2 offset){
  GameObjectMesh* meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(index));
  if (meshObj == NULL){
    return;
  }

  for (auto id : getIdsInGroup(world, index)){
    GameObjectMesh* meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(id));
    assert(meshObj != NULL);
    meshObj -> texture.textureoffset = meshObj -> texture.textureoffset + offset;
  }
}

struct TextureAndName {
  Texture texture;
  std::string textureName;
};
std::vector<TextureAndName> worldTextures(World& world){
  std::vector<TextureAndName> textures;
  for (auto [textureName, texture] : world.textures){
    textures.push_back(TextureAndName{
      .texture = texture,
      .textureName = textureName
    });
  }
  return textures;
}
void maybeChangeTexture(int index){
    GameObjectMesh* meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(index));
    if (meshObj == NULL){
      return;
    }

    auto textures = worldTextures(world);
    for (auto id : getIdsInGroup(world, index)){
      GameObjectMesh* meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(id));
      assert(meshObj != NULL);

      int overloadId = 0;
      for (int i = 0; i < textures.size(); i++){
        if (meshObj -> texture.textureOverloadId == textures.at(i).texture.textureId){
          overloadId = (i + 1) % textures.size();
        }
      }
      meshObj -> texture.textureOverloadName = textures.at(overloadId).textureName;
      meshObj -> texture.textureOverloadId = textures.at(overloadId).texture.textureId;
    }
}

int textureId = 0;
void onScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
  scroll_callback(window, state, xoffset, yoffset);

  if (state.offsetTextureMode && state.selectedIndex != -1){
    float offsetAmount = yoffset * 0.001;
    maybeApplyTextureOffset(state.selectedIndex, glm::vec2(state.manipulatorAxis == YAXIS ? offsetAmount : 0, state.manipulatorAxis == YAXIS ? 0 : offsetAmount));
  }
  if (!state.offsetTextureMode && state.selectedIndex != -1 && idExists(world, state.selectedIndex)){
    maybeChangeTexture(state.selectedIndex);
  }

  if (voxelPtr == NULL){
    return;
  }
  
  if (yoffset > 0){
    textureId += 1;
    applyTextureToCube(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels, textureId);
  }
  if (yoffset < 0){
    textureId -= 1;
    applyTextureToCube(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels, textureId);
  }
}

void keyCharCallback(GLFWwindow* window, unsigned int codepoint){
  schemeBindings.onKeyCharCallback(codepoint); 
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
  schemeBindings.onKeyCallback(getKeyRemapping(keyMapper, key), scancode, action, mods);
  if (key == 261 && voxelPtr != NULL){  // delete
    removeVoxel(voxelPtr -> voxel, voxelPtr -> voxel.selectedVoxels);
    voxelPtr -> voxel.selectedVoxels.clear();
  } 

  if (key == GLFW_KEY_LEFT && action == 1 && state.selectedIndex != -1){
    if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
      setGameObjectPosition(state.selectedIndex, snapTranslateUp(getGameObjectPosition(state.selectedIndex, false), state.manipulatorAxis));
    }else if (state.manipulatorMode == ROTATE){
      setGameObjectRotation(state.selectedIndex, snapAngleDown(getGameObjectRotation(state.selectedIndex, false), state.manipulatorAxis));
    }else if (state.manipulatorMode == SCALE){
      setGameObjectScale(state.selectedIndex, snapScaleDown(getGameObjectScale(state.selectedIndex), state.manipulatorAxis));
    }
  }
  if (key == GLFW_KEY_RIGHT && action == 1 && state.selectedIndex != -1){
    if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
      setGameObjectPosition(state.selectedIndex, snapTranslateDown(getGameObjectPosition(state.selectedIndex, false), state.manipulatorAxis));
    }else if (state.manipulatorMode == ROTATE){
      setGameObjectRotation(state.selectedIndex, snapAngleUp(getGameObjectRotation(state.selectedIndex, false), state.manipulatorAxis));
    }else if (state.manipulatorMode == SCALE){
      setGameObjectScale(state.selectedIndex, snapScaleUp(getGameObjectScale(state.selectedIndex), state.manipulatorAxis));
    }
  }
  if (key == GLFW_KEY_UP && action == 1 && state.selectedIndex != -1){
    if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
      setSnapTranslateUp();
    }else if (state.manipulatorMode == ROTATE){
      setSnapAngleUp();
    }else if (state.manipulatorMode == SCALE){
      setSnapScaleUp();
    }
  }
  if (key == GLFW_KEY_DOWN && action == 1 && state.selectedIndex != -1){
    if (state.manipulatorMode == NONE || state.manipulatorMode == TRANSLATE){
      setSnapTranslateDown();
    }else if (state.manipulatorMode == ROTATE){
      setSnapAngleDown();
    }else if (state.manipulatorMode == SCALE){
      setSnapScaleDown();
    }
  }

  if (key == GLFW_KEY_K && action == 1){
    state.portalTextureIndex--;
    if (state.portalTextureIndex < 0){
      state.portalTextureIndex = 0;
    }
    std::cout << "portal index: " << state.portalTextureIndex << std::endl;
  }
  if (key == GLFW_KEY_L && action == 1){
    state.portalTextureIndex++;
    std::cout << "portal index: " << state.portalTextureIndex << std::endl;
  }

  if (key == GLFW_KEY_Q && action == 1){
    std::cout << "apply masking up: " << std::endl;
    HeightMapData& hm = *(getHeightmaps(world.objectMapping).begin() -> second);
    applyMasking(hm, hm.width / 2, hm.height / 2, mask, 1.f, []() -> void { 
      std::cout << "recalc physics placeholder" << std::endl;
    });
  }
  if (key == GLFW_KEY_E && action == 1){
    std::cout << "apply masking down: " << std::endl;
    HeightMapData& hm = *(getHeightmaps(world.objectMapping).begin() -> second);
    applyMasking(hm, hm.width / 2, hm.height / 2, mask, -1.f, []() -> void {
      std::cout << "recalc physics placeholder" << std::endl;
    });
  }
}

void expandVoxelUp(){
  if (voxelPtr == NULL){
    return;
  }
  expandVoxels(voxelPtr -> voxel, 0, useYAxis ? -1 : 0, !useYAxis ? -1 : 0);
}
void expandVoxelDown(){
  if (voxelPtr == NULL){
    return;
  }
  expandVoxels(voxelPtr -> voxel , 0, useYAxis ? 1 : 0, !useYAxis ? 1 : 0);
}
void expandVoxelLeft(){
  if (voxelPtr == NULL){
    return;
  }
  expandVoxels(voxelPtr -> voxel, -1, 0, 0);
}
void expandVoxelRight(){
  if (voxelPtr == NULL){
    return;
  }
  expandVoxels(voxelPtr -> voxel, 1, 0, 0);
}


void onMouseButton(){    
  //for (auto [id, scene] : world.scenes){
  //  std::cout << scenegraphAsDotFormat(scene, world.objectMapping) << std::endl;
  //}
}