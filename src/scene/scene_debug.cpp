#include "./scene_debug.h"

std::string getDotInfoForNode(std::string nodeName, int nodeId, objid sceneId, objid groupId, glm::vec3 position, std::vector<std::string> meshes){
  return std::string("\"") + nodeName + "(" + std::to_string(nodeId) + ", " + std::to_string(sceneId) + ")" + " pos: " + print(position) + " meshes: [" + join(meshes, ' ') + "] groupId: " + std::to_string(groupId) + "\"";
}

std::string scenegraphAsDotFormat(SceneSandbox& sandbox, std::map<objid, GameObjectObj>& objectMapping){
  std::string graph = "";
  std::string prefix = "strict graph {\n";
  std::string suffix = "}"; 

  std::string relations = "";
  forEveryGameobj(sandbox, [&sandbox, &relations, &objectMapping](objid id, GameObject& gameobj) -> void {
    auto obj = getGameObjectH(sandbox, id);
    auto childId = id;
    auto parentId = obj.parentId;
    auto groupId = obj.groupId;
    auto parentGroupId = parentId != - 1 ? getGroupId(sandbox, parentId) : -1;

    auto childName = gameobj.name;
    auto parentName = parentId == -1 ? "root" : getGameObject(sandbox, parentId).name;
        
    auto positionParent = parentId == -1 ? glm::vec3(0.f, 0.f, 0.f) : getGameObject(sandbox, parentId).transformation.position;
    auto positionChild = childId == -1 ? glm::vec3(0.f, 0.f, 0.f) : gameobj.transformation.position;
    relations = relations + getDotInfoForNode(parentName, parentId, obj.sceneId, parentGroupId, positionParent, getMeshNames(objectMapping, parentId)) + " -- " + getDotInfoForNode(childName, childId,  getGameObjectH(sandbox, childId).sceneId, groupId, positionChild, getMeshNames(objectMapping, childId)) + "\n";
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

  //   std::set<objid> children;

  return content;
}

void printPhysicsInfo(PhysicsInfo physicsInfo){
  BoundInfo info = physicsInfo.boundInfo;
  std::cout << "x: [ " << info.xMin << ", " << info.xMax << "]" << std::endl;
  std::cout << "y: [ " << info.yMin << ", " << info.yMax << "]" << std::endl;
  std::cout << "z: [ " << info.zMin << ", " << info.zMax << "]" << std::endl;
  std::cout << "pos: (" << physicsInfo.transformation.position.x << ", " << physicsInfo.transformation.position.y << ", " << physicsInfo.transformation.position.z << ")" << std::endl;
  std::cout << "box: (" << physicsInfo.collisionInfo.x << ", " << physicsInfo.collisionInfo.y << ", " << physicsInfo.collisionInfo.z << ")" << std::endl;
}

void dumpPhysicsInfo(std::map<objid, btRigidBody*>& rigidbodys){
  for (auto [i, rigidBody]: rigidbodys){
    std::cout << "PHYSICS:" << std::to_string(i) << ":" <<  print(getPosition(rigidBody));
  }
}
