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

void setShaderObjectData(GLint shaderProgram, bool hasBones, glm::vec4 tint, glm::vec2 textureOffset, glm::vec2 textureTiling, glm::vec2 textureSize, glm::vec3 emissionAmount){
  shaderSetUniformBool(shaderProgram, "hasBones", hasBones);    
  shaderSetUniform(shaderProgram, "tint", tint);
  shaderSetUniform(shaderProgram, "textureOffset", textureOffset);
  shaderSetUniform(shaderProgram, "textureTiling", textureTiling);
  shaderSetUniform(shaderProgram, "textureSize", textureSize);
  shaderSetUniform(shaderProgram, "emissionAmount", emissionAmount);
}

int renderDefaultNode(GLint shaderProgram, Mesh& mesh, glm::mat4& matrix){
  // Transformation getTransformationFromMatrix(glm::mat4 matrix){
  // unscale this model matrix
  setShaderObjectData(shaderProgram, mesh.bones.size() > 0, glm::vec4(1.f, 1.f, 0.f, 1.f), glm::vec2(0.f, 0.f), glm::vec2(1.f, 1.f), glm::vec2(1.f, 1.f), glm::vec3(0.f, 0.f, 0.f));

  MeshUniforms meshUniforms {
    .model = matrix,
  };
  drawMesh(mesh, shaderProgram, -1, -1, false, -1, meshUniforms);
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
      bool hasBones = false;
      if (meshToRender.bones.size() > 0){
        for (int i = 0; i < 100; i++){
          auto boneUniformLocation = glGetUniformLocation(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str());
          if (i >= meshToRender.bones.size()){
            shaderSetUniform(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str(), glm::mat4(1.f));
          }else{
            shaderSetUniform(shaderProgram, ("bones[" + std::to_string(i) + "]").c_str(), meshToRender.bones.at(i).offsetMatrix);
          }
        }
        hasBones = true;
      }

      setShaderObjectData(shaderProgram, hasBones, meshObj -> tint, meshObj -> texture.textureoffset, meshObj -> texture.texturetiling, meshObj -> texture.texturesize, meshObj -> emissionAmount);
      shaderLogDebug((std::string("draw mesh: ") + meshObj -> meshNames.at(x)).c_str());

      MeshUniforms meshUniforms {
        .model = finalModelMatrix,
      };
      drawMesh(meshToRender, shaderProgram, meshObj -> texture.loadingInfo.textureId, -1, drawPoints, meshObj -> normalTexture.textureId, meshUniforms);   
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
    renderDefaultNode(shaderProgram, *defaultMeshes.nodeMesh, model);
  }
  if (meshObj != NULL && (meshObj -> meshesToRender.size() > 0) && (showDebugMask & 0b1)) {
    //api.drawLine(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 100.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 1.f));
    return renderDefaultNode(shaderProgram, *defaultMeshes.nodeMesh, finalModelMatrix);
  }

  auto cameraObj = std::get_if<GameObjectCamera>(&toRender);
  if (cameraObj != NULL && (showDebugMask & 0b10)){
    auto transform = getTransformationFromMatrix(model).position;
    auto model = glm::translate(glm::mat4(1.f), transform);
    return renderDefaultNode(shaderProgram, *defaultMeshes.cameraMesh, model);
  }

  auto soundObject = std::get_if<GameObjectSound>(&toRender);
  if (soundObject != NULL && (showDebugMask & 0b100)){
    return renderDefaultNode(shaderProgram, *defaultMeshes.soundMesh, finalModelMatrix);
  }


  auto portalObj = std::get_if<GameObjectPortal>(&toRender);
  if (portalObj != NULL){
    setShaderObjectData(shaderProgram, defaultMeshes.nodeMesh -> bones.size() > 0, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec2(0.f, 0.f), glm::vec2(1.f, 1.f), glm::vec2(1.f, 1.f), glm::vec3(0.f, 0.f, 0.f));

    MeshUniforms meshUniforms {
      .model = finalModelMatrix,
    };
    drawMesh(*defaultMeshes.portalMesh, shaderProgram, portalTexture, -1, false, -1, meshUniforms);
    return defaultMeshes.portalMesh -> numTriangles;
  }

  auto lightObj = std::get_if<GameObjectLight>(&toRender);
  if (lightObj != NULL && (showDebugMask & 0b1000)){   
    return renderDefaultNode(shaderProgram, *defaultMeshes.lightMesh, finalModelMatrix);
  }

  auto octreeObj = std::get_if<GameObjectOctree>(&toRender);
  if (octreeObj != NULL){
    setShaderObjectData(shaderProgram, false, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec2(0.f, 0.f), glm::vec2(1.f, 1.f), glm::vec2(1.f, 1.f), glm::vec3(0.f, 0.f, 0.f));
    Mesh* octreeMesh = getOctreeMesh(*octreeObj);
    modassert(octreeMesh, "no octree mesh available");

    MeshUniforms meshUniforms {
      .model = finalModelMatrix,
    };
    drawMesh(*octreeMesh, shaderProgram, -1, -1, false, -1, meshUniforms);
    return octreeMesh -> numTriangles;
  }

  auto emitterObj = std::get_if<GameObjectEmitter>(&toRender);
  if (emitterObj != NULL && (showDebugMask & 0b100000)){
    return renderDefaultNode(shaderProgram, *defaultMeshes.emitter, finalModelMatrix);
  }

  auto navmeshObj = std::get_if<GameObjectNavmesh>(&toRender);
  if (navmeshObj != NULL){
    setShaderObjectData(shaderProgram, false, glm::vec4(1.f, 1.f, 1.f, 1.f), glm::vec2(0.f, 0.f), glm::vec2(1.f, 1.f), glm::vec2(1.f, 1.f), glm::vec3(0.f, 0.f, 0.f));

    MeshUniforms meshUniforms {
      .model = finalModelMatrix,
    };
    drawMesh(navmeshObj -> mesh, shaderProgram, navmeshTexture, -1, false, -1, meshUniforms);    


    // this base id point index stuff is pretty hackey bullshit
    // maybe i should pull out a function to render the object only in selection mode. 
    // should refactor all this shit
    int pointIndex = 0;
    static objid baseId = 0;

    drawControlPoints(id, [selectionMode, shaderProgram, &model, &defaultMeshes, &pointIndex, isSelectionShader](glm::vec3 point) -> void {
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
      shaderSetUniform(shaderProgram, "encodedid", getColorFromGameobject(objectId));
      shaderSetUniform(shaderProgram, "tint", isSelected ? selectedColor : notSelectedColor);

      auto newModel = glm::translate(model, point);
      renderDefaultNode(shaderProgram, *defaultMeshes.nodeMesh, newModel);
    });
    if (!selectionMode){
      baseId = 0;
    }

    return navmeshObj -> mesh.numTriangles;
  }

  auto textObj = std::get_if<GameObjectUIText>(&toRender);
  if (textObj != NULL){
    setShaderObjectData(shaderProgram, false, textObj -> tint, glm::vec2(0.f, 0.f), glm::vec2(1.f, 1.f), glm::vec2(1.f, 1.f), glm::vec3(0.f, 0.f, 0.f));   
    return api.drawWord(shaderProgram, id, textObj -> value, 1000.f /* 1000.f => -1,1 range for each quad */, textObj -> align, textObj -> wrap, textObj -> virtualization, textObj -> cursor, textObj -> fontFamily, selectionMode);
  }

  auto prefabObj = std::get_if<GameObjectPrefab>(&toRender);
  if (prefabObj != NULL){
    auto vertexCount = 0;
    if (showDebugMask & 0b10000000){
      vertexCount += renderDefaultNode(shaderProgram, *defaultMeshes.nodeMesh, finalModelMatrix);
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


static std::string EMPTY_STR = "";
NameAndMesh getMeshesForId(std::map<objid, GameObjectObj>& mapping, objid id){  
  static std::string NULL_STR = "";

  std::vector<std::string*> meshNames;
  std::vector<Mesh*> meshes;

  GameObjectObj& gameObj = mapping.at(id);

  {
    auto meshObject = std::get_if<GameObjectMesh>(&gameObj);
    if (meshObject != NULL){
      for (int i = 0; i < meshObject -> meshesToRender.size(); i++){
        meshNames.push_back(&meshObject -> meshNames.at(i));
        meshes.push_back(&meshObject -> meshesToRender.at(i));
      }
      goto returndata;
    }
  }

  {
    auto navmeshObject = std::get_if<GameObjectNavmesh>(&gameObj);
    if (navmeshObject != NULL){
      meshNames.push_back(&EMPTY_STR); // this should just be an optional array... need to clean up getmeshees in general
      meshes.push_back(&navmeshObject -> mesh);
    }
    goto returndata;
  }

returndata:

  NameAndMesh meshData {
    .meshNames = meshNames,
    .meshes = meshes
  };

  assert(meshNames.size() == meshes.size());
  return meshData;
}

std::vector<std::string> getMeshNames(std::map<objid, GameObjectObj>& mapping, objid id){
  std::vector<std::string> names;
  if (id == 0){
    return names;
  }
  for (auto name : getMeshesForId(mapping, id).meshNames){
    names.push_back(*name);
  }

  return names;
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