#include "./main_api.h"

extern World world;
extern RenderStages renderStages;
extern SysInterface interface;
extern WorldTiming timings;
extern engineState state;
extern DefaultResources defaultResources;
extern unsigned int uiShaderProgram;
extern Stats statistics;
extern std::queue<StringAttribute> channelMessages;

extern bool bootStrapperMode;
extern NetCode netcode;
extern DrawingParams drawParams;
extern DynamicLoading dynamicLoading;
extern std::map<std::string, objid> activeLocks;
extern CScriptBindingCallbacks cBindings;
extern LineData lineData;
extern Transformation viewTransform;

extern GLFWwindow* window;
extern GLFWmonitor* monitor;
extern const GLFWvidmode* mode;
extern TimePlayback timePlayback;
extern ManipulatorTools tools;

extern std::string sqlDirectory;

float getTotalTime(){
  return statistics.now - statistics.initialTime;
}

std::vector<objid> objectsQueuedForRemoval = {};
std::vector<objid> groupsQueuedForRemoval = {};
bool gameobjExists(objid id){
  if (!idExists(world.sandbox, id)){
    return false;
  }
  auto groupId = getGroupId(world.sandbox, id); 
  for (auto idToRemove : objectsQueuedForRemoval){
    if (id == idToRemove){
      return false;
    }
  }
  for (auto idToRemove : groupsQueuedForRemoval){
    if (groupId == idToRemove){
      return false;
    }
  }
  return true;
}

std::optional<objid> getGameObjectByName(std::string name, objid sceneId, bool sceneIdExplicit){    
  return getGameObjectByNamePrefix(world, name, sceneId, sceneIdExplicit);
}


btRigidBody* getRigidBody(int32_t index){
  return world.rigidbodys.find(index) == world.rigidbodys.end() ? NULL : world.rigidbodys.at(index).body;
}
void applyImpulse(int32_t index, glm::vec3 impulse){
  auto rigidBody = getRigidBody(index);
  if (rigidBody != NULL){
    applyImpulse(rigidBody, impulse);
  }
}
void applyImpulseRel(int32_t index, glm::vec3 impulse){
  auto rigidBody = getRigidBody(index);
  if (rigidBody != NULL){
    auto relativeRotation = gameobjectRotation(world, index, false);
    glm::vec3 relativeImpulse = calculateRelativeOffset(relativeRotation, impulse, true);
    applyImpulse(rigidBody, relativeImpulse);
  }
}

void clearImpulse(int32_t index){
  auto rigidBody = getRigidBody(index);
  if (rigidBody != NULL){
    clearImpulse(rigidBody);
  }
}

void applyForce(int32_t index, glm::vec3 force){
  auto rigidBody = getRigidBody(index);
  if (rigidBody != NULL){
    applyForce(rigidBody, force);
  }
}
void applyTorque(int32_t index, glm::vec3 torque){
  auto rigidBody = getRigidBody(index);
  if (rigidBody != NULL){
    applyTorque(rigidBody, torque);
  }
}

std::optional<ModAABB> getModAABB(int32_t index){
  auto rigidBody = getRigidBody(index);
  if (rigidBody == NULL){
    return std::nullopt;
  }
  return getModAABB(rigidBody);
}


std::vector<std::string> listSceneFiles(std::optional<objid> sceneId){
  if (sceneId.has_value()){
    if (!sceneExists(world.sandbox, sceneId.value())){
      return {};
    }
    return { sceneFileForSceneId(world, sceneId.value()) };
  }
  return listFilesWithExtensions("./res/scenes", { "rawscene" });
}

bool parentScene(objid sceneId, objid* _parentSceneId){
  return parentSceneId(world.sandbox, sceneId, _parentSceneId);
}

std::vector<objid> childScenes(objid sceneId){
  return childSceneIds(world.sandbox, sceneId);
}


int32_t loadSceneWithId(std::string sceneFile, std::vector<std::vector<std::string>> additionalTokens, std::optional<std::string> name, std::optional<std::vector<std::string>> tags, std::optional<objid> id){
  std::cout << "INFO: SCENE LOADING: loading " << sceneFile << std::endl;
  std::vector<Token> addedTokens;
  for (auto &tokens : additionalTokens){
    addedTokens.push_back(Token{
      .target = tokens.at(0),
      .attribute = tokens.at(1),
      .payload = tokens.at(2),
    });
  }
  return addSceneToWorld(world, sceneFile, addedTokens, name, tags, id);
}

int32_t loadScene(std::string sceneFile, std::vector<std::vector<std::string>> additionalTokens, std::optional<std::string> name, std::optional<std::vector<std::string>> tags){
  return loadSceneWithId(sceneFile, additionalTokens, name, tags, std::nullopt);
}


int32_t loadSceneParentOffset(std::string sceneFile, glm::vec3 offset, std::string parentNodeName){
  auto name = std::to_string(getUniqueObjId()) + parentNodeName;
  GameobjAttributes attr {
    .stringAttributes = {}, 
    .numAttributes = {}, 
    .vecAttr = vectorAttributes { .vec3 = {{"position", offset}}, .vec4 = {} },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes = {};
  auto nodeOffsetId = makeObjectAttr(world.sandbox.mainScene.rootId, name, attr, submodelAttributes).value();
  std::cout << "load scene offset: " << print(offset) << std::endl;
  auto sceneId = loadScene(sceneFile, {}, std::nullopt, std::nullopt);
  auto rootId = rootIdForScene(world.sandbox, sceneId);
  makeParent(world.sandbox, rootId, nodeOffsetId);
  return nodeOffsetId;
}

std::optional<objid> sceneIdByName(std::string name){
  return sceneIdByName(world.sandbox, name);
}

objid rootIdForScene(objid sceneId){
  return rootIdForScene(world.sandbox, sceneId);
}

objid rootSceneId(){
  return rootSceneId(world.sandbox);
}

std::vector<int32_t> queuedUnloadScenes = {};

void unloadScene(int32_t sceneId){  
  std::cout << "INFO: SCENE LOADING: unloading " << sceneId << std::endl;
  queuedUnloadScenes.push_back(sceneId);
}
void doUnloadScenes(){
  for (auto sceneId : queuedUnloadScenes){
    removeSceneFromWorld(world, sceneId);
  }
  queuedUnloadScenes = {};
}
void unloadAllScenes(){
  removeAllScenesFromWorld(world);
}

// This implementation could be a lot better.  
// This ought to just update the old attributes to the new ones instead of unloaded/reloading
void resetScene(std::optional<objid> sceneId){
  std::cout << "reset scene placeholder for: " << (sceneId.has_value() ? std::to_string(sceneId.value()) : "no sceneid") << std::endl;
  if (sceneId.has_value()){
    auto sceneFile = sceneFileForSceneId(world, sceneId.value());
    auto sceneName = sceneNameForSceneId(world, sceneId.value());
    auto sceneTags = sceneTagsForSceneId(world, sceneId.value());

    auto parentObjId = listParentObjId(world.sandbox, sceneId.value());
    unloadScene(sceneId.value());
    loadSceneWithId(sceneFile, {}, sceneName, sceneTags, sceneId.value()); // additional args get lost, maybe i should keep this data around? 
    if (parentObjId.has_value()){
      auto rootObjIdNewScene = rootIdForScene(world.sandbox, sceneId.value());
      makeParent(world.sandbox, rootObjIdNewScene, parentObjId.value());
    }
  }

}

bool isNotWriteProtectedFile(std::string& fileToSave){
  auto fileExtension = getPreExtension(fileToSave);
  auto allowedToSave = true;
  if (fileExtension.has_value()){
    auto extension = fileExtension.value();
    std::cout << "extension is: " << extension << std::endl;
    allowedToSave = extension != "p";
  }
  return allowedToSave;
}

bool saveScene(bool includeIds, objid sceneId, std::optional<std::string> filename){
  auto fileToSave = sceneFileForSceneId(world, sceneId);    // MAYBE SHOULD CREATE A CACHE OF WHAT FILE WAS WHAT SCENE?
  fileToSave = filename.has_value() ? filename.value() : fileToSave;
  std::cout << "saving scene id: " << sceneId << " to file: " << fileToSave << std::endl;
  if (isNotWriteProtectedFile(fileToSave)){
    saveFile(fileToSave, serializeScene(world, sceneId, includeIds));
    return true;
  }
  std::cout << "WARNING: CANNOT SAVE: " << fileToSave << " because is a protected file" << std::endl;
  return false;
}

void saveHeightmap(objid id, std::optional<std::string> filename){
  modassert(filename.has_value(), "save heightmap required specified filename (for now)");
  saveHeightmap(world, id, filename.value());
}

std::vector<int32_t> listScenes(std::optional<std::vector<std::string>> tags){
  return allSceneIds(world.sandbox, tags);
}

std::vector<ScenegraphDebug> scenegraph(){
  std::vector<ScenegraphDebug> parentToChild;
  auto dotRelations = getDotRelations(world.sandbox, world.objectMapping);
  for (auto &dotRelation : dotRelations){
    if (dotRelation.parent.has_value()){
      parentToChild.push_back(ScenegraphDebug{
        .parent = dotRelation.parent.value().name,
        .parentId = dotRelation.parent.value().id,
        .child = dotRelation.child.name,
        .childId = dotRelation.child.id,
        .parentScene = dotRelation.parent.value().sceneId,
        .childScene = dotRelation.child.sceneId,
      });
    }
  }

  std::sort(parentToChild.begin(), parentToChild.end(), [](ScenegraphDebug one, ScenegraphDebug two) {
    int value = one.parent.compare(two.parent);
    if (value != 0){
      return value < 0;
    }
    return one.child.compare(two.child) < 0;
  });
  return parentToChild;
}

void sendLoadScene(int32_t id){
  sendLoadScene(world, netcode, bootStrapperMode, id);
}

void createScene(std::string scenename){
  auto extension = getExtension(scenename);
  bool canSave = extension.has_value() && extension == "rawscene";
  assert(canSave);
  saveFile(scenename, "");
}

void deleteScene(std::string scenename){
  auto extension = getExtension(scenename);
  bool canSave = extension.has_value() && extension == "rawscene";
  assert(canSave && isNotWriteProtectedFile(scenename));
  rmFile(scenename);
}

std::vector<int32_t> getObjectsByType(std::string type){
  if (type == "mesh"){
    std::vector indexes = getGameObjectsIndex<GameObjectMesh>(world.objectMapping);
    return indexes;
  }else if (type == "camera"){
    std::vector indexes = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
    return indexes;
  }
  return getGameObjectsIndex(world.objectMapping);
}
std::vector<int32_t> getObjectsByAttr(std::string type, std::optional<AttributeValue> value, std::optional<int32_t> sceneId){
  auto objIds = listObjInScene(world.sandbox, sceneId);
  std::vector<objid> idsWithAttrs;
  for (auto id : objIds){
    if (objectHasAttribute(world, id, type, value)){
      idsWithAttrs.push_back(id);
    }
  }
  return idsWithAttrs;
}

std::optional<std::string> getGameObjectName(int32_t index){
  if (!idExists(world.sandbox, index)){
    return std::nullopt;
  }
  return getGameObject(world, index).name;
}

GameobjAttributes getGameObjectAttr(int32_t id){
  return objectAttributes(world, id);
}

void setGameObjectAttr(int32_t id, GameobjAttributes& attr){
  modassert(idExists(world.sandbox, id), std::string("object does not exist: ") + std::to_string(id));
  setAttributes(world, id, attr);
}


//////////////////////////////////////////////////////////////
glm::vec3 getGameObjectPosition(int32_t index, bool isWorld){
  return gameobjectPosition(world, index, isWorld);
}

void setGameObjectPosition(int32_t index, glm::vec3 pos, bool isWorld){
  physicsTranslateSet(world, index, pos, !isWorld);
}

glm::vec3 getGameObjectScale(int32_t index){
  return gameobjectScale(world, index, false);
}
glm::vec3 getGameObjectScale2(int32_t index, bool isWorld){
  return gameobjectScale(world, index, isWorld);
}
void setGameObjectScale(int32_t index, glm::vec3 scale, bool isWorld){
  modassert(isWorld, "relative set scale not yet supported");
  physicsScaleSet(world, index, scale);
}

glm::quat getGameObjectRotation(int32_t index, bool isWorld){
  return gameobjectRotation(world, index, isWorld);
}

void setGameObjectRotation(int32_t index, glm::quat rotation, bool isWorld){
  physicsRotateSet(world, index, rotation, !isWorld);
}


///////////////////////


std::optional<objid> makeObjectAttr(objid sceneId, std::string name, GameobjAttributes& attributes, std::map<std::string, GameobjAttributes>& submodelAttributes){
  AttrChildrenPair attrWithChildren {
    .attr = attributes,
    .children = {},
  };

  if (idExists(world.sandbox, name, sceneId)){
    modlog("gameobj creation", std::string("object with name: ") + name + " already exists in scene: " + std::to_string(sceneId), MODLOG_WARNING);
    return std::nullopt;
  }
  return addObjectToScene(world, sceneId, name, attrWithChildren, submodelAttributes);
}

void copyObject(int32_t id){
  bool success = copyObjectToScene(world, id);
  if (!success){
    sendAlert(std::string("failure copying object: ") + std::to_string(id));
  }
}
void handleCopy(){
  modlog("clipboard", "pasting objects");
  copyAllObjects(state.editor, copyObject);
}
void handleClipboardSelect(){
  auto numObject = state.editor.selectedObjs.size();
  if (numObject == 0 && state.editor.clipboardObjs.size() > 0){
    sendAlert("cleared clipboard");
  }
  else if (numObject == 1){
    sendAlert("copied object to clipboard");
  }else if (numObject > 1){
    sendAlert("copied objects to clipboard");
  }
  modlog("clipboard", "copied objects to clipboard");
  setClipboardFromSelected(state.editor);
}

void drawText(std::string word, float left, float top, unsigned int fontSize, bool permatext, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<std::string> fontFamily, std::optional<objid> selectionId){
  //std::cout << "draw text: " << word << ": perma? " << permatext << std::endl;
  addShapeData(lineData, ShapeData{
    .shapeData = TextShapeData {
      .word = word,
      .fontFamily = fontFamily,
      .fontSize = fontSize,
      .left = left,
      .top = top,
    },
    .textureId = textureId,
    .perma = permatext,
    .ndi = ndi,
    .tint = tint.has_value() ? tint.value() : glm::vec4(1.f, 1.f, 1.f, 1.f),
    .selectionId = selectionId,
  });
}

void drawText(std::string word, float left, float top, unsigned int fontSize){
  drawText(word, left, top, fontSize, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, false, std::nullopt, std::nullopt);  
}
void drawTextNdi(std::string word, float left, float top, unsigned int fontSize){
  drawText(word, left, top, fontSize, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt);  
}


// This whole function is coupled horribly to other parts of code
void getTextDimensionsNdi(std::string word, float fontSizeNdi, bool ndi, std::optional<std::string> fontFamily, float* _width, float* _height){
  modassert(ndi, "only works with ndi for now");
  unsigned int convertedFontSize = fontSizeNdi * 1000.f * 0.5f;
  glm::vec3 offset(0.f, 0.f, 0.f);
  auto boundInfo = boundInfoForCenteredText(fontFamilyByName(fontFamily), word, 0.f, 0.f, convertedFontSize, POSITIVE_ALIGN, TextWrap { .type = WRAP_NONE, .wrapamount = 0.f }, TextVirtualization { .maxheight = -1, .offsetx = 0, .offsety = 0 }, -1, false, 0, &offset);
  *_width = boundInfo.xMax - boundInfo.xMin;
  *_height = boundInfo.yMax - boundInfo.yMin;
}

FontFamily& fontFamilyByName(std::optional<std::string> name){
  auto actualName = name.has_value() ? name.value() : "";
  for (auto &family : defaultResources.fontFamily){
    //std::cout << "Comparing " << name << " to " << family.name << std::endl;
    if (family.name == actualName){
      return family;
    }
  }
  //modassert(false, "ERROR invalid font family name: " + (name.size() == 0 ? "<empty>" : ("[" + name + "]")));
  return defaultResources.fontFamily.at(0);
}
int drawWord(GLint shaderProgram, objid id, std::string word, unsigned int fontSize, AlignType align, TextWrap wrap, TextVirtualization virtualization, UiTextCursor cursor, std::string fontFamilyName, bool drawBoundingOnly){
  return drawWordsRelative(shaderProgram, fontFamilyByName(fontFamilyName), fullModelTransform(world.sandbox, id), word, 0, 0, fontSize, align, wrap, virtualization, cursor.cursorIndex, cursor.cursorIndexLeft, cursor.highlightLength, drawBoundingOnly);
}

void drawRect(float centerX, float centerY, float width, float height, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture){
  //std::cout << "draw text: " << word << ": perma? " << permatext << std::endl;
  addShapeData(lineData, ShapeData{
    .shapeData = RectShapeData {
      .centerX = centerX,
      .centerY = centerY,
      .width = width,
      .height = height,
      .texture = texture,
    },
    .textureId = textureId,
    .perma = perma,
    .ndi = ndi,
    .tint = tint.has_value() ? tint.value() : glm::vec4(1.f, 1.f, 1.f, 1.f),
    .selectionId = selectionId,
  });
}

void drawLine2D(glm::vec3 fromPos, glm::vec3 toPos, bool perma, std::optional<glm::vec4> tint, std::optional<unsigned int> textureId, bool ndi, std::optional<objid> selectionId, std::optional<std::string> texture){
  addShapeData(lineData, ShapeData{
    .shapeData = LineShapeData {
      .fromPos = fromPos,
      .toPos = toPos,
    },
    .textureId = textureId,
    .perma = perma,
    .ndi = ndi,
    .tint = tint.has_value() ? tint.value() : glm::vec4(1.f, 1.f, 1.f, 1.f),
    .selectionId = selectionId,
  });
}

std::vector<std::string> listAnimations(int32_t id){
  std::vector<std::string> animationNames;
  auto groupId = getGroupId(world.sandbox, id);
  if (world.animations.find(groupId) == world.animations.end()){
    return animationNames;
  }
  auto animations = world.animations.at(groupId);
  for (auto animation : animations){
    animationNames.push_back(animation.name);
  }
  return animationNames;
}

void playAnimation(int32_t id, std::string animationToPlay, AnimationType animationType){
  //if (!idExists(world.sandbox, id)){
  //  return;
  //}
  modassert(idExists(world.sandbox, id), std::string("play animation, id does not exist: " + std::to_string(id)));
  modlog("animation", std::string("play animation: ") + animationToPlay + ", for id = " + std::to_string(id));
  addAnimation(world, timings, id, animationToPlay, timeSeconds(false), animationType);
}

void stopAnimation(int32_t id){
  modlog("animation", std::string("stop animation: for id = ") + std::to_string(id));
  removeAnimation(world, timings, id);
}


bool objIdInVector(std::vector<objid> ids, objid id){
  for (auto compareId : ids){
    if (compareId == id){
      return true;
    }
  }
  return false;
}

std::vector<objid> idsInGroupById(objid id){
  modassert(gameobjExists(id), "gameobj does not exist");
  // filter ids pending to be removed
  auto allIds = getIdsInGroupByObjId(world.sandbox, id);
  std::vector<objid> idsNotRemoved;
  for (auto id : allIds){
    if (!objIdInVector(objectsQueuedForRemoval, id) && !objIdInVector(groupsQueuedForRemoval, id)){
      idsNotRemoved.push_back(id);
    }
  }
  return idsNotRemoved;
}

objid groupId(objid id){
  modassert(gameobjExists(id), "gameobj does not exist");
  auto groupId = getGroupId(world.sandbox, id); 
  return groupId;
}

void removeObjectById(objid id){
  objectsQueuedForRemoval.push_back(id);
}
void removeByGroupId(int32_t idInGroup){
  groupsQueuedForRemoval.push_back(idInGroup);
}
void doRemoveQueuedRemovals(){
  for (auto id : objectsQueuedForRemoval){
    removeObjectFromScene(world, id);
  }
  objectsQueuedForRemoval = {};

  for (auto groupId : groupsQueuedForRemoval){
    removeGroupFromScene(world, groupId);
  }
  groupsQueuedForRemoval = {};
}

std::optional<objid> prefabId(objid id){
  return prefabId(world, id);
}

std::vector<std::string> listModels(){
  return listFilesWithExtensions("./res/models", { "obj", "dae", "gltf" });
}
std::vector<std::string> listTextures(){
  return listFilesWithExtensions("./res/textures", { "png", "jpg" });
}
std::vector<std::string> listSoundFiles(){
  return { listFilesWithExtensions("./res/sounds", { "wav" }) };
}
std::vector<std::string> listHeightmaps(){
  return { listFilesWithExtensions("./res/heightmaps", { "png", "jpg" }) };
}
std::vector<std::string> listHeightmapBrushes(){
  return { listFilesWithExtensions("./res/brush", { "png", "jpg" }) };
}
std::vector<std::string> listSceneFiles(){
  return { listFilesWithExtensions("./res/scenes", { "rawscene" }) };
}

std::vector<std::string> listResources(std::string resourceType){
  if (resourceType == "sounds"){
    return listSoundFiles();
  }else if (resourceType == "textures"){
    return listTextures();
  }else if (resourceType == "models"){
    return listModels();
  }else if (resourceType == "heightmaps"){
    return listHeightmaps();
  }else if (resourceType == "heightmap-brushes"){
    return listHeightmapBrushes();
  }else if (resourceType == "scenefiles"){
    return listSceneFiles();
  }
  modassert(false, "invalid resource type: " + resourceType);
  return {};
}

void sendNotifyMessage(std::string message, std::any value){
  channelMessages.push(StringAttribute {
    .strTopic = message,
    .strValue = value,
  });
}

void sendAlert(std::string message){
  sendNotifyMessage("alert", message);
}

double timeSeconds(bool realtime){
  if (realtime){
    return statistics.now;
  }
  return timePlayback.currentTime;
}

double timeElapsed(){
  return statistics.deltaTime;
}

struct ActiveRecording {
  objid targetObj;
  Recording recording;
};

std::map<objid, ActiveRecording> activeRecordings;
objid createRecording(objid id){  
  auto recordingId = getUniqueObjId();
  assert(activeRecordings.find(recordingId) == activeRecordings.end());
  activeRecordings[recordingId] = ActiveRecording{
    .targetObj = id,
    .recording = createRecording(),
  };
  return recordingId;
}
void saveRecording(objid recordingId, std::string filepath){
  auto recording = activeRecordings.at(recordingId);
  std::cout << "SAVING RECORDING STARTED - " << filepath << std::endl;
  saveRecording(filepath, recording.recording, serializePropertySuffix);
  activeRecordings.erase(recordingId);
  std::cout << "SAVING RECORDING COMPLETE - " << filepath << std::endl;
}

struct PlayingRecording {
  float startTime;
  RecordingPlaybackType type;
  Recording recording;
};
std::map<objid, PlayingRecording> playingRecordings;
void playRecording(objid id, std::string recordingPath, std::optional<RecordingPlaybackType> type){
  stopRecording(id);
  playingRecordings[id] = PlayingRecording {
    .startTime = getTotalTime(),
    .type = type.has_value() ? type.value() : RECORDING_PLAY_ONCE,
    .recording = loadRecording(recordingPath, parsePropertySuffix, interface.readFile),
  };
}
void stopRecording(objid id){
  if (playingRecordings.find(id) != playingRecordings.end()){
    playingRecordings.erase(id);
  }
}

void tickRecordings(float time){
  for (auto &[id, activeRecording] : activeRecordings){
    auto localTransform = gameobjectTransformation(world, activeRecording.targetObj, false);
    saveRecordingIndex(activeRecording.recording, "position", localTransform.position, time);
  } 

  std::vector<objid> recordingsToRemove;
  for (auto &[id, recording] : playingRecordings){
    bool isComplete = false;
    auto interpolatedProperties = recordingPropertiesInterpolated(recording.recording, time, interpolateAttribute, recording.startTime, recording.type, &isComplete);
    if (isComplete){
      modassert(recording.type != RECORDING_PLAY_LOOP, "recording playback - got complete loop type");
      recordingsToRemove.push_back(id);
    }
    setProperty(world, id, interpolatedProperties);
  }
  for (auto id : recordingsToRemove){
    stopRecording(id);
  }
}


std::vector<HitObject> raycastW(glm::vec3 pos, glm::quat direction, float maxDistance){
  return raycast(world, pos, direction, maxDistance);
}

std::vector<HitObject> contactTest(objid id){
  return contactTest(world, id);
}

std::vector<HitObject> contactTestShape(glm::vec3 pos, glm::quat orientation, glm::vec3 scale){
  return contactTestShape(world.physicsEnvironment, world.rigidbodys, pos, orientation, scale);
}

glm::vec3 moveRelative(glm::vec3 posFrom, glm::quat orientation, float distance){
  return moveRelative(posFrom, orientation, glm::vec3(0.f, 0.f, -1 * distance), false);
}
glm::vec3 moveRelative(glm::vec3 posFrom, glm::quat orientation, glm::vec3 vec){
  return moveRelative(posFrom, orientation, vec, false);
}

void nextTexture(){
  nextTexture(drawParams, world.textures.size());
}
void previousTexture(){
  previousTexture(drawParams);
}

void setNavmeshTexture(unsigned int textureId){
  state.navmeshTextureId = textureId;
}
void setTexture(objid index, std::string textureName){
  setTexture(world, index, textureName, setNavmeshTexture);
}
void maybeChangeTexture(int index){
  auto textureName = worldTextures(world).at(drawParams.activeTextureIndex).textureName;
  setTexture(world, index, textureName, setNavmeshTexture);
}

void setState(std::string stateName){
  if (stateName == "paint_on"){
    state.shouldPaint = true;
  }else if (stateName == "paint_off"){
    state.shouldPaint = false;
  }else if (stateName == "translate"){
    state.manipulatorMode = TRANSLATE;
  }else if (stateName == "scale"){
    state.manipulatorMode = SCALE;
  }else if (stateName == "rotate"){
    state.manipulatorMode = ROTATE;  
  }else if (stateName == "next_texture"){
    nextTexture();
  }else if (stateName == "prev_texture"){
    previousTexture();
  }
}

void  setCrosshairSprite(){
  if (state.crosshair == ""){
    defaultResources.defaultMeshes.crosshairSprite = NULL;
    return;
  }
  defaultResources.defaultMeshes.crosshairSprite = &world.meshes.at(state.crosshair).mesh;
}

void windowPositionCallback(GLFWwindow* window, int xpos, int ypos){
  if (!state.fullscreen){
    state.savedWindowsize.x = xpos;
    state.savedWindowsize.y = ypos;
  }
}
void windowSizeCallback(GLFWwindow* window, int width, int height){
  if (!state.fullscreen){
    state.savedWindowsize.z = width; 
    state.savedWindowsize.w = height;
  }
}
void toggleFullScreen(bool fullscreen){
  if (fullscreen){
    glfwSetWindowMonitor(window, monitor, 0, 0, mode -> width, mode->height, 0);
  }else{
    glfwSetWindowMonitor(window, NULL, state.savedWindowsize.x, state.savedWindowsize.y, state.savedWindowsize.z, state.savedWindowsize.w, 0);
  }
}

void setCulling(bool cullEnabled){
  if (cullEnabled){
    glEnable(GL_CULL_FACE);  
  }else{
    glDisable(GL_CULL_FACE);  
  }
  std::cout << "culling enabled: " << state.cullEnabled << std::endl;
  sendAlert(std::string("culling toggled: ") + (cullEnabled ? "enabled" : "disabled"));
}
void setWorldState(std::vector<ObjectValue> values){
  std::vector<ObjectValue> renderStagesValues;
  std::vector<ObjectValue> otherValues;
  for (auto &value: values){
    bool isRenderValue = value.object.at(0) == '$';
    if (isRenderValue){
      auto translatedObject = value.object.substr(1, value.object.size());
      value.object = translatedObject;
      renderStagesValues.push_back(value);
    }else{
      otherValues.push_back(value);
    }
  }

  for (auto &renderStagesValue : renderStagesValues){
    setRenderStageState(renderStages, renderStagesValue);
  }
  
  auto oldFullScreen = state.fullscreen;
  auto oldCullEnabled = state.cullEnabled;
  setState(state, otherValues, statistics.now);
  if (oldFullScreen != state.fullscreen){
    toggleFullScreen(state.fullscreen);
  }
  if (oldCullEnabled != state.cullEnabled){
    setCulling(state.cullEnabled);
  }
  //// todo add updates for each state ... should check if it was changed in state maybe?
  setCrosshairSprite();
}

std::vector<ObjectValue> getWorldState(){
  return getState(state);
}

void setLayerState(std::vector<StrValues> values){
  setLayerOptions(world.sandbox.layers, values);
}

void setFloatState(std::string stateName, float value){
  if (stateName == "opacity"){
    drawParams.opacity = value;
  }
  else if (stateName == "drawsize"){
    drawParams.scale = glm::vec3(1.f, 1.f, 1.f) * value;
  }
  else if (stateName == "drawcolor-r"){
    drawParams.tint.x = value;
  }
  else if (stateName == "drawcolor-g"){
    drawParams.tint.y = value;
  }
  else if (stateName == "drawcolor-b"){
    drawParams.tint.z = value;
  }
}

void setIntState(std::string stateName, int value){
  if (stateName == "set_texture"){
    std::cout << "value is: " << value << std::endl;
    maybeChangeTexture(value);
  }  
}

objid listSceneId(int32_t id){
  return sceneId(world.sandbox, id);
}

Transformation getCameraTransform(){
  if (state.useDefaultCamera || state.activeCameraObj == NULL){
    return defaultResources.defaultCamera.transformation;
  }
  if (state.cameraInterp.shouldInterpolate){
    auto lerpAmount = (statistics.now - state.cameraInterp.startingTime) / state.cameraInterp.length;
    if (lerpAmount >= 1){
      state.cameraInterp.shouldInterpolate = false;
      state.activeCameraObj = &getGameObject(world, state.cameraInterp.targetCam);
      state.activeCameraData = &getCamera(world, state.activeCameraObj -> id);
      cBindings.onCameraSystemChange(state.activeCameraObj -> name, state.useDefaultCamera);
    }
    auto oldCameraPosition = gameobjectTransformation(world, state.activeCameraObj -> id, true);
    auto newCameraPosition = gameobjectTransformation(world, state.cameraInterp.targetCam, true);
    return interpolate(oldCameraPosition, newCameraPosition, lerpAmount, lerpAmount, lerpAmount);
  }
  return gameobjectTransformation(world, state.activeCameraObj -> id, true);
}
void maybeResetCamera(int32_t id){
  if (state.activeCameraObj != NULL &&  id == state.activeCameraObj -> id){
    state.activeCameraObj = NULL;
    state.activeCameraData = NULL;
    std::cout << "active camera reset" << std::endl;
  }
  if (state.cameraInterp.targetCam == id){
    state.cameraInterp.shouldInterpolate = false;
  }
}
void setActiveCamera(int32_t cameraId, float interpolationTime){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
  if (! (std::find(cameraIndexs.begin(), cameraIndexs.end(), cameraId) != cameraIndexs.end())){
    std::cout << "index: " << cameraId << " is not a valid index" << std::endl;
    modassert(false, "invalid camera");
    return;
  }


  if (interpolationTime > 0){
    state.cameraInterp = CamInterpolation {
      .shouldInterpolate = true,
      .startingTime = statistics.now,
      .length = interpolationTime,
      .targetCam = cameraId,
    };
    return;
  }

  state.useDefaultCamera = false;
  state.activeCameraObj = &getGameObject(world, cameraId);
  state.activeCameraData = &getCamera(world, cameraId);
  setSelectedIndex(state.editor, cameraId, true);
  cBindings.onCameraSystemChange(state.activeCameraObj -> name, state.useDefaultCamera);
  std::cout << "set active camera to id: " << cameraId << std::endl;
  std::cout << "camera data: " << state.activeCameraData -> enableDof << ", " << state.activeCameraData -> minBlurDistance << ", " << state.activeCameraData -> maxBlurDistance << std::endl;
}
void setActiveCamera(std::string name, objid sceneId){
  auto object = getGameObjectByName(name, sceneId, false);
  if (!object.has_value()){
    std::cout << "ERROR SETTING CAMERA: does the camera: " << name << " exist?" << std::endl;
    assert(false);
  }
  setActiveCamera(object.value(), -1);
}

Transformation getView(){
  return viewTransform;
}

void nextCamera(){
  auto cameraIndexs = getGameObjectsIndex<GameObjectCamera>(world.objectMapping);
  if (cameraIndexs.size() == 0){  // if we do not have a camera in the scene, we use default
    state.activeCameraObj = NULL;
    state.activeCameraData = NULL;
    return;
  }

  state.activeCamera = (state.activeCamera + 1) % cameraIndexs.size();
  int32_t activeCameraId = cameraIndexs.at(state.activeCamera);
  setActiveCamera(activeCameraId, -1);
}
void moveCamera(glm::vec3 offset, std::optional<bool> relative){
  bool isRelative = !relative.has_value() || relative.value() == true;
  if (isRelative){
    if (state.activeCameraObj == NULL){
      defaultResources.defaultCamera.transformation.position = moveRelative(defaultResources.defaultCamera.transformation.position, defaultResources.defaultCamera.transformation.rotation, glm::vec3(offset), false);
    }else{
      auto cameraLocalTransform = gameobjectTransformation(world, state.activeCameraObj -> id, false);
      setGameObjectPosition(state.activeCameraObj ->id, moveRelative(cameraLocalTransform.position, cameraLocalTransform.rotation, glm::vec3(offset), false), true);
    }
  }else{
    if (state.activeCameraObj == NULL){
      defaultResources.defaultCamera.transformation.position = offset;
    }else{
      setGameObjectPosition(state.activeCameraObj ->id, offset, true);
    }
  }
}
void moveCamera(glm::vec3 offset){
  moveCamera(offset, std::nullopt);
}
void rotateCamera(float xoffset, float yoffset){
  if (state.activeCameraObj == NULL){
    defaultResources.defaultCamera.transformation.rotation = setFrontDelta(defaultResources.defaultCamera.transformation.rotation, xoffset, yoffset, 0, 0.1);
  }else{
    auto cameraRelativeRotation = gameobjectRotation(world, state.activeCameraObj -> id, false);
    setGameObjectRotation(state.activeCameraObj ->id, setFrontDelta(cameraRelativeRotation, xoffset, yoffset, 0, 0.1), true);
  }
}
void setCameraRotation(glm::quat orientation){
  if (state.activeCameraObj == NULL){
    defaultResources.defaultCamera.transformation.rotation = orientation;
  }else{
    setGameObjectRotation(state.activeCameraObj ->id, orientation, true);
  }
}


void playSoundState(objid id, std::optional<float> volume, std::optional<glm::vec3> position){
  playSoundState(world.objectMapping, id, volume, position); 
}

void playSoundState(std::string source, objid sceneId, std::optional<float> volume, std::optional<glm::vec3> position){
  std::cout << "Info: play sound: " << source << std::endl;
  auto gameobj = getGameObjectByName(source, sceneId, false);
  if (gameobj.has_value()){
    playSoundState(world.objectMapping, gameobj.value(), volume, position); 
  }else{
    std::cout << "ERROR: no source named: " << source << " in scene: " << sceneId << std::endl;
    assert(false);
  }
}

void stopSoundState(std::string source, objid sceneId){
  std::cout << "Info: stop sound: " << source << std::endl;
  auto gameobj = getGameObjectByName(source, sceneId, false);
  if (gameobj.has_value()){
    stopSoundState(world.objectMapping, gameobj.value()); 
  }else{
    std::cout << "ERROR: no source named: " << source << " in scene: " << sceneId << std::endl;
    //assert(false);
  }
}

unsigned int activeTextureId(){
  return world.textures.at(activeTextureName(drawParams, world.textures)).texture.textureId;
}

void emit(objid id, std::optional<glm::vec3> initPosition, std::optional<glm::quat> initOrientation, std::optional<glm::vec3> initVelocity, std::optional<glm::vec3> initAvelocity, std::optional<objid> parentId){
  emit(world, id, NewParticleOptions {
    .position = initPosition,
    .orientation = initOrientation,
    .velocity = initVelocity,
    .angularVelocity = initAvelocity, 
    .parentId = parentId,
  });
}

objid addLoadingAround(objid id){
  return addLoadingAround(dynamicLoading, id);
}
void removeLoadingAround(objid id){
  removeLoadingAround(dynamicLoading, id);
}

void makeParent(objid child, objid parent){
  makeParent(world.sandbox, child, parent);
}

void createGeneratedMesh(std::vector<glm::vec3> face, std::vector<glm::vec3> points, std::string destMesh){
  createGeneratedMesh(world, face, points, destMesh);
}

objid addLineNextCycle(glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, std::optional<glm::vec4> color, std::optional<unsigned int> textureId, std::optional<unsigned int> linewidth){
  return addLineToNextCycleTint(lineData, fromPos, toPos, permaline, owner, color, textureId, linewidth);
}

objid addLineNextCyclePhysicsDebug(glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner){
  return addLineToNextCycle(lineData, fromPos, toPos, permaline, owner, GREEN, std::nullopt);
}
void drawLine(glm::vec3 point1, glm::vec3 point2){
  glm::vec3 offset(-20.f, -27.f, 70.f);
  addLineNextCyclePhysicsDebug(point1 + offset, point2 + offset,  false, 0);
}
glm::vec3 navPosition(objid id, glm::vec3 target){
  return aiNavigate(world, id, target, drawLine).value();
}

bool lock(std::string key, objid owner){
  //std::cout << "lock: (" << key << ", " << owner << ")" << std::endl;
  auto canLock = activeLocks.find(key) == activeLocks.end();
  if (!canLock){
    return false;
  }
  activeLocks[key] = owner;
  //std::cout << "lock: " << key << std::endl;
  return true;
}
bool unlock(std::string key, objid owner){
  //std::cout << "unlock: (" << key << ", " << owner << ")" << std::endl;
  auto lockExists = activeLocks.find(key) != activeLocks.end();
  auto ownerOwnsKey = lockExists && activeLocks.at(key) == owner;
  if (!lockExists){
    return true;
  }  
  if (ownerOwnsKey){
    activeLocks.erase(key);
    //std::cout << "unlock: " << key << std::endl;
    return true;
  }
  std::cout << "ERROR: tried to unlock a key that did not own (" << key << "," << owner << ")" << std::endl;
  return false;
}

void removeLocks(objid owner){
  std::vector<std::string> locksOwnedByOwner;
  for (auto &[lockname, id] : activeLocks){
    if (id == owner){
      locksOwnedByOwner.push_back(lockname);
    }
  }
  for (auto lockname : locksOwnedByOwner){
    activeLocks.erase(lockname);
  }
}

void takeScreenshot(std::string filepath){
  state.takeScreenshot = true;
  state.screenshotPath = filepath;
}


std::vector<UserTexture> userTextures;
std::vector<UserTexture> textureIdsToRender(){
  return userTextures;
}

UserTexture* userTextureById(unsigned int id){
  for (auto &userTexture : userTextures){
    if (userTexture.id == id){
      return &userTexture;
    }
  }
  assert(false);
  return NULL;
}

unsigned int createTexture(std::string name, unsigned int width, unsigned int height, objid ownerId){
  MODTODO("create texture -> use ownership id of the script being used");
  std::cout << "create texture: " << name << std::endl;
  auto selectionTextureId = loadTextureWorldEmpty(world, name +"_selection_texture", ownerId, width, height, std::nullopt).textureId;
  auto textureID = loadTextureWorldEmpty(world, name, ownerId, width, height, selectionTextureId).textureId;
  userTextures.push_back(UserTexture{
    .id = textureID,
    .ownerId = ownerId,
    .selectionTextureId = selectionTextureId,
    .autoclear = false,
    .shouldClear = true,
    .clearTextureId = std::nullopt, //world.textures.at("./res/textures/wood.jpg").texture.textureId,//,
    .clearColor = glm::vec4(0.f, 0.f, 0.f, 1.f),
  });

  modlog("create-texture", "number of textures: " + std::to_string(userTextures.size()));
  return textureID;
}

void freeTexture(std::string name, objid ownerId){
  auto textureId = world.textures.at(name).texture.textureId;
  int selectionTextureId = -1;

  std::vector<UserTexture> remainingTextures;
  for (auto userTexture : userTextures){
    if (userTexture.id != textureId){
      remainingTextures.push_back(userTexture);
    }else{
      selectionTextureId = userTexture.selectionTextureId;
    }
  }
  userTextures = remainingTextures;
  freeTextureRefsIdByOwner(world, ownerId, textureId);
  modassert(selectionTextureId > 0, "selectionTextureId not found");
  freeTextureRefsIdByOwner(world, ownerId, selectionTextureId);
}
void freeTexture(objid ownerId){
  std::vector<UserTexture> remainingTextures;
  for (auto userTexture : userTextures){
    if (userTexture.ownerId != ownerId){
      remainingTextures.push_back(userTexture);
    }
  }
  userTextures = remainingTextures;
  freeTextureRefsByOwner(world, ownerId);
}

// clear texture, should automatically load texture
void clearTexture(unsigned int textureId, std::optional<bool> autoclear, std::optional<glm::vec4> color, std::optional<std::string> texture){
  modassert(!(autoclear.has_value() && !autoclear.value()), "autoclear set to false, which means always don't clear, probably meant std::nullopt to not set autoclear to mean neverclear");
  UserTexture& userTex = *userTextureById(textureId);
  std::optional<unsigned int> clearTextureId = std::nullopt;
  if (texture.has_value()){
    std::cout << "texture: " << texture.value() << std::endl;
    clearTextureId = world.textures.at(texture.value()).texture.textureId;
    userTex.clearColor = glm::vec4(1.f, 1.f, 1.f, 1.f);
  }
  userTex.clearTextureId = clearTextureId;
  
  if (color.has_value()){
    userTex.clearColor = color.value();
  }
  if (!autoclear.has_value()){
    userTex.shouldClear = true;
    return;
  }
  userTex.autoclear = autoclear.value();
}

void markUserTexturesCleared(){
  for (auto &userTexture : userTextures){
    userTexture.shouldClear = false;
  }
}

extern std::map<std::string, std::string> args;
std::map<std::string, std::string> getArgs(){
  return args;
}

std::vector<std::vector<std::string>> executeSqlQuery(sql::SqlQuery& query, bool* valid){
  auto args = getArgs();

  std::string error = "";
  auto result = sql::executeSqlQuery(query, sqlDirectory, valid, &error);
  if (!*valid){
    modlog("sql", std::string("invalid query execution: " + error), MODLOG_ERROR);
  }
  return result;
}


//////////////////////////
struct ScheduledTask {
  objid ownerId;
  std::function<void(void*)> fn;
  float time;  // delay time + now 
  void* data;
};

std::vector<ScheduledTask> scheduledTasks;
std::vector<ScheduledTask> tasksToSchedule; // taks to schedule is sepearate since want enqueue only in the tick, since task.fn can modify 
void schedule(objid id, float delayTimeMs, void* data, std::function<void(void*)> fn) {
  tasksToSchedule.push_back(ScheduledTask { 
    .ownerId = id,
    .fn = fn,
    .time = statistics.now * 1000 + delayTimeMs,
    .data = data,
  });
}

void removeScheduledTask(std::set<objid> ids){
  std::vector<ScheduledTask> newScheduledTasks;
  for (int i = 0; i < scheduledTasks.size(); i++){
    if (ids.find(i) == ids.end()){
      newScheduledTasks.push_back(scheduledTasks.at(i));
    }
  }
  scheduledTasks = newScheduledTasks;
}
void removeScheduledTaskByOwner(std::set<objid> ids){
  std::vector<ScheduledTask> newScheduledTasks;
  for (int i = 0; i < scheduledTasks.size(); i++){
    if (ids.find(scheduledTasks.at(i).ownerId) == ids.end()){
      newScheduledTasks.push_back(scheduledTasks.at(i));
    }
  }
  scheduledTasks = newScheduledTasks; 
}

void tickScheduledTasks(){
  for (auto &task : tasksToSchedule){
    scheduledTasks.push_back(task);
  }
  tasksToSchedule = {};

  std::set<objid> idsToRemove;
  float currTime = statistics.now * 1000;
  //std::cout << "num tasks: " << scheduledTasks.size() << std::endl;
  for (int i = 0; i < scheduledTasks.size(); i++){
    ScheduledTask& task = scheduledTasks.at(i);
    //std::cout << "task time: " << task.time << ", currTime = " << currTime << std::endl;
    auto shouldExecuteTask = currTime > task.time;
    if (!shouldExecuteTask){
      continue;
    }
    modlog("SCHEDULER", "executing scheduled task, owner = " + std::to_string(task.ownerId));
    task.fn(task.data); // if this wasn't copied, this could screw up the loop
    idsToRemove.insert(i);  
  }
  removeScheduledTask(idsToRemove);
  //modlog("scheduled tasks", "num tasks: " + std::to_string(scheduledTasks.size()));
}

void handleChangedResourceFiles(std::set<std::string> changedFiles){
  for (auto &file : changedFiles){
    if (getFileType(file) == IMAGE_EXTENSION){
      maybeReloadTextureWorld(world, file);
    }
  }
}

void setLogEndpoint(std::optional<std::function<void(std::string&)>> fn){
  modlogSetLogEndpoint(fn);
  modlog("logging", std::string("custom logEndpoint set: ") + std::string(fn.has_value() ? "true" : "false"));
}

const char* getClipboardString(){
  return glfwGetClipboardString(window); 
  return NULL;
}

void setClipboardString(const char* string){
  // this is really slow for some reason 
  glfwSetClipboardString(window, string);
}

void sendManipulatorEvent(MANIPULATOR_EVENT event){
  onManipulatorEvent(state.manipulatorState, tools, event);
}