#include "./object_types.h"

void shaderLogDebug(const char* str);

ObjectMapping getObjectMapping() {
  ObjectMapping objectMapping {
    .objects = {},
  };
	return objectMapping;
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

void addObjectType(ObjectMapping& objectMapping, objid id, std::string name, GameobjAttributes& attr, ObjectTypeUtil util){
  assert(objectMapping.objects.find(id) == objectMapping.objects.end());
  modlog("objecttype - add", std::to_string(id));
  auto objectType = getType(name);
  if (objectType == "default"){
    objectMapping.objects[id] = createMesh(attr, util);
    return;
  }
  if (objectType == "camera"){
    objectMapping.objects[id] = createCamera(attr, util);
    return;  
  }
  if (objectType == "portal"){
    objectMapping.objects[id] = createPortal(attr, util);
    return;  
  }
  if (objectType == "light"){
    objectMapping.objects[id] = createLight(attr, util);
    return;  
  }
  if (objectType == "camera"){
    objectMapping.objects[id] = createCamera(attr, util);
    return;  
  }
  if (objectType == "sound"){
    objectMapping.objects[id] = createSound(attr, util);
    return;  
  }
  if (objectType == "text"){
    objectMapping.objects[id] = createUIText(attr, util);
    return;  
  }
  if (objectType == "navmesh"){
    objectMapping.objects[id] = createNavmesh(attr, util);
    return;  
  }
  if (objectType == "emitter"){
    objectMapping.objects[id] = createEmitter(attr, util);
    return;  
  }
  if (objectType == "octree"){
    objectMapping.objects[id] = createOctree(attr, util);
    return;  
  }
  if (objectType == "prefab"){
    objectMapping.objects[id] = createPrefabObj(attr, util);
    return;  
  }

  modassert(false, "invalid object type");
}

void removeObject(
  ObjectMapping& objectMapping,
  objid id, 
  std::function<void(std::string)> unbindCamera,
  std::function<void(objid)> unloadScene
){
  if (objectMapping.objects.find(id) == objectMapping.objects.end()){
    return;
  }

  ObjectRemoveUtil util { 
    .id = id, 
    .unloadScene = unloadScene
  };

  GameObjectObj& Object = objectMapping.objects.at(id);
  {
    auto gameobjectCamera = std::get_if<GameObjectCamera>(&Object);
    if (gameobjectCamera){
      // do nothing
      objectMapping.objects.erase(id);
      return;
    }   
  }
  {
    auto gameobjectPortal = std::get_if<GameObjectPortal>(&Object);
    if (gameobjectPortal){
      // do nothing
      objectMapping.objects.erase(id);
      return;
    }   
  }
  {
    auto gameobjectLight = std::get_if<GameObjectLight>(&Object);
    if (gameobjectLight){
      // do nothing
      removeLight(*gameobjectLight, util);
      objectMapping.objects.erase(id);
      return;
    }    
  }
  {
    auto gameobjectSound = std::get_if<GameObjectSound>(&Object);
    if (gameobjectSound){
      // do nothing
      removeSound(*gameobjectSound, util);
      objectMapping.objects.erase(id);
      return;
    }
  }
  {
    auto gameobjectUiText = std::get_if<GameObjectUIText>(&Object);
    if (gameobjectUiText){
      // do nothing
      objectMapping.objects.erase(id);
      return;
    }
  }
  {
    auto gameobjectNavmesh = std::get_if<GameObjectNavmesh>(&Object);
    if (gameobjectNavmesh){
      removeNavmesh(*gameobjectNavmesh, util);
      objectMapping.objects.erase(id);
      return;
    }
  }
  {
    auto gameobjectEmitter = std::get_if<GameObjectEmitter>(&Object);
    if (gameobjectEmitter){
      // do nothing
      removeEmitterObj(*gameobjectEmitter, util);
      objectMapping.objects.erase(id);
      return;
    }
  }
  {
    auto gameobjectMesh = std::get_if<GameObjectMesh>(&Object);
    if (gameobjectMesh){
      // do nothing
      objectMapping.objects.erase(id);
      return;
    }
  }
  {
    auto gameObjectOctree = std::get_if<GameObjectOctree>(&Object);
    if (gameObjectOctree){
      // do nothing
      objectMapping.objects.erase(id);
      return;
    }
  }
  { 
    auto gameObjectPrefab = std::get_if<GameObjectPrefab>(&Object);
    if (gameObjectPrefab){
      removePrefabObj(*gameObjectPrefab, util);
      objectMapping.objects.erase(id);
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
  glm::mat4 model,
  bool drawPoints,
  DefaultMeshes& defaultMeshes,
  bool selectionMode,
  bool drawBones,
  RenderObjApi api,
  glm::mat4& finalModelMatrix
){

  GameObjectObj& toRender = objectMapping.objects.at(id);
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

std::optional<AttributeValuePtr> getObjectAttributePtr(GameObjectObj& Object, const char* field){
  {
    auto gameobjectCamera = std::get_if<GameObjectCamera>(&Object);
    if (gameobjectCamera){
      return getCameraAttribute(*gameobjectCamera, field);
    }   
  }
  {
    auto gameobjectPortal = std::get_if<GameObjectPortal>(&Object);
    if (gameobjectPortal){
      return getPortalAttribute(*gameobjectPortal, field);
    }   
  }
  {
    auto gameobjectLight = std::get_if<GameObjectLight>(&Object);
    if (gameobjectLight){
      return getLightAttribute(*gameobjectLight, field);
    }    
  }
  {
    auto gameobjectSound = std::get_if<GameObjectSound>(&Object);
    if (gameobjectSound){
      return getSoundAttribute(*gameobjectSound, field);
    }
  }
  {
    auto gameobjectUiText = std::get_if<GameObjectUIText>(&Object);
    if (gameobjectUiText){
      return getTextAttribute(*gameobjectUiText, field);
    }
  }
  {
    auto gameobjectNavmesh = std::get_if<GameObjectNavmesh>(&Object);
    if (gameobjectNavmesh){
      return getNavmeshAttribute(*gameobjectNavmesh, field);
    }
  }
  {
    auto gameobjectEmitter = std::get_if<GameObjectEmitter>(&Object);
    if (gameobjectEmitter){
      return getEmitterAttribute(*gameobjectEmitter, field);
    }
  }
  {
    auto gameobjectMesh = std::get_if<GameObjectMesh>(&Object);
    if (gameobjectMesh){
      return getMeshAttribute(*gameobjectMesh, field);
    }
  }
  {
    auto gameObjectOctree = std::get_if<GameObjectOctree>(&Object);
    if (gameObjectOctree){
      return getOctreeAttribute(*gameObjectOctree, field);
    }
  }
  { 
    auto gameObjectPrefab = std::get_if<GameObjectPrefab>(&Object);
    if (gameObjectPrefab){
      return getPrefabAttribute(*gameObjectPrefab, field);
    }
  }
  assert(false);
}

bool setObjectAttribute(ObjectMapping& objectMapping, objid id, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags& flags){
  GameObjectObj& Object = objectMapping.objects.at(id);

  {
    auto gameobjectCamera = std::get_if<GameObjectCamera>(&Object);
    if (gameobjectCamera){
      return setCameraAttribute(*gameobjectCamera, field, value, util, flags);
    }   
  }
  {
    auto gameobjectPortal = std::get_if<GameObjectPortal>(&Object);
    if (gameobjectPortal){
      return setPortalAttribute(*gameobjectPortal, field, value, util, flags);
    }   
  }
  {
    auto gameobjectLight = std::get_if<GameObjectLight>(&Object);
    if (gameobjectLight){
      return setLightAttribute(*gameobjectLight, field, value, util, flags);
    }    
  }
  {
    auto gameobjectSound = std::get_if<GameObjectSound>(&Object);
    if (gameobjectSound){
      return setSoundAttribute(*gameobjectSound, field, value, util, flags);
    }
  }
  {
    auto gameobjectUiText = std::get_if<GameObjectUIText>(&Object);
    if (gameobjectUiText){
      return setTextAttribute(*gameobjectUiText, field, value, util, flags);
    }
  }
  {
    auto gameobjectNavmesh = std::get_if<GameObjectNavmesh>(&Object);
    if (gameobjectNavmesh){
      return false; // do nothing
    }
  }
  {
    auto gameobjectEmitter = std::get_if<GameObjectEmitter>(&Object);
    if (gameobjectEmitter){
      return setEmitterAttribute(*gameobjectEmitter, field, value, util, flags);
    }
  }
  {
    auto gameobjectMesh = std::get_if<GameObjectMesh>(&Object);
    if (gameobjectMesh){
      return setMeshAttribute(*gameobjectMesh, field, value, util, flags);
    }
  }
  {
    auto gameObjectOctree = std::get_if<GameObjectOctree>(&Object);
    if (gameObjectOctree){
      return false;
    }
  }
  { 
    auto gameObjectPrefab = std::get_if<GameObjectPrefab>(&Object);
    if (gameObjectPrefab){
      return setPrefabAttribute(*gameObjectPrefab, field, value, util, flags);
    }
  }
  modassert(false, "setAttribute invalid type");
  return false;
}
  
std::vector<std::pair<std::string, std::string>> getAdditionalFields(objid id, ObjectMapping& objectMapping, std::function<std::string(int)> getTextureName, std::function<void(std::string, std::string&)> saveFile){
  GameObjectObj Object = objectMapping.objects.at(id);
  ObjectSerializeUtil serializeUtil {
    .textureName = getTextureName,
    .saveFile = saveFile,
  };

  {
    auto gameobjectCamera = std::get_if<GameObjectCamera>(&Object);
    if (gameobjectCamera){
      return serializeCamera(*gameobjectCamera, serializeUtil);
    }   
  }
  {
    auto gameobjectPortal = std::get_if<GameObjectPortal>(&Object);
    if (gameobjectPortal){
      return serializePortal(*gameobjectPortal, serializeUtil);
    }   
  }
  {
    auto gameobjectLight = std::get_if<GameObjectLight>(&Object);
    if (gameobjectLight){
      return serializeLight(*gameobjectLight, serializeUtil);
    }    
  }
  {
    auto gameobjectSound = std::get_if<GameObjectSound>(&Object);
    if (gameobjectSound){
      return serializeSound(*gameobjectSound, serializeUtil);
    }
  }
  {
    auto gameobjectUiText = std::get_if<GameObjectUIText>(&Object);
    if (gameobjectUiText){
      return serializeText(*gameobjectUiText, serializeUtil);
    }
  }
  {
    auto gameobjectNavmesh = std::get_if<GameObjectNavmesh>(&Object);
    if (gameobjectNavmesh){
      modassert(false, "serialize navmesh not implemented");
      return {};
    }
  }
  {
    auto gameobjectEmitter = std::get_if<GameObjectEmitter>(&Object);
    if (gameobjectEmitter){
      return serializeEmitter(*gameobjectEmitter, serializeUtil);
    }
  }
  {
    auto gameobjectMesh = std::get_if<GameObjectMesh>(&Object);
    if (gameobjectMesh){
      return serializeMesh(*gameobjectMesh, serializeUtil);
    }
  }
  {
    auto gameObjectOctree = std::get_if<GameObjectOctree>(&Object);
    if (gameObjectOctree){
      return serializeOctree(*gameObjectOctree, serializeUtil);
    }
  }
  { 
    auto gameObjectPrefab = std::get_if<GameObjectPrefab>(&Object);
    if (gameObjectPrefab){
      return serializePrefabObj(*gameObjectPrefab, serializeUtil);
    }
  }
  modassert(false, "serialize objtype not implemented for this type");
  return {};
}

std::vector<Mesh> noMeshes;
std::vector<Mesh>& getMeshesForId(ObjectMapping& mapping, objid id){  
  GameObjectObj& gameObj = mapping.objects.at(id);

  {
    GameObjectMesh* meshObject = std::get_if<GameObjectMesh>(&gameObj);
    if (meshObject != NULL){
      return meshObject -> meshesToRender;
    }
  }

  {
    GameObjectNavmesh* navmeshObject = std::get_if<GameObjectNavmesh>(&gameObj);
    if (navmeshObject != NULL){
      return navmeshObject -> meshes;
    }
  }
  return noMeshes;
}

std::vector<std::string> emptyNames;
std::vector<std::string>& getMeshNames(ObjectMapping& mapping, objid id){
  GameObjectObj& gameObj = mapping.objects.at(id);
  GameObjectMesh* meshObject = std::get_if<GameObjectMesh>(&gameObj);
  if (meshObject != NULL){
    return meshObject -> meshNames;
  }
  return emptyNames;
}

bool isNavmesh(ObjectMapping& mapping, objid id){
  auto object = mapping.objects.at(id); 
  auto navmesh = std::get_if<GameObjectNavmesh>(&object);
  return navmesh != NULL;
}

std::optional<Texture> textureForId(ObjectMapping& mapping, objid id){
  auto Object = mapping.objects.at(id); 

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

void updateObjectPositions(ObjectMapping& mapping, objid id, glm::vec3 position, Transformation& viewTransform){
  auto object = mapping.objects.at(id); 
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

void playSoundState(ObjectMapping& mapping, objid id, std::optional<float> volume, std::optional<glm::vec3> position){
  if (mapping.objects.find(id) == mapping.objects.end()){
    return;
  }
  auto object = mapping.objects.at(id);
  auto soundObj = std::get_if<GameObjectSound>(&object);
  if (soundObj != NULL){
    playSource(soundObj -> source, volume, position);
  }else{
    std::cout << "WARNING: " << id << " is not a sound object" << std::endl;
  }
}

void stopSoundState(ObjectMapping& mapping, objid id){
  if (mapping.objects.find(id) == mapping.objects.end()){
    return;
  }
  auto object = mapping.objects.at(id);
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

GameObjectOctree* getOctree(ObjectMapping& mapping, objid id){
  GameObjectObj& objectOctree = mapping.objects.at(id);
  GameObjectOctree* octreeObject = std::get_if<GameObjectOctree>(&objectOctree);
  return octreeObject;
}