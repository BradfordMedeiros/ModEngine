#include "./scene.h"

FullScene deserializeFullScene(std::string content){
  std::map<std::string, Mesh> meshes;
  auto objectMapping = getObjectMapping();

  auto addObjectAndLoadMesh = [&meshes, &objectMapping](short id, std::string type, std::string field, std::string payload) -> void {
    addObject(id, type, field, payload, objectMapping, meshes, "./res/models/box/box.obj", [&meshes](std::string meshName) -> void {  // @todo this is duplicate with commented below
      meshes[meshName] = loadMesh(meshName, "./res/textures/default.jpg");
    });
  };

  Scene scene = deserializeScene(content, addObjectAndLoadMesh, fields);
  FullScene fullscene = {
    .scene = scene,
    .meshes = meshes,
    .objectMapping = objectMapping,
  };
  return fullscene;
}

std::string serializeFullScene(Scene& scene, std::map<short, GameObjectObj> objectMapping){
  return serializeScene(scene, [&objectMapping](short objectId)-> std::vector<std::pair<std::string, std::string>> {
    return getAdditionalFields(objectId, objectMapping);
  });
}

void addObjectToFullScene(FullScene& fullscene, std::string name, std::string meshName, glm::vec3 pos){
  addObjectToScene(fullscene.scene, name, meshName, pos, [&fullscene](short id, std::string type, std::string field, std::string payload) -> void {
    addObject(id, type, field, payload, fullscene.objectMapping, fullscene.meshes, "./res/models/box/box.obj", [&fullscene](std::string meshName) -> void { // @todo dup with commented above
      fullscene.meshes[meshName] = loadMesh(meshName, "./res/textures/default.jpg");
    });
  });
}
