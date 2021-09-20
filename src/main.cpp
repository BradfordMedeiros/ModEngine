#include <csignal>
#include <cxxopts.hpp>

#include "./main_input.h"
#include "./scene/scene.h"
#include "./scene/common/vectorgfx.h"
#include "./scene/animation/timeplayback.h"
#include "./scene/animation/animation.h"
#include "./scene/animation/playback.h"
#include "./scene/animation/recorder.h"
#include "./scheme/scriptmanager.h"
#include "./shaders.h"
#include "./translations.h"
#include "./colorselection.h"
#include "./state.h"
#include "./network/network.h"
#include "./network/servers.h"
#include "./network/activemanager.h"
#include "./worldloader.h"
#include "./keymapper.h"
#include "./layers.h"
#include "./drawing.h"
#include "./easyuse/editor.h"
#include "./easyuse/manipulator.h"
#include "./common/profiling.h"
#include "./benchmark.h"
#include "./extensions.h"
#include "./netscene.h"
#include "./worldtiming.h"

unsigned int framebufferProgram;
unsigned int drawingProgram;
unsigned int blurProgram;
unsigned int quadVAO;

GameObject defaultCamera = GameObject {
  .id = -1,
  .name = "defaultCamera",
  .transformation = Transformation {
    .position = glm::vec3(0.f, -2.f, 0.f),
    .scale = glm::vec3(1.0f, 1.0f, 1.0f),
    .rotation = glm::quat(1.0, 0, 0, 0.0f),
  }
};

bool showDebugInfo = false;
std::string shaderFolderPath;

bool disableInput = false;
int numChunkingGridCells = 0;
bool useChunkingSystem = false;
std::string rawSceneFile;
bool bootStrapperMode = false;
NetCode netcode { };

engineState state = getDefaultState(1920, 1080);
World world;
DefaultMeshes defaultMeshes;

SysInterface interface;
std::string textureFolderPath;
float now = 0;
float deltaTime = 0.0f; // Time between current frame and last frame
int numTriangles = 0;   // # drawn triangles (eg drawelements(x) -> missing certain calls like eg text)

DynamicLoading dynamicLoading;
std::vector<Line> lines;
std::vector<Line> bluelines;
std::vector<Line> permaLines;

std::map<unsigned int, Mesh> fontMeshes;

glm::mat4 view;
unsigned int framebufferTexture;
unsigned int framebufferTexture2;
unsigned int framebufferTexture3;
unsigned int fbo;
unsigned int depthTextures[32];

const int numPortalTextures = 16;
unsigned int portalTextures[16];
std::map<objid, unsigned int> portalIdCache;

glm::mat4 orthoProj;
unsigned int uiShaderProgram;

SchemeBindingCallbacks schemeBindings;
std::queue<StringString> channelMessages;
KeyRemapper keyMapper;
extern std::vector<InputDispatch> inputFns;

std::map<std::string, objid> activeLocks;

std::vector<LayerInfo> layers;
  

// 0th depth texture is the main depth texture used for eg z buffer
// other buffers are for the lights
const int numDepthTextures = 32;
int activeDepthTexture = 0;

DrawingParams drawParams = getDefaultDrawingParams();
Benchmark benchmark = createBenchmark(false);

void updateDepthTexturesSize(){
  for (int i = 0; i < numDepthTextures; i++){
    glBindTexture(GL_TEXTURE_2D, depthTextures[i]);
    // GL_DEPTH_COMPONENT32F
    glTexImage2D(GL_TEXTURE_2D, 0,  GL_DEPTH_STENCIL, state.currentScreenWidth, state.currentScreenHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }
}
void generateDepthTextures(){
  glGenTextures(numDepthTextures, depthTextures);
  for (int i = 0; i < numDepthTextures; i++){
    glBindTexture(GL_TEXTURE_2D, depthTextures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    updateDepthTexturesSize();
  }
}
void setActiveDepthTexture(int index){
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);  
  unsigned int texture = depthTextures[index];
  glBindTexture(GL_TEXTURE_2D, texture);
  // GL_DEPTH_ATTACHMENT
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
}

void updatePortalTexturesSize(){
  for (int i = 0; i < numPortalTextures; i++){
    glBindTexture(GL_TEXTURE_2D, portalTextures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state.currentScreenWidth, state.currentScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);   
  }
}
void generatePortalTextures(){
  glGenTextures(numPortalTextures, portalTextures);
  for (int i = 0; i < numPortalTextures; i++){
    glBindTexture(GL_TEXTURE_2D, portalTextures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    updatePortalTexturesSize();
  }
}

float initialTime = glfwGetTime();
WorldTiming timings = createWorldTiming(initialTime);

TimePlayback timePlayback(
  initialTime, 
  [](float currentTime, float elapsedTime) -> void {
    tickAnimations(timings, elapsedTime);
  }, 
  []() -> void {}
); 

float getTotalTime(){
  return now - initialTime;
}

bool useYAxis = true;
void onDebugKey(){
  useYAxis = !useYAxis;
  if (timePlayback.isPaused()){
    timePlayback.play();
  }else{
    timePlayback.pause();
  }
}

unsigned int textureToPaint = -1;
bool canPaint = false;

void applyPainting(objid id){
  auto texture = textureForId(world, id);
  if (texture.has_value()){
    textureToPaint = texture.value().textureId;
    canPaint = true;
  }
  //std::cout << "texture id is: " << texture.textureId << std::endl;
}

void handlePainting(UVCoord uvsToPaint){
  if (!canPaint || !state.shouldPaint){
    return;
  }

  glUseProgram(drawingProgram); 

  glBindTexture(GL_TEXTURE_2D, textureToPaint);
  int w, h;
  int miplevel = 0;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_WIDTH, &w);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, miplevel, GL_TEXTURE_HEIGHT, &h);

  glViewport(0, 0, w, h);

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureToPaint, 0);

  glUniformMatrix4fv(glGetUniformLocation(drawingProgram, "model"), 1, GL_FALSE, glm::value_ptr(
    glm::scale(
      glm::translate(glm::mat4(1.0f), uvToNDC(uvsToPaint)), 
      glm::vec3(0.01f, 0.01f, 0.01f) * drawParams.scale)
    )
  );
  glUniform1f(glGetUniformLocation(drawingProgram, "opacity"), drawParams.opacity);
  glUniform3fv(glGetUniformLocation(drawingProgram, "tint"), 1, glm::value_ptr(drawParams.tint));

  glBindTexture(GL_TEXTURE_2D, activeTextureId());
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glViewport(0, 0, state.currentScreenWidth, state.currentScreenHeight);
}
void handleTerrainPainting(UVCoord uvCoord){
  if (state.shouldTerrainPaint && state.mouseIsDown){
    applyHeightmapMasking(world, selected(state.editor), state.terrainPaintDown ? -10.f : 10.f, uvCoord.x, uvCoord.y, true);
  }
}

bool selectItemCalled = false;
bool shouldCallItemSelected = false;
void selectItem(objid selectedId, Color pixelColor){
  std::cout << "SELECT ITEM CALLED!" << std::endl;
  if (!showDebugInfo){
    return;
  }

  applyPainting(selectedId);

  auto idToUse = state.groupSelection ? getGroupId(world.sandbox, selectedId) : selectedId;

  auto selectedSubObj = getGameObject(world, selectedId);
  auto selectedObject =  getGameObject(world, idToUse);
  applyFocusUI(world.objectMapping, selectedId, sendNotifyMessage);

  shouldCallItemSelected = true;
  onManipulatorSelectItem(
    idToUse, 
    selectedSubObj.name,
    []() -> objid {
      return makeObjectAttr(
        0,
        "manipulator", 
        { 
          {"mesh", "./res/models/ui/manipulator.fbx" }, 
          {"layer", "no_depth" }
        }, 
        {}, 
        {}
      );
    },
    removeObjectById,
    getGameObjectPos,
    setGameObjectPosition
  );

  setSelectedIndex(state.editor, idToUse, selectedObject.name, !state.multiselect);
  state.selectedName = selectedObject.name + "(" + std::to_string(selectedObject.id) + ")";
  state.additionalText = "     <" + std::to_string((int)(255 * pixelColor.r)) + ","  + std::to_string((int)(255 * pixelColor.g)) + " , " + std::to_string((int)(255 * pixelColor.b)) + ">  " + " --- " + state.selectedName;
}

void notSelectItem(){
  auto manipulatorId = getManipulatorId();
  if (manipulatorId != 0){
    onManipulatorUnselect(removeObjectById);
    unsetSelectedIndex(state.editor, manipulatorId, true);
  }
}

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos, glm::vec3 normal){
  auto obj1Id = getIdForCollisionObject(world, obj1);
  auto obj2Id = getIdForCollisionObject(world, obj2);
  auto obj1Name = getGameObject(world, obj1Id).name;
  auto obj2Name = getGameObject(world, obj2Id).name;
  maybeTeleportObjects(world, obj1Id, obj2Id);
  schemeBindings.onCollisionEnter(obj1Id, obj2Id, contactPos, normal, normal * glm::vec3(-1.f, -1.f, -1.f)); 
}
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2){
  schemeBindings.onCollisionExit(getIdForCollisionObject(world, obj1), getIdForCollisionObject(world, obj2));
}


// This is wasteful, as obviously I shouldn't be loading in all the textures on load, but ok for now. 
// This shoiuld really just be creating a list of names, and then the cycle above should cycle between possible textures to load, instead of what is loaded 
void loadAllTextures(){
  for (auto texturePath : listFilesWithExtensions(textureFolderPath, { "png", "jpg" })){
    loadTextureWorld(world, texturePath, -1);
  }
  for (auto texturePath : listFilesWithExtensions("/home/brad/automate/mosttrusted/gameresources/build/", { "png", "jpg" })){
    loadTextureWorld(world, texturePath, -1);
  }
}

void addLineNextCycle(glm::vec3 fromPos, glm::vec3 toPos){
  Line line = {
    .fromPos = fromPos,
    .toPos = toPos
  };
  lines.push_back(line);
}

std::vector<glm::vec3> traversalPositions;
std::vector<glm::vec3> parentTraversalPositions;
void addPositionToRender(glm::mat4 modelMatrix, glm::mat4 parentMatrix){
  //traversalPositions.push_back(getTransformationFromMatrix(modelMatrix).position);
  //parentTraversalPositions.push_back(getTransformationFromMatrix(parentMatrix).position);
}
void clearTraversalPositions(){
  traversalPositions.clear();
  parentTraversalPositions.clear();
}
void drawTraversalPositions(){
  for (int i = 0; i < traversalPositions.size(); i++){
    auto fromPos = traversalPositions.at(i);
    auto toPos = parentTraversalPositions.at(i);
    addLineNextCycle(fromPos, toPos);
  }
}

std::map<std::string, GLint> shaderNameToId;
GLint getShaderByName(std::string fragShaderName, GLint shaderProgram){
  if (fragShaderName == ""){
    return shaderProgram;
  }
  if (shaderNameToId.find(fragShaderName) == shaderNameToId.end()){
    auto shaderId = loadShader(shaderFolderPath + "/vertex.glsl", fragShaderName);
    shaderNameToId[fragShaderName] = shaderId;   
  }
  return shaderNameToId.at(fragShaderName);
}

void setShaderData(GLint shader, glm::mat4 proj, glm::mat4 view, std::vector<LightInfo>& lights, bool orthographic, glm::vec3 color, objid id, std::vector<glm::mat4> lightProjview, glm::vec3 cameraPosition){
  auto projview = (orthographic ? glm::ortho(-1.f, 1.f, -1.f, 1.f, 0.f, 100.0f) : proj) * view;

  glUseProgram(shader);
  glUniform1i(glGetUniformLocation(shader, "maintexture"), 0);        
  glUniform1i(glGetUniformLocation(shader, "emissionTexture"), 1);
  glUniform1i(glGetUniformLocation(shader, "opacityTexture"), 2);
  glUniform1i(glGetUniformLocation(shader, "lightDepthTexture"), 3);
  glUniform1i(glGetUniformLocation(shader, "cubemapTexture"), 4);


  glActiveTexture(GL_TEXTURE0); 

  glUniformMatrix4fv(glGetUniformLocation(shader, "projview"), 1, GL_FALSE, glm::value_ptr(projview));
  glUniform3fv(glGetUniformLocation(shader, "cameraPosition"), 1, glm::value_ptr(cameraPosition));
  glUniform1i(glGetUniformLocation(shader, "enableDiffuse"), state.enableDiffuse);
  glUniform1i(glGetUniformLocation(shader, "enableSpecular"), state.enableSpecular);

  glUniform1i(glGetUniformLocation(shader, "numlights"), lights.size());
  for (int i = 0; i < lights.size(); i++){
    glm::vec3 position = lights.at(i).pos;
    glUniform3fv(glGetUniformLocation(shader, ("lights[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(shader, ("lightscolor[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(lights.at(i).light.color));
    glUniform3fv(glGetUniformLocation(shader, ("lightsdir[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(directionFromQuat(lights.at(i).rotation)));
    glUniform3fv(glGetUniformLocation(shader, ("lightsatten[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(lights.at(i).light.attenuation));
    glUniform1f(glGetUniformLocation(shader,  ("lightsmaxangle[" + std::to_string(i) + "]").c_str()), lights.at(i).light.maxangle);
    glUniform1i(glGetUniformLocation(shader,  ("lightsisdir[" + std::to_string(i) + "]").c_str()), lights.at(i).light.type == LIGHT_DIRECTIONAL);

    if (lightProjview.size() > i){
      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, depthTextures[1]);
      glUniformMatrix4fv(glGetUniformLocation(shader, "lightsprojview"), 1, GL_FALSE, glm::value_ptr(lightProjview.at(i)));
      glActiveTexture(GL_TEXTURE0);
    }
  }
  /////////////////////////////
  glUniform3fv(glGetUniformLocation(shader, "tint"), 1, glm::value_ptr(color));
  glUniform4fv(glGetUniformLocation(shader, "encodedid"), 1, glm::value_ptr(getColorFromGameobject(id)));
}

glm::vec3 getTintIfSelected(bool isSelected){
  if (isSelected && state.highlight){
    return glm::vec3(1.0f, 0.0f, 0.0f);
  }
  return glm::vec3(1.f, 1.f, 1.f);
}

int renderWorld(World& world,  GLint shaderProgram, glm::mat4* projection, glm::mat4 view,  glm::mat4 model, std::vector<LightInfo>& lights, std::vector<PortalInfo> portals, std::vector<glm::mat4> lightProjview, glm::vec3 cameraPosition){
  glUseProgram(shaderProgram);
  clearTraversalPositions();
  int numTriangles = 0;
  int numDepthClears = 0;

  traverseSandbox(world.sandbox, [&world, &layers, &numDepthClears, shaderProgram, projection, view, &portals, &lights, &lightProjview, &numTriangles, &cameraPosition](int32_t id, glm::mat4 modelMatrix, glm::mat4 parentModelMatrix, LayerInfo& layer, std::string shader) -> void {
    assert(id >= 0);
    auto proj = projection == NULL ? projectionFromLayer(layer) : *projection;

     // This could easily be moved to reduce opengl context switches since the onObject sorts on layers (so just have to pass down).  
    bool orthographic = layer.orthographic;
    if (state.depthBufferLayer != layer.depthBufferLayer){
      state.depthBufferLayer = layer.depthBufferLayer;
      glClear(GL_DEPTH_BUFFER_BIT);
      numDepthClears++;
    }

    addPositionToRender(modelMatrix, parentModelMatrix);

    bool objectSelected = idInGroup(world, id, selectedIds(state.editor));
    auto newShader = getShaderByName(shader, shaderProgram);
    setShaderData(newShader, proj, view, lights, orthographic, getTintIfSelected(objectSelected), id, lightProjview, cameraPosition);

    if (state.visualizeNormals){
      glUniformMatrix4fv(glGetUniformLocation(newShader, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
      drawMesh(world.meshes.at("./res/models/cone/cone.obj").mesh, newShader); 
    }
  
    // bounding code //////////////////////
    auto gameObjV = world.objectMapping.at(id);
    auto meshObj = std::get_if<GameObjectMesh>(&gameObjV); 
    if (meshObj != NULL && meshObj -> meshesToRender.size() > 0){
      // @TODO i use first mesh to get sizing for bounding box, obviously that's questionable
      auto bounding = getBoundRatio(world.meshes.at("./res/models/boundingbox/boundingbox.obj").mesh.boundInfo, meshObj -> meshesToRender.at(0).boundInfo);
      glUniformMatrix4fv(glGetUniformLocation(newShader, "model"), 1, GL_FALSE, glm::value_ptr(glm::scale(getMatrixForBoundRatio(bounding, modelMatrix), glm::vec3(1.01f, 1.01f, 1.01f))));

      if (objectSelected){
        drawMesh(world.meshes.at("./res/models/boundingbox/boundingbox.obj").mesh, newShader);
      }
      glUniform1f(glGetUniformLocation(newShader, "discardTexAmount"), meshObj -> discardAmount); 
      glUniform1f(glGetUniformLocation(newShader, "emissionAmount"), meshObj -> emissionAmount); 
    }

    glUniformMatrix4fv(glGetUniformLocation(newShader, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniform1f(glGetUniformLocation(newShader, "time"), getTotalTime());

    bool isPortal = false;
    bool isPerspectivePortal = false;

    for (auto portal : portals){
      if (id == portal.id){
        isPortal = true;
        isPerspectivePortal = portal.perspective;
        break;
      }
    }

    bool portalTextureInCache = portalIdCache.find(id) != portalIdCache.end();
    glStencilMask(isPortal ? 0xFF : 0x00);

    auto trianglesDrawn = renderObject(
      newShader, 
      id, 
      world.objectMapping, 
      state.showCameras, 
      state.showBoneWeight,
      state.useBoneTransform,
      (isPortal && portalTextureInCache &&  !isPerspectivePortal) ? portalIdCache.at(id) : -1,
      modelMatrix,
      state.drawPoints,
      drawWord,
      [&modelMatrix, &newShader](glm::vec3 pos) -> int {
        glUniformMatrix4fv(glGetUniformLocation(newShader, "model"), 1, GL_FALSE, glm::value_ptr(glm::translate(modelMatrix, pos)));
        return drawSphere();
      },
      defaultMeshes
    );
    numTriangles = numTriangles + trianglesDrawn;

    glStencilFunc(GL_EQUAL, 1, 0xFF);
    if (isPortal && portalTextureInCache && isPerspectivePortal){
      glUseProgram(framebufferProgram); 
      glDisable(GL_DEPTH_TEST);
      glBindVertexArray(quadVAO);
      glBindTexture(GL_TEXTURE_2D,  portalIdCache.at(id));
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glEnable(GL_DEPTH_TEST);
      glUseProgram(newShader); 
    }
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);
    addPositionToRender(modelMatrix, parentModelMatrix);
  });
  
  auto maxExpectedClears = numUniqueDepthLayers(layers);
  if (numDepthClears > maxExpectedClears){
    std::cout << "num clears: " << numDepthClears << std::endl;
    std::cout << "num unique depth clears: " << maxExpectedClears << std::endl;
    assert(false);
  }
  return numTriangles;
}

void renderVector(GLint shaderProgram, glm::mat4 view, glm::mat4 model){
  auto projection = projectionFromLayer(layers.at(0));
  glUseProgram(shaderProgram);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projview"), 1, GL_FALSE, glm::value_ptr(projection * view));    
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.05, 1.f, 0.f)));
  glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), false);    

  // Some texture needs to be bound, who cares what. 
  // Probably conceptual fix would be to add "hasMainTexture"
  glBindTexture(GL_TEXTURE_2D, world.textures.at("./res/textures/wood.jpg").texture.textureId);


  // Draw grid for the chunking logic if that is specified, else lots draw the snapping translations
  if (numChunkingGridCells > 0){
    float offset = ((numChunkingGridCells % 2) == 0) ? (dynamicLoading.mappingInfo.chunkSize / 2) : 0;
    drawGrid3DCentered(numChunkingGridCells, dynamicLoading.mappingInfo.chunkSize, offset, offset, offset);
    glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.05, 1.f, 0.f)));
  }else{
    for (auto id : selectedIds(state.editor)){
      auto selectedObj = id;
      if (state.manipulatorMode == TRANSLATE && selectedObj != -1){
        float snapGridSize = getSnapTranslateSize();
        if (snapGridSize > 0){
          auto position = getGameObjectPosition(selectedObj, false);
          drawGrid3DCentered(10, snapGridSize, position.x, position.y, position.z);  
          glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.05, 1.f, 1.f)));     
        }
      }
    }
  }
  drawCoordinateSystem(100.f);

  glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.f, 1.f, 0.f)));
  if (permaLines.size() > 0){
   drawLines(permaLines);
  }

  glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(1.f, 0.f, 0.f)));
  if (lines.size() > 0){
   drawLines(lines);
  }

  glUniform3fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec3(0.f, 1.f, 0.f)));
  if (bluelines.size() > 0){
   drawLines(bluelines);
  }
  lines.clear();
  bluelines.clear();

  if (state.showCameras){
    drawTraversalPositions();   
  }
}

void renderSkybox(GLint shaderProgram, glm::mat4 view, glm::vec3 cameraPosition){
  auto projection = projectionFromLayer(layers.at(0));
  std::vector<LightInfo> lights = {};
  std::vector<glm::mat4> lightProjView = {};

  auto value = glm::mat3(view);  // Removes last column aka translational component --> thats why when you move skybox no move!
  setShaderData(shaderProgram, projection, value, lights, false, glm::vec3(1.f, 1.f, 1.f), 0, lightProjView, cameraPosition);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
  drawMesh(world.meshes.at("skybox").mesh, shaderProgram); 
}

void renderUI(Mesh& crosshairSprite, unsigned int currentFramerate, Color pixelColor, int numObjects, int numScenesLoaded){
  glUseProgram(uiShaderProgram);
  glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(orthoProj)); 

  if (!showDebugInfo){
    return;
  }

  if(!state.isRotateSelection){
     drawSpriteAround(uiShaderProgram, crosshairSprite, state.cursorLeft, state.currentScreenHeight - state.cursorTop, 20, 20);
  }

  drawText(std::to_string(currentFramerate) + state.additionalText, 10, 20, 4);

  std::string manipulatorAxisString;
  if (state.manipulatorAxis == XAXIS){
    manipulatorAxisString = "xaxis";
  }else if (state.manipulatorAxis == YAXIS){
    manipulatorAxisString = "yaxis";
  }else if (state.manipulatorAxis == ZAXIS){
    manipulatorAxisString = "zaxis";
  }else{
    manipulatorAxisString = "noaxis";
  }
  drawText("manipulator axis: " + manipulatorAxisString, 10, 50, 3);
  drawText("position: " + print(defaultCamera.transformation.position), 10, 60, 3);
  drawText("rotation: " + print(defaultCamera.transformation.rotation), 10, 70, 3);
  drawText("cursor: " + std::to_string(state.cursorLeft) + " / " + std::to_string(state.cursorTop)  + "(" + std::to_string(state.currentScreenWidth) + "||" + std::to_string(state.currentScreenHeight) + ")", 10, 90, 3);
  
  if (selected(state.editor) != -1){
    auto obj = getGameObject(world, selected(state.editor));
    drawText("position: " + print(obj.transformation.position), 10, 100, 3);
    drawText("scale: " + print(obj.transformation.scale), 10, 110, 3);
    drawText("rotation: " + print(obj.transformation.rotation), 10, 120, 3);
  }
    
  drawText("pixel color: " + std::to_string(pixelColor.r) + " " + std::to_string(pixelColor.g) + " " + std::to_string(pixelColor.b), 10, 140, 3);
  drawText("showing color: " + std::string(state.showBoneWeight ? "bone weight" : "bone indicies") , 10, 150, 3);

  drawText(std::string("animation info: ") + (timePlayback.isPaused() ? "paused" : "playing"), 10, 170, 3);
  drawText("using animation: " + std::to_string(-1) + " / " + std::to_string(-1) , 40, 180, 3);
  drawText("using object id: -1" , 40, 190, 3);

  drawText(std::string("triangles: ") + std::to_string(numTriangles), 10, 200, 3);
  drawText(std::string("num gameobjects: ") + std::to_string(numObjects), 10, 210, 3);
  drawText(std::string("num scenes loaded: ") + std::to_string(numScenesLoaded), 10, 220, 3);
}

void onClientMessage(std::string message){
  schemeBindings.onTcpMessage(message);
}

std::string screenshotPath = "./res/textures/screenshot.png";
void takeScreenshot(std::string filepath){
  state.takeScreenshot = true;
  screenshotPath = filepath;
}

void genFramebufferTexture(unsigned int *texture){
  glGenTextures(1, texture);
  glBindTexture(GL_TEXTURE_2D, *texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state.currentScreenWidth, state.currentScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

bool wroteCrash = false;
void signalHandler(int signum) {
  if (showDebugInfo && !wroteCrash){
    wroteCrash = true;
    auto crashFile = "./build/crash.info";
    std::cout << "wrote crash file: " << crashFile << std::endl;
    saveFile(crashFile, dumpDebugInfo());
  }
  exit(signum);  
}

void onObjDelete(objid id){
 std::cout << "deleted obj id: " << id << std::endl;
  if (state.activeCameraObj != NULL &&  id == state.activeCameraObj -> id){
    state.activeCameraObj = NULL;
    std::cout << "active camera reset" << std::endl;
  }
  if (id == isSelected(state.editor, id)){
    unsetSelectedIndex(state.editor, id, true);
  }
}

std::map<std::string, std::string> args;
std::map<std::string, std::string> getArgs(){
  return args;
}


int main(int argc, char* argv[]){
  signal(SIGABRT, signalHandler);  

  cxxopts::Options cxxoption("ModEngine", "ModEngine is a game engine for hardcore fps");
  cxxoption.add_options()
   ("s,shader", "Folder path of default shader", cxxopts::value<std::string>()->default_value("./res/shaders/default"))
   ("t,texture", "Additional textures folder to use", cxxopts::value<std::string>()->default_value("./res"))
   ("x,scriptpath", "Script file to use", cxxopts::value<std::vector<std::string>>()->default_value(""))
   ("u,uishader", "Shader to use for ui", cxxopts::value<std::string>()->default_value("./res/shaders/ui"))
   ("c,camera", "Camera to use after initial load", cxxopts::value<std::string>()->default_value(""))
   ("o,font", "Font to use", cxxopts::value<std::string>()->default_value("./res/textures/fonts/gamefont"))
   ("f,fullscreen", "Enable fullscreen mode", cxxopts::value<bool>()->default_value("false"))
   ("i,info", "Show debug info", cxxopts::value<bool>()->default_value("false"))
   ("k,skiploop", "Skip main game loop", cxxopts::value<bool>()->default_value("false"))
   ("d,dumpphysics", "Dump physics info to file for external processing", cxxopts::value<bool>()->default_value("false"))
   ("b,bootstrapper", "Run the server as a server bootstrapper only", cxxopts::value<bool>()->default_value("false"))
   ("p,physics", "Enable physics", cxxopts::value<bool>()->default_value("false"))
   ("y,debugphysics", "Enable physics debug drawing", cxxopts::value<bool>()->default_value("false"))
   ("n,noinput", "Disable default input (still allows custom input handling in scripts)", cxxopts::value<bool>()->default_value("false"))
   ("g,grid", "Size of grid chunking grid used for open world streaming, default to zero (no grid)", cxxopts::value<int>()->default_value("0"))
   ("w,world", "Use streaming chunk system", cxxopts::value<std::string>()->default_value(""))
   ("r,rawscene", "Rawscene file to use (only used when world = false)", cxxopts::value<std::vector<std::string>>() -> default_value(""))
   ("a,args", "Args to provide to scheme", cxxopts::value<std::vector<std::string>>() -> default_value(""))
   ("m,mapping", "Key mapping file to use", cxxopts::value<std::string>()->default_value(""))
   ("l,benchmark", "Benchmark file to write results", cxxopts::value<std::string>()->default_value(""))
   ("e,timetoexit", "Time to run the engine before exiting in ms", cxxopts::value<int>()->default_value("0"))
   ("q,headlessmode", "Hide the window of the game engine", cxxopts::value<bool>()->default_value("false"))
   ("j,extensions", "SO files to load", cxxopts::value<std::vector<std::string>>() -> default_value(""))
   ("z,layers", "Layers file to specify render layers", cxxopts::value<std::string>() -> default_value("./res/layers.layerinfo"))
   ("h,help", "Print help")
  ;        

  const auto result = cxxoption.parse(argc, argv);
  bool dumpPhysics = result["dumpphysics"].as<bool>();
  bool headlessmode = result["headlessmode"].as<bool>();
  numChunkingGridCells = result["grid"].as<int>();

  std::string worldfile = result["world"].as<std::string>();
  useChunkingSystem = worldfile != "";

  auto parsedArgs = result["args"].as<std::vector<std::string>>();
  for (auto arg : parsedArgs){
    auto parsedArg = split(arg, '=');
    assert(parsedArg.size() <= 2);
    if (parsedArg.size() == 2){
      args[parsedArg.at(0)] = parsedArg.at(1);
    }else{
      args[parsedArg.at(0)] = "";
    }
  }

  layers = parseLayerInfo(result["layers"].as<std::string>());

  auto rawScenes = result["rawscene"].as<std::vector<std::string>>();
  rawSceneFile =  rawScenes.size() > 0 ? rawScenes.at(0) : "./res/scenes/example.rawscene";

  auto extensions = loadExtensions(result["extensions"].as<std::vector<std::string>>());
  extensionsInit(extensions, currentModuleId, getArgs);

  keyMapper = readMapping(result["mapping"].as<std::string>(), inputFns);

  if (result["help"].as<bool>()){
    std::cout << cxxoption.help() << std::endl;
    return 0;
  }
  bool enablePhysics = result["physics"].as<bool>();
  bootStrapperMode = result["bootstrapper"].as<bool>();

  shaderFolderPath = result["shader"].as<std::string>();
  textureFolderPath = result["texture"].as<std::string>();
  const std::string framebufferShaderPath = "./res/shaders/framebuffer";
  const std::string uiShaderPath = result["uishader"].as<std::string>();
  showDebugInfo = result["info"].as<bool>();
  
  auto benchmarkFile = result["benchmark"].as<std::string>();
  auto shouldBenchmark = benchmarkFile != "";
  auto timetoexit = result["timetoexit"].as<int>();

  benchmark = createBenchmark(shouldBenchmark);

  std::cout << "LIFECYCLE: program starting" << std::endl;
  disableInput = result["noinput"].as<bool>();

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  state.currentScreenWidth = mode->width;
  state.currentScreenHeight = mode->height;
 
  GLFWwindow* window = glfwCreateWindow(state.currentScreenWidth, state.currentScreenHeight, "ModEngine", result["fullscreen"].as<bool>() ? monitor : NULL, NULL);

  if (window == NULL){
    std::cerr << "ERROR: failed to create window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (headlessmode){
    glfwHideWindow(window);
  }else{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
  }

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
     std::cerr << "ERROR: failed to load opengl functions" << std::endl;
     glfwTerminate();
     return -1;
  }

  startSoundSystem();

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);  

  genFramebufferTexture(&framebufferTexture);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

  genFramebufferTexture(&framebufferTexture2);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebufferTexture2, 0);

  genFramebufferTexture(&framebufferTexture3);

  generateDepthTextures();
  generatePortalTextures();
  setActiveDepthTexture(0);

  if (!glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE){
    std::cerr << "ERROR: framebuffer incomplete" << std::endl;
    return -1;
  }

  float quadVertices[] = {
    -1.0f,  1.0f,  0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f, 0.0f,
     1.0f, -1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f,  0.0f, 1.0f,
    1.0f, -1.0f,  1.0f, 0.0f,
    1.0f,  1.0f,  1.0f, 1.0f
  };
  unsigned int quadVBO;
  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0); 
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
  
  auto onFramebufferSizeChange = [](GLFWwindow* window, int width, int height) -> void {
     std::cout << "EVENT: framebuffer resized:  new size-  " << "width("<< width << ")" << " height(" << height << ")" << std::endl;
     state.currentScreenWidth = width;
     state.currentScreenHeight = height;
     glBindTexture(GL_TEXTURE_2D, framebufferTexture);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state.currentScreenWidth, state.currentScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

     glBindTexture(GL_TEXTURE_2D, framebufferTexture2);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state.currentScreenWidth, state.currentScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

     glBindTexture(GL_TEXTURE_2D, framebufferTexture3);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, state.currentScreenWidth, state.currentScreenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

     glViewport(0, 0, state.currentScreenWidth, state.currentScreenHeight);

     updateDepthTexturesSize();
     updatePortalTexturesSize();


     // TODO orthoproj is using current screen width and height.  Switch this to match NDI for simplification. 
     orthoProj = glm::ortho(0.0f, (float)state.currentScreenWidth, 0.0f, (float)state.currentScreenHeight, -1.0f, 1.0f);  
  }; 

  onFramebufferSizeChange(window, state.currentScreenWidth, state.currentScreenHeight);
  glfwSetFramebufferSizeCallback(window, onFramebufferSizeChange); 
  glPointSize(10.f);
  
  std::cout << "INFO: shader file path is " << shaderFolderPath << std::endl;
  unsigned int shaderProgram = loadShader(shaderFolderPath + "/vertex.glsl", shaderFolderPath + "/fragment.glsl");
  
  std::cout << "INFO: framebuffer file path is " << framebufferShaderPath << std::endl;
  framebufferProgram = loadShader(framebufferShaderPath + "/vertex.glsl", framebufferShaderPath + "/fragment.glsl");

  std::string depthShaderPath = "./res/shaders/depth";
  std::cout << "INFO: depth file path is " << depthShaderPath << std::endl;
  unsigned int depthProgram = loadShader(depthShaderPath + "/vertex.glsl", depthShaderPath + "/fragment.glsl");

  std::cout << "INFO: ui shader file path is " << uiShaderPath << std::endl;
  uiShaderProgram = loadShader(uiShaderPath + "/vertex.glsl",  uiShaderPath + "/fragment.glsl");

  std::string selectionShaderPath = "./res/shaders/selection";
  std::cout << "INFO: selection shader path is " << selectionShaderPath << std::endl;
  unsigned int selectionProgram = loadShader(selectionShaderPath + "/vertex.glsl", selectionShaderPath + "/fragment.glsl");

  std::string drawingShaderPath = "./res/shaders/drawing";
  std::cout << "INFO: drawing shader path is: " << drawingShaderPath << std::endl;
  drawingProgram = loadShader(drawingShaderPath + "/vertex.glsl", drawingShaderPath + "/fragment.glsl");

  std::string blurShaderPath = "./res/shaders/blur";
  std::cout << "INFO: blur shader path is: " << blurShaderPath << std::endl;
  blurProgram = loadShader(blurShaderPath + "/vertex.glsl", blurShaderPath + "/fragment.glsl");

  fontMeshes = loadFontMeshes(readFont(result["font"].as<std::string>()));
  Mesh crosshairSprite = loadSpriteMesh("./res/textures/crosshairs/crosshair029.png", loadTexture);
 
  createStaticSchemeBindings(
    listSceneId,
    loadScene,
    unloadScene,
    unloadAllScenes,
    listScenes,
    listSceneFiles,
    sendLoadScene,
    createScene,
    moveCamera, 
    rotateCamera, 
    removeObjectById, 
    getObjectsByType, 
    setActiveCamera,
    drawText,
    addLineNextCycle,
    getGameObjectName,
    getGameObjectAttr,
    setGameObjectAttr,
    getGameObjectPosition,
    setGameObjectPosition,
    setGameObjectPositionRelative,
    getGameObjectRotation,
    setGameObjectRotation,
    setFrontDelta,
    moveRelative,
    orientationFromPos,
    getGameObjectByName,
    applyImpulse,
    applyImpulseRel,
    clearImpulse,
    listAnimations,
    playAnimation,
    listSounds,
    playSoundState,
    listModels,
    sendNotifyMessage,
    timeSeconds,
    timeElapsed,
    saveScene,
    listServers,
    connectServer,
    disconnectServer,
    sendMessageToActiveServer,
    sendDataUdp,
    playRecording,
    stopRecording,
    createRecording,
    saveRecording,
    makeObjectAttr,
    makeParent,
    raycastW,
    takeScreenshot,
    setState,
    setFloatState,
    setIntState,
    navPosition, 
    scmEmit,
    addLoadingAround,
    removeLoadingAround,
    createGeneratedMesh,
    setSkybox,
    getArgs,
    lock,
    unlock,
    debugInfo,
    extensions.registerGuileFns
  );
  registerGuileTypes(extensions);


  schemeBindings = getSchemeCallbacks();
  if(bootStrapperMode){
    netcode = initNetCode(schemeBindings.onPlayerJoined, schemeBindings.onPlayerLeave);
  }

  for (auto script : result["scriptpath"].as<std::vector<std::string>>()){
    loadScript(script, getUniqueObjId(), -1, bootStrapperMode, true);
  }

  BulletDebugDrawer drawer(addLineNextCycle);
  btIDebugDraw* debuggerDrawer = result["debugphysics"].as<bool>() ?  &drawer : NULL;

  interface = SysInterface {
    .loadScript = loadScriptFromWorld,
    .unloadScript = [&extensions](std::string scriptpath, objid id) -> void {
      unloadScript(scriptpath, id, [&extensions, id]() -> void {
        extensionsUnloadScript(extensions, id);
      }); 
      removeLocks(id);
    },
    .stopAnimation = stopAnimation,
    .getCurrentTime = getTotalTime
  };

  std::vector<std::string> defaultMeshesToLoad {
    "./res/models/ui/node.obj",
    "./res/models/boundingbox/boundingbox.obj",
    "./res/models/unit_rect/unit_rect.obj",
    "./res/models/cone/cone.obj",
    "../gameresources/build/objtypes/camera.gltf",
    "./res/models/box/plane.dae",
    "./res/models/controls/input.obj",
    "./res/models/controls/unitxy.obj",  
    "../gameresources/build/objtypes/emitter.gltf",  
    "../gameresources/build/objtypes/sound.gltf",
    "../gameresources/build/objtypes/light.gltf",
  };
  world = createWorld(
    onObjectEnter, 
    onObjectLeave, 
    [&world](GameObject& obj) -> void {
      netObjectUpdate(world, obj, netcode, bootStrapperMode);
    }, 
    [&world](GameObject& obj) -> void {
      netObjectCreate(world, obj, netcode, bootStrapperMode);
    },
    [](objid id, bool isNet) -> void {
      netObjectDelete(id, isNet, onObjDelete, netcode, bootStrapperMode);
    }, 
    debuggerDrawer, 
    layers,
    interface,
    defaultMeshesToLoad
  );
  defaultMeshes = DefaultMeshes{
    .nodeMesh = &world.meshes.at("./res/models/ui/node.obj").mesh,
    .portalMesh = &world.meshes.at("./res/models/box/plane.dae").mesh,
    .cameraMesh = &world.meshes.at("../gameresources/build/objtypes/camera.gltf").mesh, 
    .voxelCubeMesh = &world.meshes.at("./res/models/unit_rect/unit_rect.obj").mesh,
    .unitXYRect = &world.meshes.at("./res/models/controls/unitxy.obj").mesh,
    .soundMesh = &world.meshes.at("../gameresources/build/objtypes/sound.gltf").mesh,
    .lightMesh = &world.meshes.at("../gameresources/build/objtypes/light.gltf").mesh,
    .emitter = &world.meshes.at("../gameresources/build/objtypes/emitter.gltf").mesh,
    .nav = &world.meshes.at("./res/models/ui/node.obj").mesh,
  };

  //loadAllTextures();
  
  dynamicLoading = createDynamicLoading(worldfile);

  std::cout << "INFO: # of intitial raw scenes: " << rawScenes.size() << std::endl;
  for (auto rawScene : rawScenes){
    loadScene(rawScene);
  }
  

  auto defaultCameraName = result["camera"].as<std::string>();
  if (defaultCameraName != ""){
    auto ids =  getByName(world.sandbox, defaultCameraName);
    auto sceneForId = sceneId(world.sandbox, ids.at(0));
    setActiveCamera(defaultCameraName, sceneForId);
  }

  glfwSetCursorPosCallback(window, onMouseEvents); 
  glfwSetMouseButtonCallback(window, onMouseCallback);
  glfwSetScrollCallback(window, onScrollCallback);
  glfwSetKeyCallback(window, keyCallback);
  glfwSetCharCallback(window, keyCharCallback);
  glfwSetDropCallback(window, drop_callback);
  glfwSetJoystickCallback(joystickCallback);

  unsigned int frameCount = 0;
  float previous = glfwGetTime();
  float last60 = previous;

  unsigned int currentFramerate = 0;
  std::cout << "INFO: render loop starting" << std::endl;

  GLenum buffers_to_render[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2,buffers_to_render);

  int frameratelimit = 60;
  bool hasFramelimit = frameratelimit != 0;
  float minDeltaTime = !hasFramelimit ? 0 : (1.f / frameratelimit);
  
  if (result["skiploop"].as<bool>()){
    goto cleanup;
  }

  state.cullEnabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  PROFILE("MAINLOOP",
  while (!glfwWindowShouldClose(window)){
  PROFILE("FRAME",
    frameCount++;

    fpscountstart:
    now = glfwGetTime();
    deltaTime = now - previous;   

    if (timetoexit != 0){
      float timeInSeconds = timetoexit / 1000.f;
      if (now > timeInSeconds){
        std::cout << "INFO: TIME TO EXIST EXPIRED" << std::endl;
        goto cleanup;
      }
    }

    if (hasFramelimit &&  (deltaTime < minDeltaTime)){
      goto fpscountstart;
    }

    previous = now;

    int numObjects = getNumberOfObjects(world.sandbox);
    int numScenesLoaded = getNumberScenesLoaded(world.sandbox);
    logBenchmarkTick(benchmark, deltaTime, numObjects, numTriangles);

    if (!state.pauseWorldTiming){
      timePlayback.setElapsedTime(deltaTime);
    }

    if (frameCount == 60){
      frameCount = 0;
      float timedelta = now - last60;
      last60 = now;
      currentFramerate = floor((60.f/(timedelta) + 0.5f));
    }

    onWorldFrame(world, deltaTime, getTotalTime(), enablePhysics, dumpPhysics, interface);

    auto time = getTotalTime();
    tickRecordings(time);

    onNetCode(world, interface, netcode, onClientMessage, bootStrapperMode);

    auto viewTransform = (state.useDefaultCamera || state.activeCameraObj == NULL) ? defaultCamera.transformation : fullTransformation(world.sandbox, state.activeCameraObj -> id);
    
    auto forward = calculateRelativeOffset(viewTransform.rotation, {0, 0, -1 }, false);
    auto up  = calculateRelativeOffset(viewTransform.rotation, {0, 1, 0 }, false);
    setListenerPosition(
      viewTransform.position.x, viewTransform.position.y, viewTransform.position.z,
      { forward.x, forward.y, forward.z},
      { up.x, up.y, up.z }
    );
    
    view = renderView(viewTransform.position, viewTransform.rotation);

    glfwSwapBuffers(window);
    
    std::vector<LightInfo> lights = getLightInfo(world);
    std::vector<PortalInfo> portals = getPortalInfo(world);
    assert(portals.size() <= numPortalTextures);

    // depth buffer from point of view SMf 1 light source (all eventually, but 1 for now)

    std::vector<glm::mat4> lightMatrixs;

    for (int i = 0; i < lights.size(); i++){
      setActiveDepthTexture(i + 1);
      auto light = lights.at(i);
      auto lightView = renderView(light.pos, light.rotation);
    
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebufferTexture2, 0);

      glEnable(GL_DEPTH_TEST);
      glClearColor(255.0, 255.0, 255.0, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glm::mat4 lightProjection = glm::ortho<float>(-2000, 2000,-2000, 2000, 1.f, 3000);  // need to choose these values better
      auto lightProjview = lightProjection * lightView;
      lightMatrixs.push_back(lightProjview);

      renderWorld(world, selectionProgram, &lightProjection, lightView, glm::mat4(1.0f), lights, portals, {}, light.pos); 
    }

    PROFILE(
      "RENDERING-SELECTION",

      setActiveDepthTexture(0);
      // 1ST pass draws selection program shader to be able to handle selection 
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      glEnable(GL_DEPTH_TEST);
      glClearColor(0.0, 0.0, 0.0, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glDisable(GL_BLEND);
    
      renderWorld(world, selectionProgram, NULL, view, glm::mat4(1.0f), lights, portals, lightMatrixs, viewTransform.position);
    )

    // Each portal requires a render pass
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_BLEND);

    auto uvCoord = getUVCoord(state.cursorLeft, state.cursorTop, state.currentScreenHeight);
    Color hoveredItemColor = getPixelColor(state.cursorLeft, state.cursorTop, state.currentScreenHeight);
    auto hoveredId = getIdFromColor(hoveredItemColor);
    
    state.lastHoveredIdInScene = state.hoveredIdInScene;
    state.hoveredIdInScene = idExists(world.sandbox, hoveredId);
    state.lastHoverIndex = state.currentHoverIndex;
    state.currentHoverIndex = hoveredId;

    if (selectItemCalled){
      if (state.hoveredIdInScene){
        selectItem(hoveredId, hoveredItemColor);
      }else{
        notSelectItem();
      }
      selectItemCalled = false;
      applyUICoord(
        world.objectMapping, 
        [](std::string topic, float value) -> void { 
          StringString message {
            .strTopic = topic,
            .strValue = value,
          };
          channelMessages.push(message);
        }, 
        selected(state.editor), 
        uvCoord.x, 
        uvCoord.y
      );
    }
    onManipulatorUpdate(
      getGameObjectPos, 
      setGameObjectPosition, 
      getGameObjectScale,
      setGameObjectScale,
      view, 
      state.manipulatorMode, 
      state.offsetX, 
      state.offsetY
    );
    handlePainting(uvCoord);
    handleTerrainPainting(uvCoord);
     
    if (useChunkingSystem){
      handleChunkLoading(
        dynamicLoading, 
        [](objid id) -> glm::vec3 { 
          return getGameObjectPosition(id, true);
        }, 
        loadSceneParentOffset, 
        removeObjectById
      );
    }

    assert(portals.size() <= numPortalTextures);

    PROFILE("PORTAL_RENDERING", 
      std::map<objid, unsigned int> nextPortalCache;
      for (int i = 0; i < portals.size(); i++){
        auto portal = portals.at(i);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, portalTextures[i], 0);
        glClearColor(0.0, 0.0, 0.0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto portalViewMatrix = renderPortalView(portal, viewTransform);
        // Render Skybox code -> need to think harder about stencil
        glDepthMask(GL_FALSE);
        renderSkybox(shaderProgram, portalViewMatrix, portal.cameraPos);  // Probably better to render this at the end 
        glDepthMask(GL_TRUE);
        /////////////////////////////

        renderWorld(world, shaderProgram, NULL, portalViewMatrix, glm::mat4(1.0f), lights, portals, lightMatrixs, portal.cameraPos);
      
        nextPortalCache[portal.id] = portalTextures[i];
      }
      portalIdCache = nextPortalCache;
    )

    PROFILE("MAIN_RENDERING",
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glUseProgram(framebufferProgram); 
      glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glDisable(GL_DEPTH_TEST);
      glBindVertexArray(quadVAO);
      glBindTexture(GL_TEXTURE_2D, framebufferTexture);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      // 2ND pass renders what we care about to the screen.
      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      glBindTexture(GL_TEXTURE_2D, framebufferTexture);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

      glClearColor(0.0, 0.0, 0.0, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |  GL_STENCIL_BUFFER_BIT);

      glDepthMask(GL_FALSE);
      glDisable(GL_STENCIL_TEST);
      renderSkybox(shaderProgram, view, viewTransform.position);  // Probably better to render this at the end 

      glDepthMask(GL_TRUE);
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_STENCIL_TEST);
      glStencilMask(0xFF);
      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  
      glStencilFunc(GL_ALWAYS, 1, 0xFF);

      numTriangles = renderWorld(world, shaderProgram, NULL, view, glm::mat4(1.0f), lights, portals, lightMatrixs, viewTransform.position);
    )

    Color pixelColor = getPixelColor(state.cursorLeft, state.cursorTop, state.currentScreenHeight);
    if (shouldCallItemSelected){
      auto selectedId = selected(state.editor);
      if (selectedId != -1){
        schemeBindings.onObjectSelected(selectedId, glm::vec3(pixelColor.r, pixelColor.g, pixelColor.b));
      }
      shouldCallItemSelected = false;
    }

    if (state.lastHoverIndex != state.currentHoverIndex){
      if (state.lastHoveredIdInScene){
        schemeBindings.onObjectHover(state.lastHoverIndex, false);
      }
      if (state.hoveredIdInScene){
        schemeBindings.onObjectHover(state.currentHoverIndex, true);
      }
    }

    glDisable(GL_STENCIL_TEST);

    if (showDebugInfo){
      renderVector(shaderProgram, view, glm::mat4(1.0f));
    }
    renderUI(crosshairSprite, currentFramerate, pixelColor, numObjects, numScenesLoaded);

    handleInput(window);
    glfwPollEvents();
    
    schemeBindings.onFrame();
    schemeBindings.onMessage(channelMessages);  // modifies the queue
    extensionsOnFrame(extensions);

    portalIdCache.clear();


    // depends on framebuffer texture, outputs to framebuffer texture 2
    // Blurring draws the framebuffer texture 
    // The blur program blurs it one in one direction and saves in framebuffer texture 3 
    // then we take framebuffer texture 3, and use that like the original framebuffer texture
    // run it through again, blurring in other fucking direction 
    // We swap to attachment 2 which was just the old bloom attachment for final render pass
    PROFILE("BLOOM-RENDERING",
      glUseProgram(blurProgram);
      glUniform1i(glGetUniformLocation(blurProgram, "firstpass"), true);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture3, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebufferTexture3, 0);

      glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glDisable(GL_DEPTH_TEST | GL_STENCIL_TEST);

      glBindTexture(GL_TEXTURE_2D, framebufferTexture2);
      glBindVertexArray(quadVAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);
   
      glUniform1i(glGetUniformLocation(blurProgram, "firstpass"), false);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture2, 0);
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebufferTexture2, 0);
      glBindTexture(GL_TEXTURE_2D, framebufferTexture3);
      glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    )

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glUseProgram((state.renderMode == RENDER_DEPTH) ? depthProgram : framebufferProgram); 
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);

    glActiveTexture(GL_TEXTURE1);
    glUniform1i(glGetUniformLocation(framebufferProgram, "enableBloom"), state.enableBloom);
    glUniform1f(glGetUniformLocation(framebufferProgram, "bloomAmount"), state.bloomAmount);
    glUniform1i(glGetUniformLocation(framebufferProgram, "bloomTexture"), 1);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture2);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(framebufferProgram, "framebufferTexture"), 0);

    if (state.renderMode == RENDER_FINAL){
      glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    }else if (state.renderMode == RENDER_PORTAL){
      assert(state.textureIndex <= numPortalTextures && state.textureIndex >= 0);
      glBindTexture(GL_TEXTURE_2D, portalTextures[state.textureIndex]);  
    }else if (state.renderMode == RENDER_PAINT){
      glBindTexture(GL_TEXTURE_2D, textureToPaint);
    }else if (state.renderMode == RENDER_DEPTH){
      assert(state.textureIndex <=  numDepthTextures && state.textureIndex >= 0);
      glBindTexture(GL_TEXTURE_2D, depthTextures[state.textureIndex]);
    }

    glDrawArrays(GL_TRIANGLES, 0, 6);

    if (state.takeScreenshot){
      state.takeScreenshot = false;
      saveScreenshot(screenshotPath);
    }
  )})

  std::cout << "LIFECYCLE: program exiting" << std::endl;

  cleanup:   
    if (shouldBenchmark){
      saveFile(benchmarkFile, dumpDebugInfo());
    }
    deinitPhysics(world.physicsEnvironment); 
    stopSoundSystem();
    glfwTerminate(); 
    unloadExtensions(extensions);

  return 0;
}
