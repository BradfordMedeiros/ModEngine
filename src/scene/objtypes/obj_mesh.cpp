#include "./obj_mesh.h"

GameObjectMesh createMesh(GameobjAttributes& attr, ObjectTypeUtil& util){
  // get rid of meshes attribute completely, make ensuremeshloaded return the meshes you're actually responsible for
  // basically top level ensureMesh(attr("mesh") => your nodes, then the child ones can be logic'd in via being smart about ensureMeshLoaded :) 
  std::string rootMeshName = attr.stringAttributes.find("mesh") == attr.stringAttributes.end()  ? "" : attr.stringAttributes.at("mesh");
  auto meshNamesForObj = util.ensureMeshLoaded(rootMeshName);
  std::vector<std::string> meshNames;
  std::vector<Mesh> meshesToRender;
  for (auto meshName : meshNamesForObj){
    std::cout << "trying to get mesh name: " << meshName << std::endl;
    meshNames.push_back(meshName);
    meshesToRender.push_back(util.createMeshCopy(meshName));  
  }

  GameObjectMesh obj {
    .meshNames = meshNames,
    .meshesToRender = meshesToRender,
    .isDisabled = attr.stringAttributes.find("disabled") != attr.stringAttributes.end(),
    .nodeOnly = meshNames.size() == 0,
    .rootMesh = rootMeshName,
    .texture = texinfoFromFields(attr, util.ensureTextureLoaded),
    .discardAmount = attr.numAttributes.find("discard") == attr.numAttributes.end() ? 0.f : attr.numAttributes.at("discard"),
    .emissionAmount = attr.numAttributes.find("emission") == attr.numAttributes.end() ? 0.f : attr.numAttributes.at("emission"),
    .tint = attr.vecAttr.vec4.find("tint") == attr.vecAttr.vec4.end() ? glm::vec4(1.f, 1.f, 1.f, 1.f) : attr.vecAttr.vec4.at("tint"),
  };
  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeMesh(GameObjectMesh obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  if (obj.rootMesh != ""){
    pairs.push_back(std::pair<std::string, std::string>("mesh", obj.rootMesh));
  }
  if (obj.isDisabled){
    pairs.push_back(std::pair<std::string, std::string>("disabled", "true"));
  }
  addSerializedTextureInformation(pairs, obj.texture);
  if (!isIdentityVec(obj.tint)){
    pairs.push_back(std::pair<std::string, std::string>("tint", serializeVec(obj.tint)));
  }

  return pairs;  
}

void meshObjAttr(GameObjectMesh& meshObj, GameobjAttributes& _attributes){
  if (meshObj.meshNames.size() > 0){
    _attributes.stringAttributes["mesh"] = meshObj.meshNames.at(0);
  }
  _attributes.stringAttributes["isDisabled"] = meshObj.isDisabled ? "true" : "false";
  _attributes.vecAttr.vec4["tint"] = meshObj.tint;
}

void setMeshAttributes(GameObjectMesh& meshObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  if (attributes.stringAttributes.find("isDisabled") != attributes.stringAttributes.end()){
    meshObj.isDisabled = attributes.stringAttributes.at("isDisabled") == "true";;
  }
  setTextureAttributes(meshObj.texture, attributes, util);
  if (attributes.vecAttr.vec4.find("tint") != attributes.vecAttr.vec4.end()){
    meshObj.tint = attributes.vecAttr.vec4.at("tint");
  }
}