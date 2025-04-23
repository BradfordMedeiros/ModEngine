#include "./scene_debug.h"

std::string getDotInfoForNode(DotInfo& info){
  return std::string("\"") + info.name + "(id: " + std::to_string(info.id) + ", sceneId:" + std::to_string(info.sceneId) + ", " + "groupId: " + std::to_string(info.groupId) + ", prefabId: " + print(info.prefabId) + 
  ") pos: " + print(info.position) + " scale: " + print(info.scale) +  " rot: " + print(info.rotation) + " bone = " + std::string(info.isBone ? "true" : "false") +
  " meshes: [" + join(info.meshes, ' ') + "] disabled: " + print(info.isDisabled) +  "\"";
}

std::optional<bool> isMeshDisabled(ObjectMapping& objectMapping, objid id){
  GameObjectObj& gameObjV = objectMapping.objects.at(id); 
  auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 
  if (!meshObj){
    return std::nullopt;
  }
  return meshObj -> isDisabled;
}

std::vector<DotInfos> getDotRelations(SceneSandbox& sandbox, ObjectMapping& objectMapping){
  std::vector<DotInfos> dotRelations;
  forEveryGameobj(sandbox, [&sandbox, &objectMapping, &dotRelations](objid id, GameObject& childObj) -> void {
    auto childObjH = getGameObjectH(sandbox, id);

    bool objectMappingExists = objectMapping.objects.find(id) != objectMapping.objects.end();
    DotInfo childInfo {
      .name = childObj.name,
      .id = childObjH.id,
      .sceneId = childObjH.sceneId,
      .groupId = childObjH.groupId,
      .prefabId = childObjH.prefabId,
      .position = childObj.transformation.position,
      .scale = childObj.transformation.scale,
      .rotation = childObj.transformation.rotation, 
      .isBone = childObj.isBone,
      .meshes = objectMappingExists ? getMeshNames(objectMapping, id) : std::vector<std::string>{},
      .isDisabled = isMeshDisabled(objectMapping, id),
    };
    if (id == 0){
      dotRelations.push_back(DotInfos{
        .child = childInfo,
        .parent = std::nullopt,
      });
      return;   
    }
    auto parentId = childObjH.parentId;  // this isn't really supposed to happen but indicates a bug
 
    auto parentObj = getGameObject(sandbox, parentId);
    auto parentObjH = getGameObjectH(sandbox, parentId);

    bool parentObjectMappingExists = objectMapping.objects.find(parentId) != objectMapping.objects.end();
    DotInfo parentInfo {
      .name = parentObj.name,
      .id = parentObj.id,
      .sceneId = parentObjH.sceneId,
      .groupId = parentObjH.groupId,
      .prefabId = parentObjH.prefabId,
      .position = parentObj.transformation.position,
      .scale = parentObj.transformation.scale,
      .rotation = parentObj.transformation.rotation,
      .isBone = parentObj.isBone,
      .meshes = parentObjectMappingExists ? getMeshNames(objectMapping, parentId) : std::vector<std::string>{},
      .isDisabled = isMeshDisabled(objectMapping, parentId),
    };
    dotRelations.push_back(DotInfos{
      .child = childInfo,
      .parent = parentInfo,
    });
  }); 
  return dotRelations;
}

std::string scenegraphAsDotFormat(SceneSandbox& sandbox, ObjectMapping& objectMapping){
  std::vector<DotInfos> dotRelations = getDotRelations(sandbox, objectMapping);
  std::string graph = "";
  std::string prefix = "strict graph {\n";
  std::string suffix = "}"; 
  std::string relations = "";
  for (auto &dotInfo : dotRelations){
    if (dotInfo.parent.has_value()){
      relations = relations + getDotInfoForNode(dotInfo.parent.value()) + " -- " + getDotInfoForNode(dotInfo.child) + "\n";
    }else{
      relations =  relations + getDotInfoForNode(dotInfo.child) + "\n";
    }
  }
  graph = graph + prefix + relations + suffix;
  return graph;
}

std::string debugAllGameObjects(SceneSandbox& sandbox){
  std::string content = "";
  for (auto &[id, gameobj] : sandbox.mainScene.idToGameObjects){
    content += std::to_string(id) + " " + std::to_string(gameobj.id) + " " + gameobj.name + "\n";
  }
  return content;
}

std::string debugAllGameObjectsH(SceneSandbox& sandbox){
  std::string content = "";
  for (auto &[id, gameobj] : sandbox.mainScene.idToGameObjectsH){
    content += std::to_string(id) + 
      " " + std::to_string(gameobj.id) + 
      " " + std::to_string(gameobj.sceneId) + 
      " " + std::to_string(gameobj.groupId) + 
      " " + std::to_string(gameobj.parentId) + " | ";
    for (auto id : gameobj.children){
      content = content + " " + std::to_string(id);      
    }
    content = content + "\n";
  }
  return content;
}

std::string debugAllGameObjectObj(std::map<objid, GameObjectObj>& objectMapping){
  std::string content = "";
  for (auto &[id, _] : objectMapping){
    content += std::to_string(id);
    content = content + "\n";
  }
  return content; 
}

std::string debugTransformCache(SceneSandbox& sandbox){
  std::string content = "";
  for (auto &[id, transformElement] : sandbox.mainScene.absoluteTransforms){
    auto transform = transformElement.transform;
    content += std::to_string(id) + " " + "(" + print(transform.position) + ") (" + print(transform.scale) + ")\n";
  }
  content = content + "\n";
  return content;  
}

std::string debugLoadedTextures(std::map<std::string, TextureRef>& textures){
  std::string content = "";
  for (auto &[name, texture] : textures){
    content += name + " " + "[";
    for (auto id : texture.owners){
      content = content + std::to_string(id) + " ";
    }
    auto textureSize = getTextureSizeInfo(texture.texture);
    content = content + "] width = " + std::to_string(textureSize.width) + ", height = " + std::to_string(textureSize.height) + "\n";
  }
  content = content + "\n";
  return content;
}

std::string debugLoadedMeshes(std::map<std::string, MeshRef>& meshes){
  std::string content = "";
  for (auto &[name, mesh] : meshes){
    content += name + " " + "[";
    for (auto id : mesh.owners){
      content = content + std::to_string(id) + " ";
    }
    content = content + "]\n";
  }
  content = content + "\n";
  return content; 
}

std::string debugAnimations(std::map<objid, std::vector<Animation>>& animations){
  std::string content = "";
  for (auto &[id, animVals] : animations){
    content += std::to_string(id) + " " + "[";
    for (auto animation : animVals){
      content = content + animation.name + " ";
    }
    content = content + "]\n";
  }
  content = content + "\n";
  return content; 
}

std::string debugPhysicsInfo(std::map<objid, PhysicsValue>& rigidbodys){
  std::string content = "";
  for (auto [id, physicsBody]: rigidbodys){
    content += std::to_string(id) + "\n";
  }
  return content;
}

std::string debugSceneInfo(SceneSandbox& sandbox){
  std::string content = "";
  for (auto [id, sceneMetadata]: sandbox.sceneIdToSceneMetadata){
    content += std::to_string(id) + " " + print(sceneMetadata.name) + " " + sceneMetadata.scenefile + "\n";
  }
  return content;
}
void printPhysicsInfo(PhysicsInfo physicsInfo){
  BoundInfo info = physicsInfo.boundInfo;
  std::cout << "x: [ " << info.xMin << ", " << info.xMax << "]" << std::endl;
  std::cout << "y: [ " << info.yMin << ", " << info.yMax << "]" << std::endl;
  std::cout << "z: [ " << info.zMin << ", " << info.zMax << "]" << std::endl;
  std::cout << "pos: (" << physicsInfo.transformation.position.x << ", " << physicsInfo.transformation.position.y << ", " << physicsInfo.transformation.position.z << ")" << std::endl;
  std::cout << "box: (" << physicsInfo.transformation.scale.x << ", " << physicsInfo.transformation.scale.y << ", " << physicsInfo.transformation.scale.z << ")" << std::endl;
}

