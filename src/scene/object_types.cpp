#include "./object_types.h"

void shaderLogDebug(const char* str);

std::map<objid, GameObjectObj> getObjectMapping() {
	std::map<objid, GameObjectObj> objectMapping;
	return objectMapping;
}

std::size_t getVariantIndex(GameObjectObj gameobj){
  return gameobj.index();
}

std::optional<AttributeValuePtr> nothingObjectAttribute(GameObjectObj& obj, const char* field){
  //modassert(false, "object attribute not yet implemented for this type");
  return std::nullopt; 
}

bool nothingSetAttribute(GameObjectObj& obj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&){
  modassert(false, std::string("set attr not implemented for this type, field: ") + std::string(field));
  return false;
}

template<typename T>
std::function<std::optional<AttributeValuePtr>(GameObjectObj& obj, const char* field)> convertObjectAttribute(std::function<std::optional<AttributeValuePtr>(T&, const char* field)> getAttr) {   
  return [getAttr](GameObjectObj& obj, const char* field) -> std::optional<AttributeValuePtr> {
    auto objInstance = std::get_if<T>(&obj);
    assert(objInstance != NULL);
    return getAttr(*objInstance, field);
  };
}

template<typename T>
std::function<bool(GameObjectObj& obj, const char* field, AttributeValue value, ObjectSetAttribUtil&, SetAttrFlags&)> convertElementSetAttrValue(std::function<bool(T&, const char* field, AttributeValue value, ObjectSetAttribUtil&, SetAttrFlags&)> setAttr) {   
  return [setAttr](GameObjectObj& obj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags& flags) -> bool {
    auto objInstance = std::get_if<T>(&obj);
    assert(objInstance != NULL);
    return setAttr(*objInstance, field, value, util, flags);
  };
}


//  std::function<std::vector<std::pair<std::string, std::string>>(GameObjectObj&)> serialize;
template<typename T>
std::function<std::vector<std::pair<std::string, std::string>>(GameObjectObj& obj, ObjectSerializeUtil& util)> convertSerialize(std::function<std::vector<std::pair<std::string, std::string>>(T&, ObjectSerializeUtil&)> serialize) {   
  return [serialize](GameObjectObj& obj, ObjectSerializeUtil& util) -> std::vector<std::pair<std::string, std::string>> {
    auto objInstance = std::get_if<T>(&obj);
    assert(objInstance != NULL);
    return serialize(*objInstance, util);
  };
}

template<typename T>
std::function<void(GameObjectObj& obj, ObjectRemoveUtil&)> convertRemove(std::function<void(T&, ObjectRemoveUtil&)> rmObject) {   
  return [rmObject](GameObjectObj& obj, ObjectRemoveUtil& util) -> void {
    auto objInstance = std::get_if<T>(&obj);
    assert(objInstance != NULL);
    rmObject(*objInstance, util);
  };
}

std::vector<std::pair<std::string, std::string>> serializeNotImplemented(GameObjectObj& obj, ObjectSerializeUtil& util){
  std::cout << "ERROR: SERIALIZATION NOT YET IMPLEMENTED" << std::endl;
  //assert(false);
  return { {"not", "implemented"}};    
}

void removeDoNothing(GameObjectObj& obj, ObjectRemoveUtil& util){}

std::vector<ObjectType> objTypes = {
  ObjectType {
    .name = "camera",
    .variantType = getVariantIndex(GameObjectCamera{}),
    .createObj = createCamera,
    .objectAttribute = convertObjectAttribute<GameObjectCamera>(getCameraAttribute),
    .setAttribute = convertElementSetAttrValue<GameObjectCamera>(setCameraAttribute),
    .serialize = convertSerialize<GameObjectCamera>(serializeCamera),
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "portal",
    .variantType = getVariantIndex(GameObjectPortal{}),
    .createObj = createPortal,
    .objectAttribute = convertObjectAttribute<GameObjectPortal>(getPortalAttribute),
    .setAttribute = convertElementSetAttrValue<GameObjectPortal>(setPortalAttribute),
    .serialize = convertSerialize<GameObjectPortal>(serializePortal),
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "light",
    .variantType = getVariantIndex(GameObjectLight{}),
    .createObj = createLight,
    .objectAttribute = convertObjectAttribute<GameObjectLight>(getLightAttribute),
    .setAttribute = convertElementSetAttrValue<GameObjectLight>(setLightAttribute),
    .serialize = convertSerialize<GameObjectLight>(serializeLight),
    .removeObject = convertRemove<GameObjectLight>(removeLight),
  },
  ObjectType {
    .name = "sound",
    .variantType = getVariantIndex(GameObjectSound{}),
    .createObj = createSound,
    .objectAttribute = convertObjectAttribute<GameObjectSound>(getSoundAttribute),
    .setAttribute = convertElementSetAttrValue<GameObjectSound>(setSoundAttribute),
    .serialize = convertSerialize<GameObjectSound>(serializeSound),
    .removeObject = convertRemove<GameObjectSound>(removeSound),
  },
  ObjectType {
    .name = "text",
    .variantType = getVariantIndex(GameObjectUIText{}),
    .createObj = createUIText,
    .objectAttribute = convertObjectAttribute<GameObjectUIText>(getTextAttribute),
    .setAttribute = convertElementSetAttrValue<GameObjectUIText>(setTextAttribute),
    .serialize = convertSerialize<GameObjectUIText>(serializeText),
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "navmesh",
    .variantType = getVariantIndex(GameObjectNavmesh{}),
    .createObj = createNavmesh,
    .objectAttribute = convertObjectAttribute<GameObjectNavmesh>(getNavmeshAttribute),
    .setAttribute = nothingSetAttribute,
    .serialize = serializeNotImplemented,
    .removeObject = convertRemove<GameObjectNavmesh>(removeNavmesh),
  },
  ObjectType {
    .name = "emitter",
    .variantType = getVariantIndex(GameObjectEmitter{}),
    .createObj = createEmitter,
    .objectAttribute = convertObjectAttribute<GameObjectEmitter>(getEmitterAttribute),
    .setAttribute = convertElementSetAttrValue<GameObjectEmitter>(setEmitterAttribute),
    .serialize = convertSerialize<GameObjectEmitter>(serializeEmitter),
    .removeObject = convertRemove<GameObjectEmitter>(removeEmitterObj),
  },
  ObjectType {
    .name = "default",
    .variantType = getVariantIndex(GameObjectMesh{}),
    .createObj = createMesh,
    .objectAttribute = convertObjectAttribute<GameObjectMesh>(getMeshAttribute),
    .setAttribute = convertElementSetAttrValue<GameObjectMesh>(setMeshAttribute),
    .serialize = convertSerialize<GameObjectMesh>(serializeMesh),
    .removeObject = removeDoNothing,
  },
  ObjectType {
    .name = "octree", 
    .variantType = getVariantIndex(GameObjectOctree{}),
    .createObj = createOctree, 
    .objectAttribute = convertObjectAttribute<GameObjectOctree>(getOctreeAttribute),
    .setAttribute = nothingSetAttribute,
    .serialize = convertSerialize<GameObjectOctree>(serializeOctree),
    .removeObject  = removeDoNothing,
  },
  ObjectType {
    .name = "prefab", 
    .variantType = getVariantIndex(GameObjectPrefab{}),
    .createObj = createPrefabObj, 
    .objectAttribute = convertObjectAttribute<GameObjectPrefab>(getPrefabAttribute),
    .setAttribute = convertElementSetAttrValue<GameObjectPrefab>(setPrefabAttribute),
    .serialize = convertSerialize<GameObjectPrefab>(serializePrefabObj),
    .removeObject  = convertRemove<GameObjectPrefab>(removePrefabObj),
  },
};

GameObjectObj createObjectType(std::string objectType, GameobjAttributes& attr, ObjectTypeUtil util){
  for (auto &objType : objTypes){
    if (objectType == objType.name){
      return objType.createObj(attr, util);
    }
  }
  std::cout << "ERROR: error object type " << objectType << " invalid" << std::endl;
  assert(false);
  return GameObjectObj{};
}

void addObjectType(std::map<objid, GameObjectObj>& mapping, GameObjectObj& gameobj, objid id){
  assert(mapping.find(id) == mapping.end());
  modlog("objecttype - add", std::to_string(id));
  mapping[id] = gameobj;
}

void removeObject(
  std::map<objid, GameObjectObj>& mapping, 
  objid id, 
  std::function<void(std::string)> unbindCamera,
  std::function<void(objid)> unloadScene
){
  if (mapping.find(id) == mapping.end()){
    return;
  }
  auto Object = mapping.at(id); 
  auto variantIndex = Object.index();
  for (auto &objType : objTypes){
    if (variantIndex == objType.variantType){
      std::cout << "type is: " << objType.name << std::endl;
      ObjectRemoveUtil util { 
        .id = id, 
        .unloadScene = unloadScene
      };
      objType.removeObject(Object, util);
      modlog("objecttype - remove", std::to_string(id));
      mapping.erase(id);
      return;
    }
  }
  std::cout << "object type not implemented" << std::endl;
  assert(false);
}


int renderDefaultNode(GLint shaderProgram, Mesh& mesh, glm::mat4& matrix, objid id){
  MeshUniforms meshUniforms {
    .model = matrix,
    .tint = glm::vec4(1.f, 1.f, 0.f, 1.f),
    .bones = &mesh.bones,
    .id = id,
  };
  drawMesh(mesh, shaderProgram, false, meshUniforms);
  return mesh.numTriangles;
}

glm::vec4 getAlternatingColor(int index){
  static std::vector<glm::vec4> colors = { 
    glm::vec4(1.f, 0.f, 0.f, 1.f), 
    glm::vec4(0.f, 1.f, 0.f, 1.f), 
    glm::vec4(0.f, 0.f, 1.f, 1.f), 
  };
  index = (index + 1) % 3;
  return colors.at(index);
}

objid selectedId = 0;
int renderObject(
  GLint shaderProgram,
  bool isSelectionShader,
  objid id, 
  std::map<objid, GameObjectObj>& mapping, 
  int showDebugMask,
  unsigned int portalTexture,
  unsigned int navmeshTexture,
  glm::mat4 model,
  bool drawPoints,
  DefaultMeshes& defaultMeshes,
  bool selectionMode,
  bool drawBones,
  RenderObjApi api,
  glm::mat4& finalModelMatrix
){

  GameObjectObj& toRender = mapping.at(id);
  auto meshObj = std::get_if<GameObjectMesh>(&toRender);

  if (meshObj != NULL && !meshObj -> isDisabled && (meshObj -> meshesToRender.size() > 0)){
    int numTriangles = 0;
    for (int x = 0; x < meshObj -> meshesToRender.size(); x++){
      Mesh& meshToRender = meshObj -> meshesToRender.at(x);
      shaderLogDebug((std::string("draw mesh: ") + meshObj -> meshNames.at(x)).c_str());

      MeshUniforms meshUniforms {
        .model = finalModelMatrix,
        .emissionAmount = meshObj -> emissionAmount,
        .textureSize = meshObj -> texture.texturesize,
        .textureTiling = meshObj -> texture.texturetiling,
        .textureOffset = meshObj -> texture.textureoffset,
        .customTextureId = meshObj -> texture.loadingInfo.textureId,
        .customNormalTextureId = meshObj -> normalTexture.textureId,
        .tint = meshObj -> tint,
        .bones = &meshToRender.bones,
        .id = id,
      };
      drawMesh(meshToRender, shaderProgram, drawPoints, meshUniforms);   
      numTriangles = numTriangles + meshToRender.numTriangles; 
    }
    return numTriangles;
  }


  if (drawBones && api.isBone(id)){
    auto transform = getTransformationFromMatrix(model);
    auto parentId = api.getParentId(id);
    auto boneTransform = api.getTransform(id);
    if (parentId.has_value()){
      auto parentTransform = api.getTransform(parentId.value());
      api.drawLine(boneTransform.position, parentTransform.position, getAlternatingColor(0));
    }
    //api.drawSphere(boneTransform.position);
    auto model = glm::translate(glm::mat4(1.f), transform.position);
    renderDefaultNode(shaderProgram, *defaultMeshes.nodeMesh, model, id);
  }
  if (meshObj != NULL && (meshObj -> meshesToRender.size() > 0) && (showDebugMask & 0b1)) {
    //api.drawLine(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 100.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 1.f));
    return renderDefaultNode(shaderProgram, *defaultMeshes.nodeMesh, finalModelMatrix, id);
  }

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL && (showDebugMask & 0b10)){
    auto transform = getTransformationFromMatrix(model).position;
    auto model = glm::translate(glm::mat4(1.f), transform);
    return renderDefaultNode(shaderProgram, *defaultMeshes.cameraMesh, model, id);
  }

  auto soundObject = std::get_if<GameObjectSound>(&toRender);
  if (soundObject != NULL && (showDebugMask & 0b100)){
    return renderDefaultNode(shaderProgram, *defaultMeshes.soundMesh, finalModelMatrix, id);
  }


  auto portalObj = std::get_if<GameObjectPortal>(&toRender);
  if (portalObj != NULL){
    MeshUniforms meshUniforms {
      .model = finalModelMatrix,
      .customTextureId = portalTexture,
      .bones = &defaultMeshes.nodeMesh -> bones,
      .id = id,
    };
    drawMesh(*defaultMeshes.portalMesh, shaderProgram, false, meshUniforms);
    return defaultMeshes.portalMesh -> numTriangles;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL && (showDebugMask & 0b1000)){   
    return renderDefaultNode(shaderProgram, *defaultMeshes.lightMesh, finalModelMatrix, id);
  }

  auto octreeObj = std::get_if<GameObjectOctree>(&toRender);
  if (octreeObj != NULL){
    Mesh* octreeMesh = getOctreeMesh(*octreeObj);
    modassert(octreeMesh, "no octree mesh available");

    MeshUniforms meshUniforms {
      .model = finalModelMatrix,
      .id = id,
    };
    drawMesh(*octreeMesh, shaderProgram, false, meshUniforms);
    return octreeMesh -> numTriangles;
  }

  auto emitterObj = std::get_if<GameObjectEmitter>(&toRender);
  if (emitterObj != NULL && (showDebugMask & 0b100000)){
    return renderDefaultNode(shaderProgram, *defaultMeshes.emitter, finalModelMatrix, id);
  }

  auto navmeshObj = std::get_if<GameObjectNavmesh>(&toRender);
  if (navmeshObj != NULL){
    MeshUniforms meshUniforms {
      .model = finalModelMatrix,
      .customTextureId = navmeshTexture,
      .id = id,
    };
    drawMesh(navmeshObj -> meshes.at(0), shaderProgram, false, meshUniforms);    


    // this base id point index stuff is pretty hackey bullshit
    // maybe i should pull out a function to render the object only in selection mode. 
    // should refactor all this shit
    int pointIndex = 0;
    static objid baseId = 0;

    drawControlPoints(id, [selectionMode, shaderProgram, &model, &defaultMeshes, &pointIndex, isSelectionShader, id](glm::vec3 point) -> void {
      //modassert(false, "drawControlPoints not yet implemented");

      objid objectId = 0;
      if (selectionMode){
        if (baseId == 0){
          baseId = getUniqueObjIdReserved();
          objectId = baseId;
        }else{
          objectId = getUniqueObjIdReserved();
        }
      }else{
        objectId = baseId + pointIndex;
      }
      pointIndex++;


      static glm::vec4 selectedColor  = glm::vec4(0.f, 0.f, 1.f, 0.5f);
      static glm::vec4 notSelectedColor  = glm::vec4(1.f, 0.f, 0.f, 0.5f);

      auto isSelected = selectedId == objectId;
      glm::vec4 color = isSelected ? glm::vec4(0.f, 0.f, 1.f, 0.5f) : glm::vec4(1.f, 0.f, 0.f, 0.5f);
      //std::cout << "selected: " << selectedId << ", object id: " << objectId << ", isSelected = " << isSelected << ", color = " << print(color) << std::endl;
      shaderSetUniform(shaderProgram, "tint", isSelected ? selectedColor : notSelectedColor);

      auto newModel = glm::translate(model, point);
      renderDefaultNode(shaderProgram, *defaultMeshes.nodeMesh, newModel, id);
    });
    if (!selectionMode){
      baseId = 0;
    }

    return navmeshObj -> meshes.at(0).numTriangles;
  }

  auto textObj = std::get_if<GameObjectUIText>(&toRender);
  if (textObj != NULL){
    shaderSetUniform(shaderProgram, "tint", textObj -> tint);
    return api.drawWord(shaderProgram, id, textObj -> value, 1000.f /* 1000.f => -1,1 range for each quad */, textObj -> align, textObj -> wrap, textObj -> virtualization, textObj -> cursor, textObj -> fontFamily, selectionMode);
  }

  auto prefabObj = std::get_if<GameObjectPrefab>(&toRender);
  if (prefabObj != NULL){
    auto vertexCount = 0;
    if (showDebugMask & 0b10000000){
      vertexCount += renderDefaultNode(shaderProgram, *defaultMeshes.nodeMesh, finalModelMatrix, id);
    }
    return vertexCount;
  }

  //modassert(false, "invalid object type found");
  return 0;
}

std::optional<AttributeValuePtr> getObjectAttributePtr(GameObjectObj& toRender, const char* field){
  auto variantIndex = toRender.index();
  for (auto &objType : objTypes){
    if (variantIndex == objType.variantType){
      return objType.objectAttribute(toRender, field);
    }
  }
  assert(false);
}

bool setObjectAttribute(std::map<objid, GameObjectObj>& mapping, objid id, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags& flags){
  GameObjectObj& toRender = mapping.at(id);
  auto variantIndex = toRender.index();
  for (auto &objType : objTypes){
    if (variantIndex == objType.variantType){
      return objType.setAttribute(toRender, field, value, util, flags);
    }
  }
  std::cout << "obj type not supported" << std::endl;
  assert(false);
  return false;
}
  
std::vector<std::pair<std::string, std::string>> getAdditionalFields(objid id, std::map<objid, GameObjectObj>& mapping, std::function<std::string(int)> getTextureName){
  GameObjectObj objectToSerialize = mapping.at(id);
  auto variantIndex = objectToSerialize.index();
  for (auto &objType : objTypes){
    if (variantIndex == objType.variantType){
      ObjectSerializeUtil serializeUtil {
        .textureName = getTextureName,
      };
      return objType.serialize(objectToSerialize, serializeUtil);
    }
  }
  std::cout << "obj type not supported" << std::endl;
  assert(false);  
  return {};
}


std::vector<objid> getGameObjectsIndex(std::map<objid, GameObjectObj>& mapping){
  std::vector<objid> indicies;
  for (auto [id, _]: mapping){    
      indicies.push_back(id);
  }
  return indicies;
}


std::vector<Mesh> noMeshes;
std::vector<Mesh>& getMeshesForId(std::map<objid, GameObjectObj>& mapping, objid id){  
  std::vector<Mesh*> meshes;
  GameObjectObj& gameObj = mapping.at(id);
  {
    GameObjectMesh* meshObject = std::get_if<GameObjectMesh>(&gameObj);
    if (meshObject != NULL){
      return meshObject -> meshesToRender;
    }
  }

  {
    auto navmeshObject = std::get_if<GameObjectNavmesh>(&gameObj);
    if (navmeshObject != NULL){
      return navmeshObject -> meshes;
    }
  }
  return noMeshes;
}

std::vector<std::string> getMeshNames(std::map<objid, GameObjectObj>& mapping, objid id){
  std::vector<std::string> names;
  if (id == 0){
    return {};
  }
 
  GameObjectObj& gameObj = mapping.at(id);
  GameObjectMesh* meshObject = std::get_if<GameObjectMesh>(&gameObj);
  if (meshObject != NULL){
    return meshObject -> meshNames;
  }
  return {};
}

bool isNavmesh(std::map<objid, GameObjectObj>& mapping, objid id){
  auto object = mapping.at(id); 
  auto navmesh = std::get_if<GameObjectNavmesh>(&object);
  return navmesh != NULL;
}

std::optional<Texture> textureForId(std::map<objid, GameObjectObj>& mapping, objid id){
  auto Object = mapping.at(id); 

  auto meshObj = std::get_if<GameObjectMesh>(&Object);
  if (meshObj != NULL){
    for (int i = 0; i < meshObj -> meshNames.size(); i++){
      if (isRootMeshName(meshObj -> meshNames.at(i))){
        return meshObj -> meshesToRender.at(i).texture;
      }
    }
    std::cout << "WARNING: " << id << " is mesh obj and does not have a mesh" << std::endl;
  }
  return std::nullopt;
}

void updateObjectPositions(std::map<objid, GameObjectObj>& mapping, objid id, glm::vec3 position, Transformation& viewTransform){
  auto object = mapping.at(id); 
  auto soundObj = std::get_if<GameObjectSound>(&object);
  if (soundObj != NULL){
    if (soundObj -> center){
      setSoundPosition(soundObj -> source, viewTransform.position.x, viewTransform.position.y, viewTransform.position.z);
    }else{
      setSoundPosition(soundObj -> source, position.x, position.y, position.z);
    }
  }

  auto lightObj = std::get_if<GameObjectLight>(&object);
  if (lightObj != NULL){
    updateVoxelLightPosition(id, position, lightObj -> voxelSize);
  }
}

void playSoundState(std::map<objid, GameObjectObj>& mapping, objid id, std::optional<float> volume, std::optional<glm::vec3> position){
  if (mapping.find(id) == mapping.end()){
    return;
  }
  auto object = mapping.at(id);
  auto soundObj = std::get_if<GameObjectSound>(&object);
  if (soundObj != NULL){
    playSource(soundObj -> source, volume, position);
  }else{
    std::cout << "WARNING: " << id << " is not a sound object" << std::endl;
  }
}

void stopSoundState(std::map<objid, GameObjectObj>& mapping, objid id){
  if (mapping.find(id) == mapping.end()){
    return;
  }
  auto object = mapping.at(id);
  auto soundObj = std::get_if<GameObjectSound>(&object);
  if (soundObj != NULL){
    stopSource(soundObj -> source);
  }else{
    std::cout << "WARNING: " << id << " is not a sound object" << std::endl;
  }
}

void onObjectSelected(objid id){
  modlog("on object selected: ", std::to_string(id));
  selectedId = id;
}

void onObjectUnselected(){
  modlog("on object unselected: ", "");
  selectedId = 0;
}

std::string getType(std::string name){
  std::string type = "default";
  for (Field field : fields){
    if (name[0] == field.prefix){
      type = field.type;
    }
  }
  return type;
}