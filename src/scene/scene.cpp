#include "./scene.h"

FullScene deserializeFullScene(std::string content){
  std::map<std::string, Mesh> meshes;
  auto objectMapping = getObjectMapping();

  auto addObjectAndLoadMesh = [&meshes, &objectMapping](short id, std::string type, std::string field, std::string payload) -> void {
    addObject(id, type, field, payload, objectMapping, meshes, "./res/models/box/box.obj", [&meshes](std::string meshName) -> void {
      meshes[meshName] = loadMesh(meshName, "./res/textures/default.jpg");
    });
  
    // hacking in rigid body creation here, source of truth should come from addObject
    if (type == "default"){
      std::cout << "---------------------------------------" << std::endl;
      std::cout << "placeholder add physics rigid body here" << std::endl;
    }
  };

  std::cout << "deserialize scene placeholder" << std::endl;
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