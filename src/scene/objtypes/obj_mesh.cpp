#include "./obj_mesh.h"

std::vector<AutoSerialize> meshAutoserializer {
  AutoSerializeVec4 {
    .structOffset = offsetof(GameObjectMesh, tint),
    .structOffsetFiller = std::nullopt,
    .field = "tint",
    .defaultValue = glm::vec4(1.f, 1.f, 1.f, 1.f),
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectMesh, emissionAmount),
    .structOffsetFiller = std::nullopt,
    .field = "emission",
    .defaultValue = 0.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectMesh, discardAmount),
    .structOffsetFiller = std::nullopt,
    .field = "discard",
    .defaultValue = 0.f,
  },
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectMesh, isDisabled),
    .field = "disabled",
    .onString = "true",
    .offString = "false",
    .defaultValue = false,
  },
  AutoserializeReservedField {
    .field = "mesh",
    .fieldType = ATTRIBUTE_STRING,
  },
  AutoSerializeTextureLoaderManual {
    .structOffset = offsetof(GameObjectMesh, normalTexture),
    .field = "normal-texture",
    .defaultValue = "",
  }
};

static auto _ = addTextureAutoserializer<GameObjectMesh>(meshAutoserializer);


GameObjectMesh createMesh(GameobjAttributes& attr, ObjectTypeUtil& util){
  // get rid of meshes attribute completely, make ensuremeshloaded return the meshes you're actually responsible for
  // basically top level ensureMesh(attr("mesh") => your nodes, then the child ones can be logic'd in via being smart about ensureMeshLoaded :) 
  std::string rootMeshName = attr.stringAttributes.find("mesh") == attr.stringAttributes.end()  ? "" : attr.stringAttributes.at("mesh");
  bool isRoot = false;
  auto meshNamesForObj = util.ensureMeshLoaded(rootMeshName, &isRoot);
  std::vector<std::string> meshNames;
  std::vector<Mesh> meshesToRender;
  for (auto meshName : meshNamesForObj){
    meshNames.push_back(meshName);
    meshesToRender.push_back(util.createMeshCopy(meshName));  
  }

  GameObjectMesh obj {
    .meshNames = meshNames,
    .meshesToRender = meshesToRender,
    .nodeOnly = meshNames.size() == 0,
    .rootMesh = rootMeshName,
    .isRoot = isRoot,
  };
  std::cout << "root mesh name: " << rootMeshName << ", node only: " << obj.nodeOnly << std::endl;

  createAutoSerializeWithTextureLoading((char*)&obj, meshAutoserializer, attr, util);
  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeMesh(GameObjectMesh obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  if (obj.isRoot && obj.rootMesh != ""){
    pairs.push_back(std::pair<std::string, std::string>("mesh", obj.rootMesh));
  }
  autoserializerSerialize((char*)&obj, meshAutoserializer, pairs);
  return pairs;  
}

void meshObjAttr(GameObjectMesh& meshObj, GameobjAttributes& _attributes){
  _attributes.stringAttributes["mesh"] = meshObj.isRoot ? meshObj.rootMesh : "";
  autoserializerGetAttr((char*)&meshObj, meshAutoserializer, _attributes);
}

std::optional<AttributeValuePtr> getMeshAttribute(GameObjectMesh& meshObj, const char* field){
  return getAttributePtr((char*)&meshObj, meshAutoserializer, field);
}

bool setMeshAttributes(GameObjectMesh& meshObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  autoserializerSetAttrWithTextureLoading((char*)&meshObj, meshAutoserializer, attributes, util);
  return false;
}

bool setMeshAttribute(GameObjectMesh& meshObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util){
  return autoserializerSetAttrWithTextureLoading((char*)&meshObj, meshAutoserializer, field, value, util);
}