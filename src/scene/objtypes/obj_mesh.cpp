#include "./obj_mesh.h"

std::vector<AutoSerialize> meshAutoserializer {
  AutoSerializeVec4 {
    .structOffset = offsetof(GameObjectMesh, tint),
    .structOffsetFiller = std::nullopt,
    .field = "tint",
    .defaultValue = glm::vec4(1.f, 1.f, 1.f, 1.f),
  },
  AutoSerializeVec3 {
    .structOffset = offsetof(GameObjectMesh, emissionAmount),
    .structOffsetFiller = std::nullopt,
    .field = "emission",
    .defaultValue = glm::vec3(0.f, 0.f, 0.f),
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
  std::optional<std::string> meshStr = getStrAttr(attr, "mesh");
  if (meshStr.has_value()){
    util.ensureMeshLoaded(meshStr.value());  // this will modify attr and add meshes if this object has any 
  }

  auto meshArrStr = getArrStrAttr(attr, "meshes");
  auto meshesToLoad = meshArrStr.has_value() ? meshArrStr.value() : std::vector<std::string>{};

  modlog("mesh", std::string("meshes are: ") + print(meshesToLoad));

  std::vector<Mesh> meshesToRender;
  for (auto &mesh : meshesToLoad){
    meshesToRender.push_back(util.createMeshCopy(mesh));  
  }    

  GameObjectMesh obj {
    .rootMesh = meshStr,
    .meshNames = meshesToLoad,
    .meshesToRender = meshesToRender,
  };
  obj.rootidCache = 0;

  std::vector<std::vector<objid>> boneGameObjIdCache;
  for (auto &mesh : obj.meshesToRender){
    std::vector<objid> emptyCache;
    for (int i = 0; i < mesh.bones.size(); i++){
      emptyCache.push_back(0);
    }
    boneGameObjIdCache.push_back(emptyCache);
  }
  obj.boneGameObjIdCache = boneGameObjIdCache;

  obj.hasBones = false;
  for (int i = 0; i < obj.meshesToRender.size(); i++){
    Mesh& mesh = obj.meshesToRender.at(i);
    if (mesh.bones.size() > 0){
      obj.hasBones = true;
      break;
    }
  }

  //std::cout << "root mesh name: " << rootMeshName << ", node only: " << obj.nodeOnly << std::endl;
  createAutoSerializeWithTextureLoading((char*)&obj, meshAutoserializer, attr, util);
  return obj;
}

std::vector<std::pair<std::string, std::string>> serializeMesh(GameObjectMesh obj, ObjectSerializeUtil& util){
  // this doesn't really handle serialization for attributes properly 
  std::vector<std::pair<std::string, std::string>> pairs;

  if (obj.rootMesh.has_value()){
    pairs.push_back(std::pair<std::string, std::string>("mesh", obj.rootMesh.value()));
  }
  autoserializerSerialize((char*)&obj, meshAutoserializer, pairs);
  return pairs;  
}

//void meshObjAttr(GameObjectMesh& meshObj, GameobjAttributes& _attributes){
//  _attributes.stringAttributes["mesh"] = meshObj.isRoot ? meshObj.rootMesh : "";
//  autoserializerGetAttr((char*)&meshObj, meshAutoserializer, _attributes);
//}

std::optional<AttributeValuePtr> getMeshAttribute(GameObjectMesh& meshObj, const char* field){
  return getAttributePtr((char*)&meshObj, meshAutoserializer, field);
}

bool setMeshAttribute(GameObjectMesh& meshObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&){
  return autoserializerSetAttrWithTextureLoading((char*)&meshObj, meshAutoserializer, field, value, util);
}