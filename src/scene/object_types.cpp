#include "./object_types.h"

void shaderLogDebug(const char* str);
extern RenderObjApi api;

ObjectMapping getObjectMapping() {
  ObjectMapping objectMapping {
  };
	return objectMapping;
}

ObjectType getObjectType(std::string& name){
  for (Field field : fields){
    if (name.at(0) == field.prefix){
      return field.objectType;
    }
  }
  return OBJ_MESH;
}

void freeMeshBuffer(std::vector<GameObjectMeshBuffer>& meshes, int index){
  std::cout << "freeMeshBuffer: " << index << std::endl;
  meshes.at(index).inUse = false;
}


void addObjectType(ObjectMapping& objectMapping, objid id, std::string name, GameobjAttributes& attr, ObjectTypeUtil util, ObjTypeLookup* _objtypeLookup){
  modassert(!objExists(objectMapping, id), "addObjectType already exists");
  modlog("objecttype - add", std::to_string(id));
  auto objectType = getObjectType(name);
  _objtypeLookup -> id = id;
  if (objectType == OBJ_MESH){
    objectMapping.mesh[id] = createMesh(attr, util);
    _objtypeLookup -> type = OBJ_MESH;
    //{
    //  std::cout << "change gameobject: add " << id << std::endl;
    //  bool added = false;
    //  int index = 0;
    //  for (int i = 0; i < objectMapping.meshBuffer.size(); i++){
    //    if (!objectMapping.meshBuffer.at(i).inUse){
    //      objectMapping.meshBuffer.at(i).inUse = true;
    //      objectMapping.meshBuffer.at(i).mesh = createMesh(attr, util);
    //      added = true;
    //      index = i;
    //      break;
    //    }
    //  }
    //  if (!added){
    //    objectMapping.meshBuffer.push_back(GameObjectMeshBuffer {
    //      .inUse = true,
    //      .mesh = createMesh(attr, util),
    //    });
    //    index = objectMapping.meshBuffer.size() - 1;
    //  }
    //  _objtypeLookup -> index = index;
    //}
    return;
  }
  if (objectType == OBJ_CAMERA){
    objectMapping.camera[id] = createCamera(attr, util);
    _objtypeLookup -> type = OBJ_CAMERA;
    return;  
  }
  if (objectType == OBJ_PORTAL){
    objectMapping.portal[id] = createPortal(attr, util);
    _objtypeLookup -> type = OBJ_PORTAL;
    return;  
  }
  if (objectType == OBJ_LIGHT){
    objectMapping.light[id] = createLight(attr, util);
    _objtypeLookup -> type = OBJ_LIGHT;
    return;  
  }
  if (objectType == OBJ_SOUND){
    objectMapping.sound[id] = createSound(attr, util);
    _objtypeLookup -> type = OBJ_SOUND;
    return;  
  }
  if (objectType == OBJ_TEXT){
    objectMapping.text[id] = createUIText(attr, util);
    _objtypeLookup -> type = OBJ_TEXT;
    return;  
  }
  if (objectType == OBJ_NAVMESH){
    objectMapping.navmesh[id] = createNavmesh(attr, util);
    _objtypeLookup -> type = OBJ_NAVMESH;
    return;  
  }
  if (objectType == OBJ_EMITTER){
    objectMapping.emitter[id] = createEmitter(attr, util);
    _objtypeLookup -> type = OBJ_EMITTER;
    return;  
  }
  if (objectType == OBJ_OCTREE){
    objectMapping.octree[id] = createOctree(attr, util);
    _objtypeLookup -> type = OBJ_OCTREE;
    return;  
  }
  if (objectType == OBJ_PREFAB){
    objectMapping.prefab[id] = createPrefabObj(attr, util);
    _objtypeLookup -> type = OBJ_PREFAB;
    return;  
  }
  if (objectType == OBJ_VIDEO){
    objectMapping.video[id] = createVideoObj(attr, util);
    _objtypeLookup -> type = OBJ_VIDEO;
    return;
  }

  modassert(false, "invalid object type");
}

ObjectType getType(ObjectMapping& objectMapping, objid id){
  modassert(false, "getType invalid type");
  return OBJ_MESH;
}

void removeObject(
  ObjectMapping& objectMapping,
  objid id, 
  std::function<void(std::string)> unbindCamera,
  std::function<void(objid)> unloadScene,
  ObjTypeLookup& _objtypeLookup
){
  if (!objExists(objectMapping, id)){
    return;
  }

  ObjectRemoveUtil util { 
    .id = id, 
    .unloadScene = unloadScene
  };

  if (_objtypeLookup.type == OBJ_MESH){
    goto mesh;
  }
  if (_objtypeLookup.type == OBJ_CAMERA){
    goto camera;
  }
  if (_objtypeLookup.type == OBJ_PORTAL){
    goto portal;
  }
  if (_objtypeLookup.type == OBJ_LIGHT){
    goto light;
  }
  if (_objtypeLookup.type == OBJ_SOUND){
    goto sound;
  }
  if (_objtypeLookup.type == OBJ_EMITTER){
    goto emitter;
  }
  if (_objtypeLookup.type == OBJ_OCTREE){
    goto octree;
  }
  if (_objtypeLookup.type == OBJ_TEXT){
    goto text;
  }
  if (_objtypeLookup.type == OBJ_NAVMESH){
    goto navmesh;
  }
  if (_objtypeLookup.type == OBJ_PREFAB){
    goto prefab;
  }
  if (_objtypeLookup.type == OBJ_VIDEO){
    goto video;
  }

  mesh:
  {
    auto gameobjectMesh = getMesh(objectMapping, id);
    if (gameobjectMesh){
      // do nothing
      objectMapping.mesh.erase(id);
      return;
    }
  }

  camera:
  {
    auto gameobjectCamera = getCameraObj(objectMapping, id);;
    if (gameobjectCamera){
      // do nothing
      objectMapping.camera.erase(id);
      return;
    }   
  }

  portal:
  {
    auto gameobjectPortal = getPortal(objectMapping, id);
    if (gameobjectPortal){
      // do nothing
      objectMapping.portal.erase(id);
      return;
    }   
  }

  light:
  {
    auto gameobjectLight = getLight(objectMapping, id);
    if (gameobjectLight){
      // do nothing
      removeLight(*gameobjectLight, util);
      objectMapping.light.erase(id);
      return;
    }    
  }

  sound:
  {
    auto gameobjectSound = getSoundObj(objectMapping, id);
    if (gameobjectSound){
      // do nothing
      removeSound(*gameobjectSound, util);
      objectMapping.sound.erase(id);
      return;
    }
  }

  emitter:
  {
    auto gameobjectEmitter = getEmitter(objectMapping, id);
    if (gameobjectEmitter){
      // do nothing
      removeEmitterObj(*gameobjectEmitter, util);
      objectMapping.emitter.erase(id);
      return;
    }
  }

  octree:
  {
    auto gameObjectOctree = getOctree(objectMapping, id);
    if (gameObjectOctree){
      // do nothing
      objectMapping.octree.erase(id);
      return;
    }
  }

  text:
  {
    auto gameobjectUiText = getUIText(objectMapping, id);
    if (gameobjectUiText){
      // do nothing
      objectMapping.text.erase(id);
      return;
    }
  }

  navmesh:
  {
    auto gameobjectNavmesh = getNavmesh(objectMapping, id);
    if (gameobjectNavmesh){
      removeNavmesh(*gameobjectNavmesh, util);
      objectMapping.navmesh.erase(id);
      return;
    }
  }

  prefab:
  { 
    auto gameObjectPrefab = getPrefab(objectMapping, id);
    if (gameObjectPrefab){
      removePrefabObj(*gameObjectPrefab, util);
      objectMapping.prefab.erase(id);
      return;
    }
  }

  video:
  {
    auto gameObjectVideo = getVideo(objectMapping, id);
    if (gameObjectVideo){
      removeVideoObj(*gameObjectVideo, util);
      objectMapping.video.erase(id);
      return;
    }
  }

  modassert(false, "invalid gameobj objtype");
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
  ObjectMapping& objectMapping,
  int showDebugMask,
  unsigned int portalTexture,
  unsigned int navmeshTexture,
  glm::mat4& model,
  bool drawPoints,
  DefaultMeshes& defaultMeshes,
  bool selectionMode,
  bool drawBones,
  glm::mat4& finalModelMatrix,
  ObjTypeLookup& lookup
){

  if (lookup.type ==  OBJ_MESH){
    goto mesh;
  }
  if (lookup.type == OBJ_CAMERA){
    goto camera;
  }
  if (lookup.type == OBJ_SOUND){
    goto sound;
  }
  if (lookup.type == OBJ_PORTAL){
    goto portal;
  }
  if (lookup.type == OBJ_LIGHT){
    goto light;
  }
  if (lookup.type == OBJ_OCTREE){
    goto octree;
  }
  if (lookup.type == OBJ_EMITTER){
    goto emitter;
  }
  if (lookup.type == OBJ_NAVMESH){
    goto navmesh;
  }
  if (lookup.type == OBJ_TEXT){
    goto text;
  }
  if (lookup.type == OBJ_PREFAB){
    goto prefab;
  }
  if (lookup.type == OBJ_VIDEO){
    goto video;
  }

  modassert(false, "invalid type, should not have reached here");

  mesh: 
  {
    auto meshObj = getMesh(objectMapping, id);
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
          .bonesGroupModel = meshObj -> bonesGroupModel,
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
      return renderDefaultNode(shaderProgram, *defaultMeshes.nodeMesh, model, id);
    }
  
    if (meshObj != NULL && (meshObj -> meshesToRender.size() > 0) && (showDebugMask & 0b1)) {
      //api.drawLine(glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 100.f, 0.f), glm::vec4(1.f, 0.f, 0.f, 1.f));
      return renderDefaultNode(shaderProgram, *defaultMeshes.nodeMesh, finalModelMatrix, id);
    }
  
    if (meshObj){
      return 0;
    }
  }

  camera: 
  {
    auto cameraObj = getCameraObj(objectMapping, id);
    if (cameraObj != NULL && (showDebugMask & 0b10)){
      auto transform = getTransformationFromMatrix(model).position;
      auto model = glm::translate(glm::mat4(1.f), transform);
      return renderDefaultNode(shaderProgram, *defaultMeshes.cameraMesh, model, id);
    }
  }

  sound:
  {
    auto soundObject = getSoundObj(objectMapping, id);
    if (soundObject != NULL && (showDebugMask & 0b100)){
      return renderDefaultNode(shaderProgram, *defaultMeshes.soundMesh, finalModelMatrix, id);
    }
  }

  portal: 
  {
    auto portalObj = getPortal(objectMapping, id);
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
  }

  light:
  {
    auto lightObj = getLight(objectMapping, id);
    if (lightObj != NULL && (showDebugMask & 0b1000)){   
      return renderDefaultNode(shaderProgram, *defaultMeshes.lightMesh, finalModelMatrix, id);
    }
  }

  octree:
  {
    auto octreeObj = getOctree(objectMapping, id);
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
  }

  emitter:
  {
    auto emitterObj = getEmitter(objectMapping, id);
    if (emitterObj != NULL && (showDebugMask & 0b100000)){
      return renderDefaultNode(shaderProgram, *defaultMeshes.emitter, finalModelMatrix, id);
    }
  }

  navmesh:
  {
    auto navmeshObj = getNavmesh(objectMapping, id);
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
  }

  text:
  {
    auto textObj = getUIText(objectMapping, id);
    if (textObj != NULL){
      shaderSetUniform(shaderProgram, "tint", textObj -> tint);
      return api.drawWord(shaderProgram, id, textObj -> value, 1000.f /* 1000.f => -1,1 range for each quad */, textObj -> align, textObj -> wrap, textObj -> virtualization, textObj -> cursor, textObj -> fontFamily, selectionMode);
    }
  }
  
  prefab:
  {
    auto prefabObj = getPrefab(objectMapping, id);
    if (prefabObj != NULL){
      auto vertexCount = 0;
      if (showDebugMask & 0b10000000){
        vertexCount += renderDefaultNode(shaderProgram, *defaultMeshes.nodeMesh, finalModelMatrix, id);
      }
      return vertexCount;
    }
  }
  
  video:
  {
    auto videoObj = getVideo(objectMapping, id);
    if (videoObj && videoObj -> drawMesh){
      MeshUniforms meshUniforms {
        .model = finalModelMatrix,
        .customTextureId = videoObj -> texture.textureId,
        .id = id,
      };
      drawMesh(*defaultMeshes.portalMesh, shaderProgram, false, meshUniforms);
      return defaultMeshes.portalMesh -> numTriangles;
    }
    //modassert(false, "invalid object type found");
    return 0;
  }
}

std::optional<AttributeValuePtr> getObjectAttributePtr(ObjectMapping& objectMapping, objid id, const char* field, ObjTypeLookup& objtypeLookup){
  modassert(objExists(objectMapping, id), "getObjectAttributePtr obj does not exist");
  
  mesh:
  {
    auto gameobjectMesh = getMesh(objectMapping, id);
    if (gameobjectMesh){
      return getMeshAttribute(*gameobjectMesh, field);
    }
  }
  
  camera:
  {
    auto gameobjectCamera = getCameraObj(objectMapping, id);
    if (gameobjectCamera){
      return getCameraAttribute(*gameobjectCamera, field);
    }   
  }
  
  portal:
  {
    auto gameobjectPortal = getPortal(objectMapping, id);
    if (gameobjectPortal){
      return getPortalAttribute(*gameobjectPortal, field);
    }   
  }
  
  light:
  {
    auto gameobjectLight = getLight(objectMapping, id);
    if (gameobjectLight){
      return getLightAttribute(*gameobjectLight, field);
    }    
  }
  
  sound:
  {
    auto gameobjectSound = getSoundObj(objectMapping, id);
    if (gameobjectSound){
      return getSoundAttribute(*gameobjectSound, field);
    }
  }
  
  text:
  {
    auto gameobjectUiText = getUIText(objectMapping, id);
    if (gameobjectUiText){
      return getTextAttribute(*gameobjectUiText, field);
    }
  }
  
  navmesh:
  {
    auto gameobjectNavmesh = getNavmesh(objectMapping, id);
    if (gameobjectNavmesh){
      return getNavmeshAttribute(*gameobjectNavmesh, field);
    }
  }
  
  emitter:
  {
    auto gameobjectEmitter = getEmitter(objectMapping, id);
    if (gameobjectEmitter){
      return getEmitterAttribute(*gameobjectEmitter, field);
    }
  }
  
  octree:
  {
    auto gameObjectOctree = getOctree(objectMapping, id);
    if (gameObjectOctree){
      return getOctreeAttribute(*gameObjectOctree, field);
    }
  }
  
  prefab:
  { 
    auto gameObjectPrefab = getPrefab(objectMapping, id);
    if (gameObjectPrefab){
      return getPrefabAttribute(*gameObjectPrefab, field);
    }
  }
  
  video:
  {
    auto gameObjectVideo = getVideo(objectMapping, id);
    if (gameObjectVideo){
      return getVideoAttribute(*gameObjectVideo, field);
    }
  }
  modassert(false, "getObjectAttributePtr invalid type");
  return std::optional<AttributeValuePtr>(std::nullopt);;
}

bool setObjectAttribute(ObjectMapping& objectMapping, objid id, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags& flags, ObjTypeLookup& objtypeLookup){
  modassert(objExists(objectMapping, id), "setObjectAttribute id does not exist");
  
  mesh:
  {
    auto gameobjectMesh = getMesh(objectMapping, id);
    if (gameobjectMesh){
      return setMeshAttribute(*gameobjectMesh, field, value, util, flags);
    }
  }

  camera:
  {
    auto gameobjectCamera = getCameraObj(objectMapping, id);
    if (gameobjectCamera){
      return setCameraAttribute(*gameobjectCamera, field, value, util, flags);
    }   
  }
  
  portal:
  {
    auto gameobjectPortal = getPortal(objectMapping, id);
    if (gameobjectPortal){
      return setPortalAttribute(*gameobjectPortal, field, value, util, flags);
    }   
  }
  
  light:
  {
    auto gameobjectLight = getLight(objectMapping, id);
    if (gameobjectLight){
      return setLightAttribute(*gameobjectLight, field, value, util, flags);
    }    
  }
  
  sound:
  {
    auto gameobjectSound = getSoundObj(objectMapping, id);
    if (gameobjectSound){
      return setSoundAttribute(*gameobjectSound, field, value, util, flags);
    }
  }
  
  text:
  {
    auto gameobjectUiText = getUIText(objectMapping, id);
    if (gameobjectUiText){
      return setTextAttribute(*gameobjectUiText, field, value, util, flags);
    }
  }
  
  navmesh:
  {
    auto gameobjectNavmesh = getNavmesh(objectMapping, id);
    if (gameobjectNavmesh){
      return false; // do nothing
    }
  }
  
  emitter:
  {
    auto gameobjectEmitter = getEmitter(objectMapping, id);
    if (gameobjectEmitter){
      return setEmitterAttribute(*gameobjectEmitter, field, value, util, flags);
    }
  }
  
  octree:
  {
    auto gameObjectOctree = getOctree(objectMapping, id);
    if (gameObjectOctree){
      return false;
    }
  }
  
  prefab:
  { 
    auto gameObjectPrefab = getPrefab(objectMapping, id);
    if (gameObjectPrefab){
      return setPrefabAttribute(*gameObjectPrefab, field, value, util, flags);
    }
  }
  
  video:
  {
    auto gameObjectVideo = getVideo(objectMapping, id);
    if (gameObjectVideo){
      return setVideoAttribute(*gameObjectVideo, field, value, util, flags);
    }
  }
  modassert(false, "setAttribute invalid type");
  return false;
}
  
std::vector<std::pair<std::string, std::string>> getAdditionalFields(objid id, ObjectMapping& objectMapping, std::function<std::string(int)> getTextureName, std::function<void(std::string, std::string&)> saveFile, ObjTypeLookup& objtypeLookup){
  modassert(objExists(objectMapping, id), "getAdditionalFields obj does not exist");
  ObjectSerializeUtil serializeUtil {
    .textureName = getTextureName,
    .saveFile = saveFile,
  };

  mesh:
  {
    auto gameobjectMesh = getMesh(objectMapping, id);
    if (gameobjectMesh){
      return serializeMesh(*gameobjectMesh, serializeUtil);
    }
  }
  
  camera:
  {
    auto gameobjectCamera = getCameraObj(objectMapping, id);
    if (gameobjectCamera){
      return serializeCamera(*gameobjectCamera, serializeUtil);
    }   
  }
  
  portal:
  {
    auto gameobjectPortal = getPortal(objectMapping, id);
    if (gameobjectPortal){
      return serializePortal(*gameobjectPortal, serializeUtil);
    }   
  }
  
  light:
  {
    auto gameobjectLight = getLight(objectMapping, id);
    if (gameobjectLight){
      return serializeLight(*gameobjectLight, serializeUtil);
    }    
  }
  
  sound:
  {
    auto gameobjectSound = getSoundObj(objectMapping, id);
    if (gameobjectSound){
      return serializeSound(*gameobjectSound, serializeUtil);
    }
  }
  
  text:
  {
    auto gameobjectUiText = getUIText(objectMapping, id);
    if (gameobjectUiText){
      return serializeText(*gameobjectUiText, serializeUtil);
    }
  }
  
  navmesh:
  {
    auto gameobjectNavmesh = getNavmesh(objectMapping, id);
    if (gameobjectNavmesh){
      modassert(false, "serialize navmesh not implemented");
      return {};
    }
  }
  
  emitter:
  {
    auto gameobjectEmitter = getEmitter(objectMapping, id);
    if (gameobjectEmitter){
      return serializeEmitter(*gameobjectEmitter, serializeUtil);
    }
  }

  octree:
  {
    auto gameObjectOctree = getOctree(objectMapping, id);
    if (gameObjectOctree){
      return serializeOctree(*gameObjectOctree, serializeUtil);
    }
  }
  
  prefab:
  { 
    auto gameObjectPrefab = getPrefab(objectMapping, id);
    if (gameObjectPrefab){
      return serializePrefabObj(*gameObjectPrefab, serializeUtil);
    }
  }
  
  video:
  {
    auto gameObjectVideo = getVideo(objectMapping, id);
    if (gameObjectVideo){
      return serializeVideo(*gameObjectVideo, serializeUtil);
    }
  }
  modassert(false, "serialize objtype not implemented for this type");
  return {};
}

std::vector<Mesh> noMeshes;
std::vector<Mesh>& getMeshesForId(ObjectMapping& mapping, objid id, ObjTypeLookup& objtypeLookup){  
  {
    GameObjectMesh* meshObject = getMesh(mapping, id);
    if (meshObject != NULL){
      return meshObject -> meshesToRender;
    }
  }

  {
    GameObjectNavmesh* navmeshObject = getNavmesh(mapping, id);
    if (navmeshObject != NULL){
      return navmeshObject -> meshes;
    }
  }
  return noMeshes;
}

std::vector<std::string> emptyNames;
std::vector<std::string>& getMeshNames(ObjectMapping& mapping, objid id, ObjTypeLookup& objtypeLookup){
  GameObjectMesh* meshObject = getMesh(mapping, id);
  if (meshObject != NULL){
    return meshObject -> meshNames;
  }
  return emptyNames;
}

bool isNavmesh(ObjectMapping& mapping, objid id){
  return getNavmesh(mapping, id) != NULL;
}

std::optional<Texture> textureForId(ObjectMapping& mapping, objid id, ObjTypeLookup& objtypeLookup){
  auto meshObj = getMesh(mapping, id);
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

void updateObjectPositions(ObjectMapping& mapping, objid id, glm::vec3 position, Transformation& viewTransform){
  auto soundObj = getSoundObj(mapping, id);
  if (soundObj != NULL){
    if (soundObj -> center){
      setSoundPosition(soundObj -> source, viewTransform.position.x, viewTransform.position.y, viewTransform.position.z);
    }else{
      setSoundPosition(soundObj -> source, position.x, position.y, position.z);
    }
  }

  auto lightObj = getLight(mapping, id);
  if (lightObj != NULL){
    updateVoxelLightPosition(id, position, lightObj -> voxelSize);
  }
}

void playSoundState(ObjectMapping& mapping, objid id, std::optional<float> volume, std::optional<glm::vec3> position){
  auto soundObj = getSoundObj(mapping, id);
  if (soundObj != NULL){
    playSource(soundObj -> source, volume, position);
  }else{
    std::cout << "WARNING: " << id << " is not a sound object" << std::endl;
  }
}

void stopSoundState(ObjectMapping& mapping, objid id){
  auto soundObj = getSoundObj(mapping, id);
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

GameObjectOctree* getOctree(ObjectMapping& mapping, objid id){
  auto it = mapping.octree.find(id);
  if (it == mapping.octree.end()) {
      return NULL;
  }
  return &it->second;
}

GameObjectNavmesh* getNavmesh(ObjectMapping& mapping, objid id){
  auto it = mapping.navmesh.find(id);
  if (it == mapping.navmesh.end()) {
      return NULL;
  }
  return &it->second;
}

GameObjectLight* getLight(ObjectMapping& mapping, objid id){
  auto it = mapping.light.find(id);
  if (it == mapping.light.end()) {
      return NULL;
  }
  return &it->second;
}

GameObjectPortal* getPortal(ObjectMapping& mapping, objid id){
  auto it = mapping.portal.find(id);
  if (it == mapping.portal.end()) {
      return NULL;
  }
  return &it->second;
}

GameObjectMesh* getMesh(ObjectMapping& mapping, objid id){
  auto it = mapping.mesh.find(id);
  if (it == mapping.mesh.end()) {
      return NULL;
  }
  return &it->second;
}

GameObjectPrefab* getPrefab(ObjectMapping& mapping, objid id){
  auto it = mapping.prefab.find(id);
  if (it == mapping.prefab.end()) {
      return NULL;
  }
  return &it->second;
}

GameObjectVideo* getVideo(ObjectMapping& mapping, objid id){
  auto it = mapping.video.find(id);
  if (it == mapping.video.end()) {
      return NULL;
  }
  return &it->second;
}


GameObjectCamera* getCameraObj(ObjectMapping& mapping, objid id){
  auto it = mapping.camera.find(id);
  if (it == mapping.camera.end()) {
      return NULL;
  }
  return &it->second;
}

GameObjectUIText* getUIText(ObjectMapping& mapping, objid id){
  auto it = mapping.text.find(id);
  if (it == mapping.text.end()) {
      return NULL;
  }
  return &it->second;
}

GameObjectSound* getSoundObj(ObjectMapping& mapping, objid id){
  auto it = mapping.sound.find(id);
  if (it == mapping.sound.end()) {
      return NULL;
  }
  return &it->second;
}

GameObjectEmitter* getEmitter(ObjectMapping& mapping, objid id){
  auto it = mapping.emitter.find(id);
  if (it == mapping.emitter.end()) {
      return NULL;
  }
  return &it->second;
}

std::vector<objid> getAllLightsIndexs(ObjectMapping& mapping){
  std::vector<objid> ids;
  for (auto &[id, _] : mapping.light){
    ids.push_back(id);
  }
  return ids;
}

std::vector<objid> getAllPortalIndexs(ObjectMapping& mapping){
  std::vector<objid> ids;
  for (auto &[id, _] : mapping.portal){
    ids.push_back(id);
  }
  return ids;  
}

std::vector<objid> getAllCameraIndexs(ObjectMapping& mapping){
  std::vector<objid> ids;
  for (auto &[id, _] : mapping.camera){
    ids.push_back(id);
  }
  return ids;  
}

bool objExists(ObjectMapping& mapping, objid id){
  if (mapping.mesh.find(id) != mapping.mesh.end()){
    return true;
  }
  if (mapping.camera.find(id) != mapping.camera.end()){
    return true;
  }
  if (mapping.portal.find(id) != mapping.portal.end()){
    return true;
  }
  if (mapping.sound.find(id) != mapping.sound.end()){
    return true;
  }
  if (mapping.light.find(id) != mapping.light.end()){
    return true;
  }
  if (mapping.octree.find(id) != mapping.octree.end()){
    return true;
  }
  if (mapping.emitter.find(id) != mapping.emitter.end()){
    return true;
  }
  if (mapping.navmesh.find(id) != mapping.navmesh.end()){
    return true;
  }
  if (mapping.text.find(id) != mapping.text.end()){
    return true;
  }
  if (mapping.prefab.find(id) != mapping.prefab.end()){
    return true;
  }
  if (mapping.video.find(id) != mapping.video.end()){
    return true;
  }
  return false;
}