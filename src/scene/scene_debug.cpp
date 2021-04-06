#include "./scene_debug.h"

std::string getDotInfoForNode(std::string nodeName, int nodeId, objid sceneId, objid groupId, glm::vec3 position, std::vector<std::string> meshes){
  return std::string("\"") + nodeName + "(" + std::to_string(nodeId) + ", " + std::to_string(sceneId) + ")" + " pos: " + print(position) + " meshes: [" + join(meshes, ' ') + "] groupId: " + std::to_string(groupId) + "\"";
}

std::string scenegraphAsDotFormat(SceneSandbox& sandbox, objid sceneId, std::map<objid, GameObjectObj>& objectMapping){
  std::string graph = "";
  std::string prefix = "strict graph {\n";
  std::string suffix = "}"; 

  std::string relations = "";
  forEveryGameobj(sandbox, [sceneId, &sandbox, &relations, &objectMapping](objid id, GameObject& gameobj) -> void {
    auto obj = getGameObjectH(sandbox, id);
    if (obj.sceneId != sceneId){
      return;
    }

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
