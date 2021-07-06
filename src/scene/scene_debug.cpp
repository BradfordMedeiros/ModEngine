#include "./scene_debug.h"

std::string getDotInfoForNode(std::string nodeName, int id, objid sceneId, objid groupId, glm::vec3 position, glm::vec3 scale, glm::quat rotation, std::vector<std::string> meshes){
  return std::string("\"") + nodeName + "(id: " + std::to_string(id) + ", sceneId:" + std::to_string(sceneId) + ", " + "groupId: " + std::to_string(groupId) + 
  ") pos: " + print(position) + " scale: " + print(scale) +  " rot: " + print(rotation) + 
  " meshes: [" + join(meshes, ' ') + "]\"";
}

std::string scenegraphAsDotFormat(SceneSandbox& sandbox, std::map<objid, GameObjectObj>& objectMapping){
  std::string graph = "";
  std::string prefix = "strict graph {\n";
  std::string suffix = "}"; 

  std::string relations = "";

  forEveryGameobj(sandbox, [&sandbox, &relations, &objectMapping](objid id, GameObject& childObj) -> void {
    if (id == 0){
      return;   // will show up since things are parented to the root
    }
    auto childObjH = getGameObjectH(sandbox, id);
    auto parentId = childObjH.parentId;
    auto parentObj = getGameObject(sandbox, parentId);
    auto parentObjH = getGameObjectH(sandbox, parentId);

    relations = relations + 
     getDotInfoForNode(parentObj.name, parentObj.id, parentObjH.sceneId, parentObjH.groupId, parentObj.transformation.position, parentObj.transformation.scale, parentObj.transformation.rotation, getMeshNames(objectMapping, parentId)) 
    + " -- " + 
    getDotInfoForNode(childObj.name, childObjH.id,  childObjH.sceneId, childObjH.groupId, childObj.transformation.position, childObj.transformation.scale, childObj.transformation.rotation, getMeshNames(objectMapping, id)) + "\n";
  }); 

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

std::string debugTransformCache(SceneSandbox& sandbox){
  std::string content = "";
  for (auto &[id, transformElement] : sandbox.mainScene.absoluteTransforms){
    auto transform = transformElement.transform;
    content += std::to_string(id) + " " + "(" + print(transform.position) + ") (" + print(transform.scale) + ")\n";
  }
  content = content + "\n";
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

void dumpPhysicsInfo(std::map<objid, btRigidBody*>& rigidbodys){
  for (auto [i, rigidBody]: rigidbodys){
    std::cout << "PHYSICS:" << std::to_string(i) << ":" <<  print(getPosition(rigidBody));
  }
}
