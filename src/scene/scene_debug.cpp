#include "./scene_debug.h"

std::string getDotInfoForNode(std::string nodeName, int nodeId, objid groupId, std::vector<std::string> meshes){
  return std::string("\"") + nodeName + "(" + std::to_string(nodeId) + ")" + " meshes: [" + join(meshes, ' ') + "] groupId: " + std::to_string(groupId) + "\"";
}

std::string scenegraphAsDotFormat(Scene& scene, std::map<objid, GameObjectObj>& objectMapping){
  std::string graph = "";
  std::string prefix = "strict graph {\n";
  std::string suffix = "}"; 

  std::string relations = "";
  for (auto [id, obj] : scene.idToGameObjectsH){
    auto childId = id;
    auto parentId = obj.parentId;
    auto groupId = obj.groupId;
    auto parentGroupId = parentId != - 1 ? getGroupId(scene, parentId) : -1;

    auto childName = getGameObject(scene, childId).name;
    auto parentName = parentId == -1 ? "root" : getGameObject(scene, parentId).name;
        
    relations = relations + getDotInfoForNode(parentName, parentId, parentGroupId, getMeshNames(objectMapping, parentId)) + " -- " + getDotInfoForNode(childName, childId, groupId, getMeshNames(objectMapping, childId)) + "\n";
  }
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
