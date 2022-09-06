#include <csignal>
#include <cxxopts.hpp>

#include "./main_input.h"
#include "./scene/scene.h"
#include "./scene/common/vectorgfx.h"
#include "./scene/animation/timeplayback.h"
#include "./scene/animation/animation.h"
#include "./scene/animation/playback.h"
#include "./scene/animation/recorder.h"
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
#include "./netscene.h"
#include "./worldtiming.h"
#include "./main_test.h"
#include "./renderstages.h"
#include "./cscript/cscript.h"
#include "./cscript/cscripts/cscript_sample.h"
#include "./cscript/cscripts/cscript_scheme.h"
#include "./lines.h"
#include "./scene/common/textures_gen.h"
#include "./modlayer.h"

unsigned int framebufferProgram;
unsigned int drawingProgram;
unsigned int quadVAO;
unsigned int quadVAO3D;

GameObject defaultCamera = GameObject {
  .id = -1,
  .name = "defaultCamera",
  .transformation = Transformation {
    .position = glm::vec3(0.f, 0.f, 0.f),
    .scale = glm::vec3(1.0f, 1.0f, 1.0f),
    .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)),
  }
};

bool showDebugInfo = false;
bool showCursor = false;
std::string shaderFolderPath;

bool disableInput = false;
int numChunkingGridCells = 0;
bool useChunkingSystem = false;
std::string rawSceneFile;
bool bootStrapperMode = false;
NetCode netcode { };

engineState state = getDefaultState(1920, 1080);

World world;
RenderStages renderStages;
DefaultMeshes defaultMeshes;  

Mesh* defaultCrosshairSprite;
Mesh* crosshairSprite;


SysInterface interface;
std::string textureFolderPath;
float now = 0;
float deltaTime = 0.0f; // Time between current frame and last frame
int numTriangles = 0;   // # drawn triangles (eg drawelements(x) -> missing certain calls like eg text)

DynamicLoading dynamicLoading;

std::vector<FontFamily> fontFamily;

glm::mat4 view;
unsigned int framebufferTexture;
unsigned int framebufferTexture2;
unsigned int framebufferTexture3;
unsigned int fbo;
unsigned int depthTextures[32];
unsigned int textureDepthTextures[1];

const int numPortalTextures = 16;
unsigned int portalTextures[16];
std::map<objid, unsigned int> portalIdCache;

glm::mat4 orthoProj;
glm::mat4 ndiOrtho = glm::ortho(-1.f, 1.f, -1.f, 1.f, -1.0f, 1.0f);  

unsigned int uiShaderProgram;

CScriptBindingCallbacks cBindings;

std::queue<StringString> channelMessages;
KeyRemapper keyMapper;
extern std::vector<InputDispatch> inputFns;

std::map<std::string, objid> activeLocks;

LineData lineData = createLines();

auto fpsStat = statName("fps");
auto numObjectsStat = statName("object-count");
auto scenesLoadedStat = statName("scenes-loaded");

// 0th depth texture is the main depth texture used for eg z buffer
// other buffers are for the lights
const int numDepthTextures = 32;
int activeDepthTexture = 0;

DrawingParams drawParams = getDefaultDrawingParams();
Benchmark benchmark;

float initialTime = 0;
WorldTiming timings;

TimePlayback timePlayback(
  initialTime, 
  [](float currentTime, float elapsedTime) -> void {
    tickAnimations(world, timings, elapsedTime);
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

void renderScreenspaceLines(Texture& texture, Texture texture2, bool shouldClear, glm::vec4 clearColor, std::optional<unsigned int> clearTextureId, bool blend){
  auto texSize = getTextureSizeInfo(texture);
  auto texSize2 = getTextureSizeInfo(texture2);
  modassert(texSize.width == texSize2.width && texSize.height == texSize2.height, "screenspace - invalid tex sizes");

  glViewport(0, 0, texSize.width, texSize.height);
  updateDepthTexturesSize(textureDepthTextures, 1, texSize.width, texSize.height); // wonder if this would be better off preallocated per gend texture?
  setActiveDepthTexture(fbo, textureDepthTextures, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.textureId, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,  texture2.textureId, 0);
  
  glUseProgram(uiShaderProgram);

  if (shouldClear){ 
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |  GL_STENCIL_BUFFER_BIT);
  }
  if (blend){
    glEnable(GL_BLEND);
  }else{
    glDisable(GL_BLEND);
  }

  glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(ndiOrtho)); 
  glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
  glUniform1i(glGetUniformLocation(uiShaderProgram, "forceTint"), false);
  if (shouldClear && clearTextureId.has_value()){
    glUniform4fv(glGetUniformLocation(uiShaderProgram, "tint"), 1, glm::value_ptr(clearColor));
    glDisable(GL_DEPTH_TEST);
    glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::scale(
      glm::mat4(1.0f), 
      glm::vec3(2.f, 2.f, 2.f)
    )));
    drawMesh(*defaultMeshes.unitXYRect, uiShaderProgram, clearTextureId.value());
    glEnable(GL_DEPTH_TEST);
  }
  glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
  glUniform1i(glGetUniformLocation(uiShaderProgram, "forceTint"), true);
  glUniform4fv(glGetUniformLocation(uiShaderProgram, "tint"), 1, glm::value_ptr(glm::vec4(1.f, 1.f, 1.f, 1.f)));
  glUniform4fv(glGetUniformLocation(uiShaderProgram, "selectionId"), 1, glm::value_ptr(getColorFromGameobject(0)));
  glUniform4fv(glGetUniformLocation(uiShaderProgram, "encodedid2"), 1, glm::value_ptr(getColorFromGameobject(0)));

  //std::cout << "screenspace: lines" << std::endl;
  drawAllLines(lineData, uiShaderProgram, texture.textureId);

  //std::cout << "screenspace: textdata" << std::endl;
  glUniform1i(glGetUniformLocation(uiShaderProgram, "forceTint"), false);
  glUniform4fv(glGetUniformLocation(uiShaderProgram, "tint"), 1, glm::value_ptr(glm::vec4(0.f, 1.f, 0.f, 1.f)));

  //auto ortho = glm::ortho(0.0f, (float)texSize.width, 0.0f, (float)texSize.height, -1.0f, 1.0f);  
  glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(ndiOrtho)); 
  drawTextData(lineData, uiShaderProgram, fontFamilyByName, texture.textureId,  texSize.height, texSize.width);
}

void handlePaintingModifiesViewport(UVCoord uvsToPaint){
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
  glUniform4fv(glGetUniformLocation(drawingProgram, "tint"), 1, glm::value_ptr(drawParams.tint));

  glBindTexture(GL_TEXTURE_2D, activeTextureId());
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}
void handleTerrainPainting(UVCoord uvCoord){
  if (state.shouldTerrainPaint && state.mouseIsDown){
    applyHeightmapMasking(world, latestSelected(state.editor).value(), state.terrainPaintDown ? -10.f : 10.f, uvCoord.x, uvCoord.y, true);
  }
}


ManipulatorSelection onManipulatorSelected(){
  return ManipulatorSelection {
    .mainObj = latestSelected(state.editor),
    .selectedIds = selectedIds(state.editor),
  }; 
}
   
bool selectItemCalled = false;
bool shouldCallItemSelected = false;
bool mappingClickCalled = false;
void selectItem(objid selectedId, int layerSelectIndex){
  std::cout << "SELECT ITEM CALLED!" << std::endl;
  modlog("selection", (std::string("select item called") + ", selectedId = " + std::to_string(selectedId) + ", layerSelectIndex = " + std::to_string(layerSelectIndex)).c_str());
  if (!showCursor){
    return;
  }
  auto idToUse = state.groupSelection ? getGroupId(world.sandbox, selectedId) : selectedId;
  auto selectedSubObj = getGameObject(world, selectedId);
  auto selectedObject =  getGameObject(world, idToUse);

  if (layerSelectIndex >= 0){
    GameobjAttributes manipulatorAttr {
      .stringAttributes = {
        {"mesh", "./res/models/ui/manipulator.gltf" }, 
        {"layer", "scale" },
      },
      .numAttributes = {},
      .vecAttr = { .vec3 = {}, .vec4 = {} },
    };
    onManipulatorSelectItem(
      idToUse, 
      selectedSubObj.name,
      onManipulatorSelected,
      [&manipulatorAttr]() -> objid {
        std::map<std::string, GameobjAttributes> submodelAttributes = {};
        return makeObjectAttr(0, "manipulator", manipulatorAttr, submodelAttributes).value();
      },
      removeObjectById,
      getGameObjectPos,
      setGameObjectPosition
    );
  }
  if (idToUse == getManipulatorId()){
    return;
  }
  applyPainting(selectedId);
  applyFocusUI(world.objectMapping, selectedId, sendNotifyMessage);
  shouldCallItemSelected = true;

  setSelectedIndex(state.editor, idToUse, selectedObject.name, !state.multiselect);
  state.selectedName = selectedObject.name + "(" + std::to_string(selectedObject.id) + ")";
}


void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos, glm::vec3 normal){
  auto obj1Id = getIdForCollisionObject(world, obj1);
  auto obj2Id = getIdForCollisionObject(world, obj2);
  auto obj1Name = getGameObject(world, obj1Id).name;
  auto obj2Name = getGameObject(world, obj2Id).name;
  maybeTeleportObjects(world, obj1Id, obj2Id);
  cBindings.onCollisionEnter(obj1Id, obj2Id, contactPos, normal, normal * glm::vec3(-1.f, -1.f, -1.f)); 
}
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2){
  cBindings.onCollisionExit(getIdForCollisionObject(world, obj1), getIdForCollisionObject(world, obj2));
}


// This is wasteful, as obviously I shouldn't be loading in all the textures on load, but ok for now. 
// This shoiuld really just be creating a list of names, and then the cycle above should cycle between possible textures to load, instead of what is loaded 
void loadAllTextures(){
  for (auto texturePath : listFilesWithExtensions(textureFolderPath, { "png", "jpg" })){
    loadTextureWorld(world, texturePath, -1);
  }
  /*for (auto texturePath : listFilesWithExtensions("/home/brad/automate/mosttrusted/gameresources/build/", { "png", "jpg" })){
    loadTextureWorld(world, texturePath, -1);
  }*/
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
    addLineToNextCycle(lineData, fromPos, toPos, false, 0, GREEN, std::nullopt);
  }
}

std::map<std::string, GLint> shaderNameToId;
GLint getShaderByName(std::string fragShaderName, GLint shaderProgram, bool allowShaderOverride){
  if (fragShaderName == "" || !allowShaderOverride){
    return shaderProgram;
  }
  if (shaderNameToId.find(fragShaderName) == shaderNameToId.end()){
    auto shaderId = loadShader(shaderFolderPath + "/vertex.glsl", fragShaderName, interface.readFile);
    shaderNameToId[fragShaderName] = shaderId;   
  }
  return shaderNameToId.at(fragShaderName);
}

// Kind of crappy since the uniforms don't unset their values after rendering, but order should be deterministic so ... ok
void setRenderUniformData(unsigned int shader, RenderUniforms& uniforms){
  for (auto &uniform : uniforms.intUniforms){
    glUniform1i(glGetUniformLocation(shader, uniform.uniformName.c_str()), uniform.value);
  }
  for (auto &uniform : uniforms.floatUniforms){
    glUniform1f(glGetUniformLocation(shader, uniform.uniformName.c_str()), uniform.value);
  }
  for (auto &uniform : uniforms.vec3Uniforms){
    glUniform3fv(glGetUniformLocation(shader, uniform.uniformName.c_str()), 1, glm::value_ptr(uniform.value));
  }
  for (auto &uniform : uniforms.floatArrUniforms){
    for (int i = 0; i < uniform.value.size(); i++){
      glUniform1f(glGetUniformLocation(shader,  (uniform.uniformName + "[" + std::to_string(i) + "]").c_str()), uniform.value.at(i));
    }
  }
  for (auto &uniform : uniforms.builtInUniforms){  // todo -> avoid string comparisons
    if (uniform.builtin == "resolution"){
      glUniform2iv(glGetUniformLocation(shader, uniform.uniformName.c_str()), 1, glm::value_ptr(state.resolution));
    }else{
      std::cout << "uniform not supported: " << uniform.builtin << std::endl;
      assert(false);
    }
  }
}

void setShaderData(GLint shader, glm::mat4 proj, glm::mat4 view, std::vector<LightInfo>& lights, bool orthographic, glm::vec3 color, objid id, std::vector<glm::mat4> lightProjview, glm::vec3 cameraPosition, RenderUniforms& uniforms){
  auto projview = (orthographic ? glm::ortho(-1.f, 1.f, -1.f, 1.f, 0.f, 100.0f) : proj) * view;

  glUseProgram(shader);

  glUniform1i(glGetUniformLocation(shader, "maintexture"), 0);        
  glUniform1i(glGetUniformLocation(shader, "emissionTexture"), 1);
  glUniform1i(glGetUniformLocation(shader, "opacityTexture"), 2);
  glUniform1i(glGetUniformLocation(shader, "lightDepthTexture"), 3);
  glUniform1i(glGetUniformLocation(shader, "cubemapTexture"), 4);
  glUniform1i(glGetUniformLocation(shader, "roughnessTexture"), 5);
  glUniform1i(glGetUniformLocation(shader, "normalTexture"), 6);

  glActiveTexture(GL_TEXTURE0); 

  glUniformMatrix4fv(glGetUniformLocation(shader, "projview"), 1, GL_FALSE, glm::value_ptr(projview));
  glUniform3fv(glGetUniformLocation(shader, "cameraPosition"), 1, glm::value_ptr(cameraPosition));
  glUniform1i(glGetUniformLocation(shader, "enableDiffuse"), state.enableDiffuse);
  glUniform1i(glGetUniformLocation(shader, "enableSpecular"), state.enableSpecular);
  glUniform1i(glGetUniformLocation(shader, "enablePBR"), state.enablePBR);
  glUniform1i(glGetUniformLocation(shader, "enableLighting"), true);

  glUniform1i(glGetUniformLocation(shader, "numlights"), lights.size());

  glUniform1i(glGetUniformLocation(shader, "textureid"), 0);


  for (int i = 0; i < lights.size(); i++){
    glm::vec3 position = lights.at(i).transform.position;
    glUniform3fv(glGetUniformLocation(shader, ("lights[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(position));
    glUniform3fv(glGetUniformLocation(shader, ("lightscolor[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(lights.at(i).light.color));
    glUniform3fv(glGetUniformLocation(shader, ("lightsdir[" + std::to_string(i) + "]").c_str()), 1, glm::value_ptr(directionFromQuat(lights.at(i).transform.rotation)));
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
  glUniform4fv(glGetUniformLocation(shader, "tint"), 1, glm::value_ptr(glm::vec4(color.x, color.y, color.z, 1.f)));
  glUniform4fv(glGetUniformLocation(shader, "encodedid"), 1, glm::value_ptr(getColorFromGameobject(id)));
  glUniform3fv(glGetUniformLocation(shader, "ambientAmount"), 1, glm::value_ptr(glm::vec3(state.ambient)));

  setRenderUniformData(shader, uniforms);
}

glm::vec3 getTintIfSelected(bool isSelected){
  if (isSelected && state.highlight){
    return glm::vec3(1.0f, 0.0f, 0.0f);
  }
  return glm::vec3(1.f, 1.f, 1.f);
}

int renderWorld(World& world,  GLint shaderProgram, bool allowShaderOverride, glm::mat4* projection, glm::mat4 view,  glm::mat4 model, std::vector<LightInfo>& lights, std::vector<PortalInfo> portals, std::vector<glm::mat4> lightProjview, glm::vec3 cameraPosition){
  glUseProgram(shaderProgram);
  clearTraversalPositions();
  int numTriangles = 0;
  int numDepthClears = 0;

  traverseSandbox(world.sandbox, [&world, &numDepthClears, shaderProgram, allowShaderOverride, projection, view, &portals, &lights, &lightProjview, &numTriangles, &cameraPosition](int32_t id, glm::mat4 modelMatrix, glm::mat4 parentModelMatrix, LayerInfo& layer, std::string shader) -> void {
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
    auto newShader = getShaderByName(shader, shaderProgram, allowShaderOverride);

    // todo -> need to just cache last shader value (or sort?) so don't abuse shader swapping (ok for now i guess)
    MODTODO("improve shader state switches by looking into some sort of caching");

    setShaderData(newShader, proj, layer.disableViewTransform ? glm::mat4(1.f) : view, lights, orthographic, getTintIfSelected(objectSelected), id, lightProjview, cameraPosition, layer.uniforms);

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

    if (layer.scale){
      auto transform = getTransformationFromMatrix(modelMatrix);
      auto offset = distanceToSecondFromFirst(view, modelMatrix);
      transform.scale *=  glm::tan(layer.fov / 2.0) * offset.z; // glm::tan might not be correct
      auto mat = matrixFromComponents(transform);
      glUniformMatrix4fv(glGetUniformLocation(newShader, "model"), 1, GL_FALSE, glm::value_ptr(mat));
    }else{
      glUniformMatrix4fv(glGetUniformLocation(newShader, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    }

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

    if (layer.visible){
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
        defaultMeshes,
        renderCustomObj,
        getGameObjectPos
      );
      numTriangles = numTriangles + trianglesDrawn;
    }
  
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
  
  auto maxExpectedClears = numUniqueDepthLayers(world.sandbox.layers);
  if (numDepthClears > maxExpectedClears){
    std::cout << "num clears: " << numDepthClears << std::endl;
    std::cout << "num unique depth clears: " << maxExpectedClears << std::endl;
    assert(false);
  }
  return numTriangles;
}

void renderVector(GLint shaderProgram, glm::mat4 view, glm::mat4 model){
  auto projection = projectionFromLayer(world.sandbox.layers.at(0));
  glUseProgram(shaderProgram);

  // this list is incomplete, it probably would be better to just use a separate shader maybe too
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projview"), 1, GL_FALSE, glm::value_ptr(projection * view));    
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
  glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec4(0.05, 1.f, 0.f, 1.f)));
  glUniform1i(glGetUniformLocation(shaderProgram, "hasBones"), false);    
  glUniform1f(glGetUniformLocation(shaderProgram, "discardTexAmount"), 0);  
  glUniform1i(glGetUniformLocation(shaderProgram, "hasDiffuseTexture"), false);
  glUniform1i(glGetUniformLocation(shaderProgram, "hasEmissionTexture"), false);
  glUniform1i(glGetUniformLocation(shaderProgram, "hasOpacityTexture"), false);
  glUniform1i(glGetUniformLocation(shaderProgram, "hasCubemapTexture"), false);
  glUniform1i(glGetUniformLocation(shaderProgram, "hasRoughnessTexture"), false);


  // Draw grid for the chunking logic if that is specified, else lots draw the snapping translations
  if (showDebugInfo && numChunkingGridCells > 0){
    float offset = ((numChunkingGridCells % 2) == 0) ? (dynamicLoading.mappingInfo.chunkSize / 2) : 0;
    drawGrid3D(numChunkingGridCells, dynamicLoading.mappingInfo.chunkSize, offset, offset, offset);
    glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec4(0.05, 1.f, 0.f, 1.f)));
  }

  //////////////////
  if (state.manipulatorMode == TRANSLATE && state.showGrid){
    for (auto id : selectedIds(state.editor)){
      auto selectedObj = id;
      if (selectedObj != -1){
        float snapGridSize = getSnapTranslateSize(state.easyUse);
        if (snapGridSize > 0){
          auto position = getGameObjectPosition(selectedObj, false);

          if (state.manipulatorAxis == XAXIS){
            drawGridXY(state.gridSize, state.gridSize, snapGridSize, position.x, position.y, position.z);  
          }else if (state.manipulatorAxis == YAXIS){
            drawGridXY(state.gridSize, state.gridSize, snapGridSize, position.x, position.y, position.z);  
          }else if (state.manipulatorAxis == ZAXIS){
            drawGridYZ(state.gridSize, state.gridSize, snapGridSize, position.x, position.y, position.z);  
          }else{
            drawGrid3D(state.gridSize, snapGridSize, position.x, position.y, position.z);  
          }
          glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec4(0.05, 1.f, 1.f, 1.f)));     
        }
      }
    }    
  }

  ////////////////

  if (showDebugInfo){
    drawCoordinateSystem(100.f);
      drawAllLines(lineData, shaderProgram, std::nullopt);
  
    if (state.showCameras){
      drawTraversalPositions();   
    }    
  }
}

void renderSkybox(GLint shaderProgram, glm::mat4 view, glm::vec3 cameraPosition){
  auto projection = projectionFromLayer(world.sandbox.layers.at(0));
  std::vector<LightInfo> lights = {};
  std::vector<glm::mat4> lightProjView = {};

  auto value = glm::mat3(view);  // Removes last column aka translational component --> thats why when you move skybox no move!
  RenderUniforms noUniforms = { 
    .intUniforms = {},
    .floatUniforms = {},
    .floatArrUniforms = {},
    .vec3Uniforms = {},
    .builtInUniforms = {},
  };
  setShaderData(shaderProgram, projection, value, lights, false, glm::vec3(1.f, 1.f, 1.f), 0, lightProjView, cameraPosition, noUniforms);
  glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.f)));
  glUniform4fv(glGetUniformLocation(shaderProgram, "tint"), 1, glm::value_ptr(glm::vec4(state.skyboxcolor.x, state.skyboxcolor.y, state.skyboxcolor.z, 1.f)));
  drawMesh(world.meshes.at("skybox").mesh, shaderProgram); 
}

float offsetPerLineMargin = 0.02f;
float fontOffsetPerLine(float fontsize){
  // 1000.f => 2.f height, width
  return -1 * (fontsize / 500.f + offsetPerLineMargin);
}

void renderUI(Mesh* crosshairSprite, Color pixelColor, bool showCursor){
  glUseProgram(uiShaderProgram);
  glEnable(GL_BLEND);
  glUniformMatrix4fv(glGetUniformLocation(uiShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(ndiOrtho)); 
  glUniform1i(glGetUniformLocation(uiShaderProgram, "forceTint"), false);

  if (showCursor && crosshairSprite != NULL){
    if(!state.isRotateSelection){
      glUniform4fv(glGetUniformLocation(uiShaderProgram, "tint"), 1, glm::value_ptr(glm::vec4(1.f, 1.f, 1.f, 1.f)));
      auto location = pixelCoordToNdi(glm::ivec2(state.cursorLeft, state.currentScreenHeight - state.cursorTop), glm::vec2(state.currentScreenWidth, state.currentScreenHeight));
      drawSpriteAround(uiShaderProgram, *crosshairSprite, location.x, location.y, 0.05, 0.05);
    }
  }
  if (!showDebugInfo){
    return;
  }


  float offsetPerLine = fontOffsetPerLine(state.fontsize);
  float uiYOffset = 1.f + 3 * offsetPerLine;
  float uiXOffset = -1.f - offsetPerLine;
  
  auto currentFramerate = static_cast<int>(unwrapAttr<float>(statValue(fpsStat)));
  //std::cout << "offsets: " << uiXOffset << " " << uiYOffset << std::endl;
  drawTextNdi(std::to_string(currentFramerate) + state.additionalText, uiXOffset, uiYOffset + offsetPerLine, state.fontsize + 1);

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
  drawTextNdi("manipulator axis: " + manipulatorAxisString, uiXOffset, uiYOffset + offsetPerLine * 2, state.fontsize);
  drawTextNdi("position: " + print(defaultCamera.transformation.position), uiXOffset, uiYOffset + offsetPerLine * 3, state.fontsize);
  drawTextNdi("rotation: " + print(defaultCamera.transformation.rotation), uiXOffset, uiYOffset + offsetPerLine * 4, state.fontsize);

  float ndiX = 2 * (state.cursorLeft / (float)state.resolution.x) - 1.f;
  float ndiY = -2 * (state.cursorTop / (float)state.resolution.y) + 1.f;

  drawTextNdi("cursor: (" + std::to_string(ndiX) + " | " + std::to_string(ndiY) + ") - " + std::to_string(state.cursorLeft) + " / " + std::to_string(state.cursorTop)  + "(" + std::to_string(state.resolution.x) + "||" + std::to_string(state.resolution.y) + ")", uiXOffset, uiYOffset + + offsetPerLine * 5, state.fontsize);
  
  std::string position = "n/a";
  std::string scale = "n/a";
  std::string rotation = "n/a";

  auto selectedValue = latestSelected(state.editor);
  if (selectedValue.has_value()){
    auto selectedIndex = selectedValue.value();
    auto obj = getGameObject(world, selectedIndex);
    position = print(obj.transformation.position);
    scale = print(obj.transformation.scale);
    rotation = print(obj.transformation.rotation);
  }
  drawTextNdi("position: " + position, uiXOffset, uiYOffset + offsetPerLine * 6, state.fontsize);
  drawTextNdi("scale: " + scale, uiXOffset, uiYOffset + offsetPerLine * 7, state.fontsize);
  drawTextNdi("rotation: " + rotation, uiXOffset, uiYOffset + offsetPerLine * 8, state.fontsize);
    
  drawTextNdi("pixel color: " + std::to_string(pixelColor.r) + " " + std::to_string(pixelColor.g) + " " + std::to_string(pixelColor.b), uiXOffset, uiYOffset + offsetPerLine * 9, state.fontsize);
  drawTextNdi("showing color: " + std::string(state.showBoneWeight ? "bone weight" : "bone indicies") , uiXOffset, uiYOffset + offsetPerLine * 10, state.fontsize);

  drawTextNdi(std::string("animation info: ") + (timePlayback.isPaused() ? "paused" : "playing"), uiXOffset, uiYOffset + offsetPerLine * 11, state.fontsize);
  drawTextNdi("using animation: " + std::to_string(-1) + " / " + std::to_string(-1) , uiXOffset, uiYOffset + offsetPerLine * 12, state.fontsize);
  drawTextNdi("using object id: -1" , uiXOffset, uiYOffset + offsetPerLine * 13, state.fontsize);

  drawTextNdi(std::string("triangles: ") + std::to_string(numTriangles), uiXOffset, uiYOffset + offsetPerLine * 14, state.fontsize);
  drawTextNdi(std::string("num gameobjects: ") + std::to_string(static_cast<int>(unwrapAttr<float>(statValue(numObjectsStat)))), uiXOffset, uiYOffset + offsetPerLine * 15, state.fontsize);
  drawTextNdi(std::string("num scenes loaded: ") + std::to_string(static_cast<int>(unwrapAttr<float>(statValue(scenesLoadedStat)))), uiXOffset, uiYOffset + offsetPerLine * 16, state.fontsize);
  drawTextNdi(std::string("render mode: ") + renderModeAsStr(state.renderMode), uiXOffset, uiYOffset + offsetPerLine * 17, state.fontsize);
}

void onClientMessage(std::string message){
  cBindings.onTcpMessage(message);
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
  maybeResetCamera(id);
  unsetSelectedIndex(state.editor, id, true);
  onManipulatorIdRemoved(id, removeObjectById);
}

std::map<std::string, std::string> args;
std::map<std::string, std::string> getArgs(){
  return args;
}

float exposureAmount(){
  float elapsed = now - state.exposureStart;
  float amount = elapsed / 1.f;   
  float exposureA = glm::clamp(amount, 0.f, 1.f);
  float effectiveExposure = glm::mix(state.oldExposure, state.targetExposure, exposureA);
  return effectiveExposure;
}

float getViewspaceDepth(glm::mat4& transView, objid elementId){
  auto viewPosition = transView * fullModelTransform(world.sandbox, elementId);
  return getTransformationFromMatrix(viewPosition).position.z;
}

struct RenderContext {
  World& world;
  glm::mat4 view;
  std::vector<LightInfo> lights;
  std::vector<PortalInfo> portals;
  std::vector<glm::mat4> lightProjview;
  Transformation cameraTransform;
  std::optional<glm::mat4> projection;
};


int renderWithProgram(RenderContext& context, RenderStep& renderStep){
  int triangles = 0;
  PROFILE(
  renderStep.name.c_str(),
    if (!renderStep.enable){
      std::cout << "Warning: render step not enabled: " << renderStep.name << std::endl;
      return triangles;
    }
    glUseProgram(renderStep.shader);
    setRenderUniformData(renderStep.shader, renderStep.uniforms);
    for (int i = 0; i < renderStep.textures.size(); i++){
      auto &textureData = renderStep.textures.at(i);
      int activeTextureOffset = 7 + i; // this is funny, but basically other textures before this use up to 5, probably should centralize these values
      glUniform1i(glGetUniformLocation(renderStep.shader, textureData.nameInShader.c_str()), activeTextureOffset);
      glActiveTexture(GL_TEXTURE0 + activeTextureOffset);
      if (textureData.type == RENDER_TEXTURE_REGULAR){
        glBindTexture(GL_TEXTURE_2D, world.textures.at(textureData.textureName).texture.textureId);
      }else{
        glBindTexture(GL_TEXTURE_2D, textureData.framebufferTextureId);
      }
    }
    glActiveTexture(GL_TEXTURE0);

    setActiveDepthTexture(fbo, depthTextures, renderStep.depthTextureIndex);
    glBindFramebuffer(GL_FRAMEBUFFER, renderStep.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderStep.colorAttachment0, 0);
    if (renderStep.hasColorAttachment1){
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderStep.colorAttachment1, 0);
    }

    glClearColor(0.0, 0.0, 0.0, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |  GL_STENCIL_BUFFER_BIT);

    if (state.showSkybox && renderStep.renderSkybox){
      glDepthMask(GL_FALSE);
      renderSkybox(renderStep.shader, context.view, context.cameraTransform.position);  // Probably better to render this at the end 
      glDepthMask(GL_TRUE);    
    }
    glEnable(GL_DEPTH_TEST);
    if (renderStep.blend){
      glEnable(GL_BLEND);
    }else{
      glDisable(GL_BLEND);
    }

    if (renderStep.renderQuad3D){
      std::vector<LightInfo> lights = {};
      std::vector<glm::mat4> lightProjview = {};
      RenderUniforms uniforms { };
      setShaderData(renderStep.shader, ndiOrtho, glm::mat4(1.f), lights, false, glm::vec3(1.f, 1.f, 1.f), 0, lightProjview, glm::vec3(0.f, 0.f, 0.f), uniforms);
      glActiveTexture(GL_TEXTURE0); 
      glBindTexture(GL_TEXTURE_2D, renderStep.quadTexture);
      glBindVertexArray(quadVAO3D);
      glDrawArrays(GL_TRIANGLES, 0, 6);      
    }

    if (renderStep.enableStencil){
      glEnable(GL_STENCIL_TEST);
      glStencilMask(0xFF);
      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);  
      glStencilFunc(GL_ALWAYS, 1, 0xFF);   
    }

    if (renderStep.renderWorld){
      // important - redundant call to glUseProgram
      glm::mat4* projection = context.projection.has_value() ? &context.projection.value() : NULL;
      auto worldTriangles = renderWorld(context.world, renderStep.shader, renderStep.allowShaderOverride, projection, context.view, glm::mat4(1.0f), context.lights, context.portals, context.lightProjview, context.cameraTransform.position);
      triangles += worldTriangles;
    }
    glDisable(GL_STENCIL_TEST);

    if (renderStep.renderQuad){
      glBindTexture(GL_TEXTURE_2D, renderStep.quadTexture);
      glBindVertexArray(quadVAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);      
    }
  )
  return triangles;
}

std::map<objid, unsigned int> renderPortals(RenderContext& context){
  std::map<objid, unsigned int> nextPortalCache;
  for (int i = 0; i < context.portals.size(); i++){
    auto portal = context.portals.at(i);
    auto portalViewMatrix = renderPortalView(portal, context.cameraTransform);
    renderStagesSetPortal(renderStages, i);
    RenderContext portalRenderContext {
      .world = context.world,
      .view = portalViewMatrix,
      .lights = context.lights,
      .portals = context.portals,
      .lightProjview = context.lightProjview,
      .cameraTransform = portal.cameraTransform,
      .projection = context.projection,
    };
    //std::cout << "portal transform:  " << i << " " << print(portal.cameraTransform.position) << std::endl;
    renderWithProgram(portalRenderContext, renderStages.portal);
    nextPortalCache[portal.id] = renderStages.portal.colorAttachment0;
  }
  //std::cout << std::endl;
  return nextPortalCache;
}

std::vector<glm::mat4> renderShadowMaps(RenderContext& context){
  std::vector<glm::mat4> lightMatrixs;
  for (int i = 0; i < context.lights.size(); i++){
    auto light = context.lights.at(i);
    auto lightView = renderView(light.transform.position, light.transform.rotation);
    glm::mat4 lightProjection = glm::ortho<float>(-2000, 2000,-2000, 2000, 1.f, 3000);  // need to choose these values better
    auto lightProjview = lightProjection * lightView;
    lightMatrixs.push_back(lightProjview);

    RenderContext lightRenderContext {
      .world = context.world,
      .view = lightView,
      .lights = context.lights,
      .portals = context.portals,
      .lightProjview = context.lightProjview,
      .cameraTransform = light.transform,
      .projection = lightProjection,
    };
    renderStagesSetShadowmap(renderStages, i);
    renderWithProgram(lightRenderContext, renderStages.shadowmap);
  }
  return lightMatrixs;
}

RenderStagesDofInfo getDofInfo(bool* _shouldRender){
  bool depthEnabled = false;
  float minBlurDistance = 0.f;
  float maxBlurDistance = 0.f;
  float targetDepth = 0.f;
  float nearplane = 0.1f;
  float farplane = 100.f;
  unsigned int blurAmount = 1;

  if (state.activeCameraData != NULL){
    depthEnabled = state.activeCameraData -> enableDof;
    minBlurDistance = state.activeCameraData -> minBlurDistance;
    maxBlurDistance = state.activeCameraData -> maxBlurDistance;
    blurAmount = state.activeCameraData -> blurAmount;

    if (state.activeCameraData -> target != ""){
      auto elements = getByName(world.sandbox, state.activeCameraData -> target);
      assert(elements.size() == 1);
      auto elementId = elements.at(0);
      auto halfBlurDistance = (maxBlurDistance - minBlurDistance) * 0.5f;
      targetDepth = -1 * getViewspaceDepth(view, elementId);
      minBlurDistance = targetDepth - halfBlurDistance;
      maxBlurDistance = targetDepth + halfBlurDistance;
      //std::cout << "dof info: (" << minBlurDistance << " " << maxBlurDistance << " " << targetDepth << ")" << std::endl;
      auto layerName = getGameObject(world, elementId).layer;
      auto targetObjLayer = layerByName(layerName);
      nearplane = targetObjLayer.nearplane;
      farplane = targetObjLayer.farplane;
    }
  }
  *_shouldRender = depthEnabled;
  RenderStagesDofInfo info {
    .blurAmount = blurAmount,
    .minBlurDistance = minBlurDistance,
    .maxBlurDistance = maxBlurDistance,
    .nearplane = nearplane,
    .farplane = farplane,
  };  
  return info;
}

objid addLineNextCycle(glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner, std::optional<glm::vec4> color, std::optional<unsigned int> textureId, std::optional<unsigned int> linewidth){
  return addLineToNextCycleTint(lineData, fromPos, toPos, permaline, owner, color, textureId, linewidth);
}

objid addLineNextCycle(glm::vec3 fromPos, glm::vec3 toPos, bool permaline, objid owner){
  return addLineToNextCycle(lineData, fromPos, toPos, permaline, owner, GREEN, std::nullopt);
}

void freeLine(objid lineId){
  freeLine(lineData, lineId);
}

void onGLFWEerror(int error, const char* description){
  std::cerr << "Error: " << description << std::endl;
}


GLFWwindow* window = NULL;
GLFWmonitor* monitor = NULL;
const GLFWvidmode* mode = NULL;

int main(int argc, char* argv[]){
  signal(SIGABRT, signalHandler);  

  cxxopts::Options cxxoption("ModEngine", "ModEngine is a game engine for hardcore fps");
  cxxoption.add_options()
   ("s,shader", "Folder path of default shader", cxxopts::value<std::string>()->default_value("./res/shaders/default"))
   ("t,texture", "Additional textures folder to use", cxxopts::value<std::string>()->default_value("./res"))
   ("x,scriptpath", "Script file to use", cxxopts::value<std::vector<std::string>>()->default_value(""))
   ("u,uishader", "Shader to use for ui", cxxopts::value<std::string>()->default_value("./res/shaders/ui"))
   ("c,camera", "Camera to use after initial load", cxxopts::value<std::string>()->default_value(""))
   ("fps", "Framerate limit", cxxopts::value<int>()->default_value("0"))
   ("fps-fixed", "Whether to guarantee the framerate, which means values do not occur in realtime", cxxopts::value<bool>()->default_value("false"))
   ("fps-lag", "Extra lag to induce in each frame in ms", cxxopts::value<int>()->default_value("-1"))
   ("fps-speed", "Fps speed multiplier", cxxopts::value<int>()->default_value("1000"))
   ("f,fullscreen", "Enable fullscreen mode", cxxopts::value<bool>()->default_value("false"))
   ("i,info", "Show debug info", cxxopts::value<bool>()->default_value("false"))
   ("cursor", "Show cursor", cxxopts::value<bool>() -> default_value("true"))
   ("k,skiploop", "Skip main game loop", cxxopts::value<bool>()->default_value("false"))
   ("d,dumpphysics", "Dump physics info to file for external processing", cxxopts::value<bool>()->default_value("false"))
   ("b,bootstrapper", "Run the server in bootstrapper only", cxxopts::value<bool>()->default_value("false"))
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
   ("z,layers", "Layers file to specify render layers", cxxopts::value<std::string>() -> default_value("./res/layers.layerinfo"))
   ("test-unit", "Run unit tests", cxxopts::value<bool>()->default_value("false"))
   ("rechunk", "Rechunk the world", cxxopts::value<int>()->default_value("0"))
   ("mods", "List of mod folders", cxxopts::value<std::vector<std::string>>() -> default_value(""))
   ("font", "Default font to use", cxxopts::value<std::vector<std::string>>()->default_value("./res/textures/fonts/gamefont"))
   ("log", "List of logs to display", cxxopts::value<std::vector<std::string>>() -> default_value(""))
   ("loglevel", "Log level", cxxopts::value<int>()->default_value("0"))
   ("h,help", "Print help")
  ;        

  const auto result = cxxoption.parse(argc, argv);

  auto levels = result["log"].as<std::vector<std::string>>();
  modlogSetEnabled(levels.size() > 0, static_cast<MODLOG_LEVEL>(result["loglevel"].as<int>()), levels);

  auto runUnitTests = result["test-unit"].as<bool>();
  if (runUnitTests){
    auto returnVal = runTests();
    exit(returnVal);
  }

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

  interface = SysInterface {
    .loadCScript = [](std::string script, objid id, objid sceneId) -> void {
      //    .onCreateCustomElement = createCustomObj,
      /*  if (script == "native/basic_test"){
     return;
       }
      auto name = getGameObject(world, id).name;
      std::cout << "gameobj: " << name << " wants to load script: (" << script << ")" << std::endl;
     loadScript(script, id, sceneId, bootStrapperMode, false /*freescript*/ ;
      loadCScript(id, script.c_str(), sceneId, bootStrapperMode, false);
      /*  /*if (script == "native/basic_test"){
        return;
      }
       auto name = getGameObject(world, id).name;
      std::cout << "gameobj: " << name << " wants to load script: (" << script << ")" << std::endl;
      loadScript(script, id, sceneId, bootStrapperMode, false);*/
    },
    .unloadCScript = [](std::string scriptpath, objid id) -> void {
      unloadCScript(id);
      removeLocks(id);
      removeLinesByOwner(lineData, id);
    },
    .stopAnimation = stopAnimation,
    .getCurrentTime = getTotalTime,
    .readFile = modlayerReadFile,
    .modlayerPath = modlayerPath,
    .fontFamilyByName = fontFamilyByName,
  };

  auto mods = result["mods"].as<std::vector<std::string>>();
  for (auto mod : mods){
    installMod(mod);
  }

  auto layers = parseLayerInfo(result["layers"].as<std::string>(), interface.readFile);

  auto rawScenes = result["rawscene"].as<std::vector<std::string>>();
  rawSceneFile =  rawScenes.size() > 0 ? rawScenes.at(0) : "./res/scenes/example.rawscene";

  keyMapper = readMapping(result["mapping"].as<std::string>(), inputFns, interface.readFile);

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
  showCursor = result["cursor"].as<bool>() || showDebugInfo;
  
  auto benchmarkFile = result["benchmark"].as<std::string>();
  auto shouldBenchmark = benchmarkFile != "";
  auto timetoexit = result["timetoexit"].as<int>();

  benchmark = createBenchmark(shouldBenchmark);

  std::cout << "LIFECYCLE: program starting" << std::endl;
  disableInput = result["noinput"].as<bool>();

  state.fullscreen = result["fullscreen"].as<bool>(); // merge flags and world.state concept
  setInitialState(state, "./res/world.state", now, interface.readFile); 

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //glfwWindowHint(GLFW_DECORATED, false);
  glfwSetErrorCallback(onGLFWEerror);

  monitor = glfwGetPrimaryMonitor();
  mode = glfwGetVideoMode(monitor);
  state.currentScreenWidth = mode -> width;
  state.currentScreenHeight = mode -> height;
  window = glfwCreateWindow(state.currentScreenWidth, state.currentScreenHeight, state.windowname.c_str(), NULL, NULL);

  if (window == NULL){
    std::cerr << "ERROR: failed to create window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  if (headlessmode){
    glfwHideWindow(window);
  }

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
     std::cerr << "ERROR: failed to load opengl functions" << std::endl;
     glfwTerminate();
     return -1;
  }

  startSoundSystem();

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);  

  genFramebufferTexture(&framebufferTexture, state.resolution.x, state.resolution.y);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture, 0);

  genFramebufferTexture(&framebufferTexture2, state.resolution.x, state.resolution.y);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, framebufferTexture2, 0);

  genFramebufferTexture(&framebufferTexture3, state.resolution.x, state.resolution.y);
  //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, framebufferTexture3, 0);

  generateDepthTextures(depthTextures, numDepthTextures, state.resolution.x, state.resolution.y);
  generateDepthTextures(textureDepthTextures, 1, state.resolution.x, state.resolution.y);

  generatePortalTextures(portalTextures, numPortalTextures, state.resolution.x, state.resolution.y);
  setActiveDepthTexture(fbo, depthTextures, 0);

  if (!glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE){
    std::cerr << "ERROR: framebuffer incomplete" << std::endl;
    return -1;
  }

  quadVAO = loadFullscreenQuadVAO();
  quadVAO3D = loadFullscreenQuadVAO3D();
  //////////////////////////////

  auto onFramebufferSizeChange = [](GLFWwindow* window, int width, int height) -> void {
     std::cout << "EVENT: framebuffer resized:  new size-  " << "width("<< width << ")" << " height(" << height << ")" << std::endl;
     state.currentScreenWidth = width;
     state.currentScreenHeight = height;
     if (state.nativeViewport){
       state.viewportSize = glm::ivec2(width, height);
     }
     if (state.nativeResolution){
       state.resolution = glm::ivec2(width, height);
     }

     glBindTexture(GL_TEXTURE_2D, framebufferTexture);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, state.resolution.x, state.resolution.y, 0, GL_RGBA, GL_FLOAT, NULL);

     glBindTexture(GL_TEXTURE_2D, framebufferTexture2);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, state.resolution.x, state.resolution.y, 0, GL_RGBA, GL_FLOAT, NULL);

     glBindTexture(GL_TEXTURE_2D, framebufferTexture3);
     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, state.resolution.x, state.resolution.y, 0, GL_RGBA, GL_FLOAT, NULL);

     updateDepthTexturesSize(depthTextures, numDepthTextures, state.resolution.x, state.resolution.y);
     updatePortalTexturesSize(portalTextures, numPortalTextures, state.resolution.x, state.resolution.y);

     orthoProj = glm::ortho(0.0f, (float)state.currentScreenWidth, 0.0f, (float)state.currentScreenHeight, -1.0f, 1.0f);  
  }; 

  onFramebufferSizeChange(window, state.currentScreenWidth, state.currentScreenHeight);
  glfwSetFramebufferSizeCallback(window, onFramebufferSizeChange); 
  glPointSize(10.f);

  modlog("shaders", std::string("shader file path is ") + shaderFolderPath);
  unsigned int shaderProgram = loadShader(shaderFolderPath + "/vertex.glsl", shaderFolderPath + "/fragment.glsl", interface.readFile);
  
  modlog("shaders", std::string("framebuffer file path is ") + framebufferShaderPath);
  framebufferProgram = loadShader(framebufferShaderPath + "/vertex.glsl", framebufferShaderPath + "/fragment.glsl", interface.readFile);

  std::string depthShaderPath = "./res/shaders/depth";
  modlog("shaders", std::string("depth file path is ") + depthShaderPath);
  unsigned int depthProgram = loadShader(depthShaderPath + "/vertex.glsl", depthShaderPath + "/fragment.glsl", interface.readFile);

  modlog("shaders", std::string("ui shader file path is ") + uiShaderPath);
  uiShaderProgram = loadShader(uiShaderPath + "/vertex.glsl",  uiShaderPath + "/fragment.glsl", interface.readFile);

  std::string selectionShaderPath = "./res/shaders/selection";
  modlog("shaders", std::string("selection shader path is ") + selectionShaderPath);
  unsigned int selectionProgram = loadShader(selectionShaderPath + "/vertex.glsl", selectionShaderPath + "/fragment.glsl", interface.readFile);

  std::string drawingShaderPath = "./res/shaders/drawing";
  modlog("shaders", std::string("drawing shader path is: ") + drawingShaderPath);
  drawingProgram = loadShader(drawingShaderPath + "/vertex.glsl", drawingShaderPath + "/fragment.glsl", interface.readFile);

  std::string blurShaderPath = "./res/shaders/blur";
  modlog("shaders", std::string("blur shader path is: ") + blurShaderPath);
  unsigned int blurProgram = loadShader(blurShaderPath + "/vertex.glsl", blurShaderPath + "/fragment.glsl", interface.readFile);

  std::string basicShaderPath = "./res/shaders/basic";
  modlog("shaders", std::string("basic shader path is: ") + basicShaderPath);
  unsigned int basicProgram = loadShader(basicShaderPath + "/vertex.glsl", basicShaderPath+ "/fragment.glsl", interface.readFile);

  renderStages = loadRenderStages(fbo, 
    framebufferTexture, framebufferTexture2, framebufferTexture3, 
    depthTextures, numDepthTextures,
    portalTextures, numPortalTextures,
    RenderShaders {
      .blurProgram = blurProgram,
      .selectionProgram = selectionProgram,
      .uiShaderProgram = uiShaderProgram,
      .shaderProgram = shaderProgram,
      .basicProgram = basicProgram,
    },
    interface.readFile
  );

  CustomApiBindings pluginApi{
    .listSceneId = listSceneId,
    .loadScene = loadScene,
    .unloadScene = unloadScene,
    .unloadAllScenes = unloadAllScenes,
    .listScenes = listScenes,
    .listSceneFiles = listSceneFiles,
    .parentScene = parentScene,
    .childScenes = childScenes,
    .sceneIdByName = sceneIdByName,
    .rootIdForScene = rootIdForScene,
    .scenegraph = scenegraph,
    .sendLoadScene = sendLoadScene,
    .createScene = createScene,
    .deleteScene = deleteScene,
    .moveCamera = moveCamera,
    .rotateCamera = rotateCamera,
    .removeObjectById = removeObjectById,
    .getObjectsByType = getObjectsByType,
    .getObjectsByAttr = getObjectsByAttr,
    .setActiveCamera = setActiveCamera,
    .drawText = drawText,
    .drawLine = addLineNextCycle,
    .freeLine = freeLine,
    .getGameObjectNameForId = getGameObjectName,
    .getGameObjectAttr = getGameObjectAttr,
    .setGameObjectAttr = setGameObjectAttr,
    .getGameObjectPos = getGameObjectPosition,
    .setGameObjectPos = setGameObjectPosition,
    .setGameObjectPosRelative = setGameObjectPositionRelative,
    .getGameObjectRotation = getGameObjectRotation,
    .setGameObjectRot = setGameObjectRotationRelative,
    .setFrontDelta = setFrontDelta,
    .moveRelative = moveRelative,
    .moveRelativeVec = moveRelative,
    .orientationFromPos = orientationFromPos,
    .getGameObjectByName = getGameObjectByName,
    .applyImpulse = applyImpulse,
    .applyImpulseRel = applyImpulseRel,
    .clearImpulse = clearImpulse,
    .listAnimations = listAnimations,
    .playAnimation = playAnimation,
    .listClips = listSounds,
    .playClip = playSoundState,
    .listModels = listModels,
    .sendNotifyMessage = sendNotifyMessage,
    .timeSeconds = timeSeconds,
    .timeElapsed = timeElapsed,
    .saveScene = saveScene,
    .listServers = listServers,
    .connectServer = connectServer,
    .disconnectServer = disconnectServer,
    .sendMessageTcp = sendMessageToActiveServer,
    .sendMessageUdp = sendDataUdp,
    .playRecording = playRecording,
    .stopRecording = stopRecording,
    .createRecording = createRecording,
    .saveRecording = saveRecording,
    .makeObjectAttr = makeObjectAttr,
    .makeParent = makeParent,
    .raycast = raycastW,
    .saveScreenshot = takeScreenshot,
    .setState = setState,
    .setFloatState = setFloatState,
    .setIntState = setIntState,
    .navPosition = navPosition,
    .emit = emit,
    .loadAround = addLoadingAround,
    .rmLoadAround = removeLoadingAround,
    .generateMesh = createGeneratedMesh,
    .getArgs = getArgs,
    .lock = lock,
    .unlock = unlock,
    .debugInfo = debugInfo,
    .setWorldState = setWorldState,
    .setLayerState = setLayerState,
    .enforceLayout = enforceLayout,
    .createTexture = createTexture,
    .freeTexture = freeTexture,
    .clearTexture = clearTexture,
    .runStats = statValue,
    .stat = statName,
    .logStat = registerStat,
    .installMod = installMod,
    .uninstallMod = uninstallMod,
    .listMods = listMods,
  };
  registerAllBindings({ sampleBindingPlugin(pluginApi), cscriptSchemeBinding(pluginApi, interface.modlayerPath) });

  cBindings = getCScriptBindingCallbacks();

  BulletDebugDrawer drawer(addLineNextCycle);
  btIDebugDraw* debuggerDrawer = result["debugphysics"].as<bool>() ?  &drawer : NULL;


  if(bootStrapperMode){
    netcode = initNetCode(cBindings.onPlayerJoined, cBindings.onPlayerLeave, interface.readFile);
  }


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

  std::vector<std::string> allTexturesToLoad = {  "./res/textures/crosshairs/crosshair029.png", "./res/textures/crosshairs/crosshair008.png" };
  for (auto &layer : layers){
    if (layer.cursor == ""){
      continue;
    }
    if (layer.cursor == "none"){
      continue;
    }
    for (auto &texture : allTexturesToLoad){
      if (layer.cursor == texture){
        continue;
      }
    }
    allTexturesToLoad.push_back(layer.cursor);
  }


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
      onObjDelete(id);
      netObjectDelete(id, isNet, netcode, bootStrapperMode);
    }, 
    debuggerDrawer, 
    layers,
    interface,
    defaultMeshesToLoad,
    allTexturesToLoad
  );

  auto fontPaths = result["font"].as<std::vector<std::string>>();
  std::cout << "INFO: FONT: loading font paths (" << fontPaths.size() <<") - ";
  for (auto &fontPath : fontPaths){
    std::cout << fontPath << " ";
  }
  std::cout << std::endl;
   // this texture used for default textures, could make font mesh texture optional or something
  fontFamily = loadFontMeshes(readFontFile(fontPaths), world.textures.at("./res/textures/wood.jpg").texture);

  defaultCrosshairSprite = &world.meshes.at("./res/textures/crosshairs/crosshair008.png").mesh;
  setCrosshairSprite();  // needs to be after create world since depends on these meshes being loaded

  if (state.skybox != ""){
    loadSkybox(world, state.skybox); 
  }

  for (auto script : result["scriptpath"].as<std::vector<std::string>>()){
    loadCScript(getUniqueObjId(), script.c_str(), -1, bootStrapperMode, true);
  }

  bool fpsFixed = result["fps-fixed"].as<bool>();
  initialTime = fpsFixed  ? 0 : glfwGetTime();

  timings = createWorldTiming(initialTime);
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
  loadTextureWorld(world, "./res/models/box/grid.png", -1);

  GLFWimage images[1]; 
  images[0].pixels = stbi_load(state.iconpath.c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels 
  glfwSetWindowIcon(window, 1, images);
  stbi_image_free(images[0].pixels); 

  dynamicLoading = createDynamicLoading(worldfile, interface.readFile);
  if (result["rechunk"].as<int>()){
    rechunkAllObjects(world, dynamicLoading, result["rechunk"].as<int>());
    return 0;
  }

  std::cout << "INFO: # of intitial raw scenes: " << rawScenes.size() << std::endl;
  for (auto rawScene : rawScenes){
    loadScene(rawScene, {}, std::nullopt);
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
  glfwSwapInterval(state.swapInterval);
  toggleFullScreen(state.fullscreen);
  toggleCursor(state.captureCursor); 

  unsigned int frameCount = 0;
  float previous = now;
  float last60 = previous;

  std::cout << "INFO: render loop starting" << std::endl;

  GLenum buffers_to_render[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, buffers_to_render);

  int frameratelimit = result["fps"].as<int>();
  bool hasFramelimit = frameratelimit != 0;
  float minDeltaTime = !hasFramelimit ? 0 : (1.f / frameratelimit);

  float fixedFps = 60.f;
  float fixedDelta = 1.f / fixedFps;
  float fpsLag = (result["fps-lag"].as<int>()) / 1000.f;
  long long totalFrames = 0;
  float speedMultiplier = result["fps-speed"].as<int>() / 1000.f;
  std::cout << "speed multiplier: "  << speedMultiplier << std::endl;

  assert(!hasFramelimit || !fpsFixed);
  assert(fpsLag < 0 || !fpsFixed);
  assert(!hasFramelimit || speedMultiplier == 1000);
  assert(fpsLag < 0 || speedMultiplier == 1000);

  if (result["skiploop"].as<bool>()){
    goto cleanup;
  }

  state.cullEnabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  setShouldProfile(shouldBenchmark);

  PROFILE("MAINLOOP",
  while (!glfwWindowShouldClose(window)){
  PROFILE("FRAME",
    frameCount++;
    totalFrames++;

    fpscountstart:
    now = fpsFixed ? (fixedDelta * (totalFrames - 1)) :  (speedMultiplier * glfwGetTime());
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
    if (deltaTime < fpsLag){
      goto fpscountstart; 
    }

    previous = now;

    int numObjects = getNumberOfObjects(world.sandbox);
    registerStat(numObjectsStat, numObjects);
    registerStat(scenesLoadedStat, getNumberScenesLoaded(world.sandbox));
    logBenchmarkTick(benchmark, deltaTime, numObjects, numTriangles);

    if (!state.pauseWorldTiming){
      timePlayback.setElapsedTime(deltaTime);
    }

    if (frameCount == 60){
      frameCount = 0;
      float timedelta = now - last60;
      last60 = now;
      registerStat(fpsStat, floor((60.f/(timedelta) + 0.5f)));
    }

    onWorldFrame(world, deltaTime, getTotalTime(), enablePhysics, dumpPhysics);

    auto time = getTotalTime();
    tickRecordings(time);

    onNetCode(world, netcode, onClientMessage, bootStrapperMode);

    auto viewTransform = getCameraTransform();

    auto forward = calculateRelativeOffset(viewTransform.rotation, {0, 0, -1 }, false);
    auto up  = calculateRelativeOffset(viewTransform.rotation, {0, 1, 0 }, false);
    setListenerPosition(
      viewTransform.position.x, viewTransform.position.y, viewTransform.position.z,
      { forward.x, forward.y, forward.z},
      { up.x, up.y, up.z }
    );
    
    view = renderView(viewTransform.position, viewTransform.rotation);
    glViewport(0, 0, state.resolution.x, state.resolution.y);

    std::vector<LightInfo> lights = getLightInfo(world);
    std::vector<PortalInfo> portals = getPortalInfo(world);
    assert(portals.size() <= numPortalTextures);

    // depth buffer from point of view SMf 1 light source (all eventually, but 1 for now)

    RenderContext renderContext {
      .world = world,
      .view = view,
      .lights = lights,
      .portals = portals,
      .lightProjview = {},
      .cameraTransform = viewTransform,
      .projection = std::nullopt,
    };

    std::vector<glm::mat4> lightMatrixs;
    PROFILE(
      "RENDERING-SHADOWMAPS",
      lightMatrixs = renderShadowMaps(renderContext);
    )

    renderContext.lightProjview = lightMatrixs;


    bool depthEnabled = false;
    auto dofInfo = getDofInfo(&depthEnabled);
    updateRenderStages(renderStages, dofInfo);
    // outputs to FBO unique colors based upon ids. This eventually passed in encodedid to all the shaders which is how color is determined
    renderWithProgram(renderContext, renderStages.selection);

    //std::cout << "cursor pos: " << state.cursorLeft << " " << state.cursorTop << std::endl;
    auto adjustedCoords = pixelCoordsRelativeToViewport(state.cursorLeft, state.cursorTop, state.currentScreenHeight, state.viewportSize, state.viewportoffset, state.resolution);
    //std::cout << "adjusted coords: " << print(adjustedCoords) << std::endl;
    auto uvCoordWithTex = getUVCoordAndTextureId(adjustedCoords.x, adjustedCoords.y);
    auto uvCoord = toUvCoord(uvCoordWithTex);
    Color hoveredItemColor = getPixelColor(adjustedCoords.x, adjustedCoords.y);
    auto hoveredId = getIdFromColor(hoveredItemColor);


    state.lastHoveredIdInScene = state.hoveredIdInScene;
    state.hoveredIdInScene = idExists(world.sandbox, hoveredId);
    state.lastHoverIndex = state.currentHoverIndex;
    state.currentHoverIndex = hoveredId;
    state.additionalText = "     <" + std::to_string((int)(255 * hoveredItemColor.r)) + ","  + std::to_string((int)(255 * hoveredItemColor.g)) + " , " + std::to_string((int)(255 * hoveredItemColor.b)) + ">  " + " --- " + state.selectedName;

    bool selectItemCalledThisFrame = selectItemCalled;

    auto selectTargetId = state.editor.forceSelectIndex == 0 ? hoveredId : state.editor.forceSelectIndex;;
    auto shouldSelectItem = selectItemCalled || (state.editor.forceSelectIndex != 0);
    state.editor.forceSelectIndex = 0;

    if ((selectTargetId != getManipulatorId()) && shouldSelectItem){
      std::cout << "INFO: select item called" << std::endl;

      std::cout << "select target id: " << selectTargetId << std::endl;
      if (idExists(world.sandbox, selectTargetId)){
        std::cout << "INFO: select item called -> id in scene!" << std::endl;
        auto layerSelectIndex = getLayerForId(selectTargetId).selectIndex;

        auto layerSelectNegOne = layerSelectIndex == -1;
        auto layerSelectThreeCond = layerSelectIndex == -3 && mappingClickCalled;
        std::cout << "cond1 = " << (layerSelectNegOne ? "true" : "false") << ", condtwo = " << (layerSelectThreeCond ? "true" : "false") << ", selectindex " << layerSelectIndex << ", mapping = " << mappingClickCalled << std::endl;
        if (!(layerSelectNegOne || layerSelectThreeCond)){
          selectItem(selectTargetId, layerSelectIndex);
        }
      }else{
        std::cout << "INFO: select item called -> id not in scene! - " << selectTargetId<< std::endl;
        cBindings.onObjectUnselected();
      }
      selectItemCalled = false;
    }
    auto ndiCoords = ndiCoord();
    if (state.editor.activeObj != 0){
      applyUICoord(
        world.objectMapping,
        [&adjustedCoords](glm::vec2 coord) -> glm::vec2 {
          auto pixelCoord = ndiToPixelCoord(coord, state.resolution);
          auto uv = getUVCoord(pixelCoord.x, pixelCoord.y);
          //std::cout << "get uv coord: " << print(coord) << " - " << print(pixelCoord) << " adjusted: " << print(adjustedCoords) <<  std::endl;
          return glm::vec2(uv.x, uv.y);
        },
        [](glm::vec2 coord) -> objid {
          auto pixelCoord = ndiToPixelCoord(coord, state.resolution);
          auto id = getIdFromPixelCoord(pixelCoord.x, pixelCoord.y);
          if (id < 0){
            return -1;
          }
          return id;
        },
        [](objid id) -> glm::quat {
          return getGameObjectRotation(id, true);
        },
        [](std::string topic, std::string value) -> void { 
          StringString message {
            .strTopic = topic,
            .strValue = value,
          };
          channelMessages.push(message);
        }, 
        state.editor.activeObj,
        hoveredId < 0 ? -1 : hoveredId,
        selectItemCalledThisFrame,
        uvCoord.x, 
        uvCoord.y,
        ndiCoords.x, 
        ndiCoords.y
      );
    }

    std::string cursorForLayer("./res/textures/crosshairs/crosshair008.png");
    if (state.hoveredIdInScene){
      auto hoveredLayer = getLayerForId(hoveredId);
      if (hoveredLayer.cursor != ""){
        cursorForLayer = hoveredLayer.cursor;
      }
    }
    if (cursorForLayer == "none"){
      defaultCrosshairSprite = NULL;
    }else{
      defaultCrosshairSprite = &world.meshes.at(cursorForLayer).mesh;
    }
    

    onManipulatorUpdate(
      onManipulatorSelected, 
      [](glm::vec3 frompos, glm::vec3 topos, LineColor color) -> void {
        if (state.manipulatorLineId == 0){
          state.manipulatorLineId = getUniqueObjId();
        }
        addLineToNextCycle(lineData, frompos, topos, true, state.manipulatorLineId, color, std::nullopt);
      },
      []() -> void {
        removeLinesByOwner(lineData, state.manipulatorLineId);
      },
      getGameObjectPos, 
      setGameObjectPosition, 
      getGameObjectScale,
      setGameObjectScale,
      getGameObjectRotationRelative,
      setGameObjectRotationRelative,
      projectionFromLayer(layers.at(0)),
      view, 
      state.manipulatorMode, 
      state.offsetX, 
      state.offsetY,
      glm::vec2(adjustedCoords.x, adjustedCoords.y),
      glm::vec2(state.resolution.x, state.resolution.y),
      [&state](glm::vec3 pos) -> glm::vec3 {
        return snapTranslate(state.easyUse, pos);
      },
      [&state](glm::vec3 scale) -> glm::vec3 {
        return snapScale(state.easyUse, scale);
      },
      [&state](glm::quat rot, Axis snapAxis) -> glm::quat {
        return snapRotate(state.easyUse, rot, snapAxis);
      },
      ManipulatorOptions {
         .snapManipulatorPositions = true,
         .snapManipulatorScales = true,
         .snapManipulatorAngles = true,
         .rotateSnapRelative = true,
         .preserveRelativeScale = false,
      }
    );

    ///////////////////
    auto textureId = uvCoordWithTex.z;
    std::optional<std::string> textureName = textureId > 0 ? getTextureById(world, textureId) : std::nullopt;

    bool selectedMappingTexture = false;
    if (textureName.has_value()){
      //std::cout << "texturename: " << textureName << std::endl;
      auto mappingTexture = getMappingTexture(world, textureName.value());
      if (mappingTexture.has_value()){
        auto mappingTextureName = getTextureById(world, mappingTexture.value()).value();
        renderStages.basicTexture.quadTexture = mappingTexture.value();
        renderWithProgram(renderContext, renderStages.basicTexture);
        auto pixelCoord = uvToPixelCoord(uvCoord, state.resolution);
        Color colorFromSelection2 = getPixelColor(pixelCoord.x, pixelCoord.y);
        auto hoveredColorItemId = getIdFromColor(colorFromSelection2);
        if (hoveredColorItemId > 0){
          if (selectItemCalledThisFrame){
            cBindings.onMapping(hoveredColorItemId);
            selectedMappingTexture = true;
          }
        }
      }    
    }
    mappingClickCalled = selectedMappingTexture;
    ///////////////////////

    // Each portal requires a render pass  -- // look misplaced and unneccessary 
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glEnable(GL_BLEND);
    /////////////////////

    handlePaintingModifiesViewport(uvCoord);


    glViewport(0, 0, state.resolution.x, state.resolution.y);
    handleTerrainPainting(uvCoord);
     
    if (useChunkingSystem){
      handleChunkLoading(
        dynamicLoading, 
        [](objid id) -> glm::vec3 { 
          return getGameObjectPosition(id, true);
        }, 
        loadSceneParentOffset, 
        removeObjectById,
        state.useDefaultCamera ? &viewTransform.position : NULL
      );
    }

    assert(portals.size() <= numPortalTextures);

    PROFILE("PORTAL_RENDERING", 
      portalIdCache = renderPortals(renderContext);
    )


    ////////////////////////////

    numTriangles = renderWithProgram(renderContext, renderStages.main);
    Color colorFromSelection = getPixelColor(adjustedCoords.x, adjustedCoords.y);

      /////////////////

    Color pixelColor = getPixelColor(adjustedCoords.x, adjustedCoords.y);
    if (shouldCallItemSelected){
      auto selectedId = latestSelected(state.editor);
      if (selectedId.has_value()){
        cBindings.onObjectSelected(selectedId.value(), glm::vec3(pixelColor.r, pixelColor.g, pixelColor.b));
      }
      shouldCallItemSelected = false;
    }

    if (state.lastHoverIndex != state.currentHoverIndex){
      if (state.lastHoveredIdInScene){
        cBindings.onObjectHover(state.lastHoverIndex, false);
      }
      if (state.hoveredIdInScene){
        cBindings.onObjectHover(state.currentHoverIndex, true);
      }
    }

    handleInput(window);
    glfwPollEvents();
    
    cBindings.onFrame();
    while (!channelMessages.empty()){
      auto message = channelMessages.front();
      channelMessages.pop();
      cBindings.onMessage(message.strTopic, message.strValue);
    }

    renderVector(shaderProgram, view, glm::mat4(1.0f));
        
    portalIdCache.clear();
 

    PROFILE("BLOOM-RENDERING",
      renderWithProgram(renderContext, renderStages.bloom1);
      renderWithProgram(renderContext, renderStages.bloom2);
    )
    if (depthEnabled){
      PROFILE("DOF-RENDERING",
        renderWithProgram(renderContext, renderStages.dof1);
        renderWithProgram(renderContext, renderStages.dof2);
      )
    }
    for (auto &renderStep : renderStages.additionalRenderSteps){ // probably should be the final render
      renderWithProgram(renderContext, renderStep);
    }

    auto screenspaceTextureIds = textureIdsToRender();
    for (auto userTexture : screenspaceTextureIds){
      Texture tex {
        .textureId = userTexture.id,
      };
      Texture tex2 {
        .textureId = userTexture.selectionTextureId,
      };
      renderScreenspaceLines(tex, tex2, userTexture.shouldClear || userTexture.autoclear, userTexture.clearColor, userTexture.clearTextureId, false);
    }
    markUserTexturesCleared();



    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    auto finalProgram = (state.renderMode == RENDER_DEPTH) ? depthProgram : framebufferProgram;
    glUseProgram(finalProgram); 
    glClearColor(0.f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniform1i(glGetUniformLocation(finalProgram, "enableBloom"), state.enableBloom);
    glUniform1i(glGetUniformLocation(finalProgram, "enableFog"), state.enableFog);
    glUniform4fv(glGetUniformLocation(finalProgram, "fogColor"), 1, glm::value_ptr(state.fogColor));
    glUniform1f(glGetUniformLocation(finalProgram, "near"), 0.1);
    glUniform1f(glGetUniformLocation(finalProgram, "far"), 100);
    glUniform1f(glGetUniformLocation(finalProgram, "mincutoff"), 0.5);
    glUniform1f(glGetUniformLocation(finalProgram, "maxcuttoff"), 1.1f);

    state.exposure = exposureAmount();
    glUniform1f(glGetUniformLocation(finalProgram, "exposure"), state.exposure);

    glUniform1f(glGetUniformLocation(finalProgram, "bloomAmount"), state.bloomAmount);
    glUniform1i(glGetUniformLocation(finalProgram, "bloomTexture"), 1);
    glUniform1i(glGetUniformLocation(finalProgram, "depthTexture"), 2);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthTextures[0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, framebufferTexture2);

    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(finalProgram, "framebufferTexture"), 0);

    //  Border rendering
    if (state.borderTexture != ""){
      glViewport(0, 0, state.currentScreenWidth, state.currentScreenHeight);
      glBindTexture(GL_TEXTURE_2D, world.textures.at(state.borderTexture).texture.textureId);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glClear(GL_DEPTH_BUFFER_BIT);
    }
    //////////////////////////////////////////////////////////

    if (state.renderMode == RENDER_FINAL){
      glBindTexture(GL_TEXTURE_2D, finalRenderingTexture(renderStages));
    }else if (state.renderMode == RENDER_PORTAL){
      assert(state.textureIndex <= numPortalTextures && state.textureIndex >= 0);
      glBindTexture(GL_TEXTURE_2D, portalTextures[state.textureIndex]);  
    }else if (state.renderMode == RENDER_PAINT){
      //glBindTexture(GL_TEXTURE_2D, textureToPaint);
      glBindTexture(GL_TEXTURE_2D, world.textures.at("gentexture-scenegraph_seletion_texture").texture.textureId);
    }else if (state.renderMode == RENDER_DEPTH){
      assert(state.textureIndex <=  numDepthTextures && state.textureIndex >= 0);
      glBindTexture(GL_TEXTURE_2D, depthTextures[state.textureIndex]);
    }else if (state.renderMode == RENDER_BLOOM){
      glBindTexture(GL_TEXTURE_2D, framebufferTexture2);
    }else if (state.renderMode == RENDER_GRAPHS){
      if (screenspaceTextureIds.size() > state.textureIndex && state.textureIndex >= 0){
        glBindTexture(GL_TEXTURE_2D, screenspaceTextureIds.at(state.textureIndex).id);
      }else{
        modlog("rendering", (std::string("cannot display graph texture index: ") + std::to_string(state.textureIndex)).c_str());
      }
    }
    glViewport(state.viewportoffset.x, state.viewportoffset.y, state.viewportSize.x, state.viewportSize.y);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, state.currentScreenWidth, state.currentScreenHeight);

    Mesh* effectiveCrosshair = defaultCrosshairSprite;
    if (crosshairSprite != NULL){
      effectiveCrosshair = crosshairSprite;
    }
    renderUI(effectiveCrosshair, pixelColor, showCursor);
    drawTextData(lineData, uiShaderProgram, fontFamilyByName, std::nullopt,  state.currentScreenHeight, state.currentScreenWidth);
    disposeTempBufferedData(lineData);
    glEnable(GL_DEPTH_TEST);

    if (state.takeScreenshot){
      state.takeScreenshot = false;
      saveScreenshot(state.screenshotPath);
    }

    glfwSwapBuffers(window);
  )})

  modlog("lifecycle", "program exiting");

  cleanup:   
    if (shouldBenchmark){
      saveFile(benchmarkFile, dumpDebugInfo());
    }
    deinitPhysics(world.physicsEnvironment); 
    stopSoundSystem();
    glfwTerminate(); 

  return 0;
}
