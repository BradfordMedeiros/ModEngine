#include <csignal>
#include <cxxopts.hpp>

#include "./main_input.h"
#include "./scene/common/vectorgfx.h"
#include "./scene/objtypes/lighting/scene_lighting.h"
#include "./netscene.h"
#include "./main_util.h"
#include "./cscript/cscripts/plugins/perf-visualize.h"
#include "./cscript/cscripts/plugins/performance_graph.h"
#include "./scene/common/textures_gen.h"
#include "./sql/shell.h"
#include "./common/watch_file.h"
#include "./tests/main_test.h"

#ifdef ADDITIONAL_SRC_HEADER
  #include STR(ADDITIONAL_SRC_HEADER)
#endif
CustomApiBindings* mainApi;

extern int numberOfDrawCallsThisFrame;

// application rendering stuff
struct RenderingResources { 
  unsigned int* framebufferProgram;
  unsigned int* uiShaderProgram;
  Framebuffers framebuffers;
  UniformBuffer voxelLighting;
};

RenderingResources renderingResources { };
RenderShaders mainShaders {};

// long lived app resources
DefaultResources defaultResources {};
std::string shaderFolderPath;
std::string sqlDirectory = "./res/data/sql/";
bool bootStrapperMode = false;
std::map<std::string, std::string> args;
DrawingParams drawParams = getDefaultDrawingParams();
extern std::vector<InputDispatch> inputFns;     
extern std::map<std::string, GLint> shaderstringToId;

// per frame variable data 
bool selectItemCalled = false;
extern Stats statistics;
extern ManipulatorTools tools;
LineData lineData = createLines();
std::queue<StringAttribute> channelMessages;
std::map<std::string, objid> activeLocks;

std::map<objid, unsigned int> portalIdCache;
std::optional<Texture> textureToPaint = std::optional<Texture>(std::nullopt);

Transformation viewTransform {
  .position = glm::vec3(0.f, 0.f, 0.f),
  .scale = glm::vec3(1.f, 1.f, 1.f),
  .rotation = quatFromDirection(glm::vec3(0.f, 0.f, -1.f)),
};
glm::mat4 view;
glm::mat4 ndiOrtho = glm::ortho(-1.f, 1.f, -1.f, 1.f, -1.0f, 1.0f);  


// core system 
NetCode netcode { };
engineState state = getDefaultState(1920, 1080);
World world;
RenderStages renderStages;
SysInterface interface;
KeyRemapper keyMapper;
CScriptBindingCallbacks cBindings;
btIDebugDraw* debuggerDrawer = NULL;
Benchmark benchmark;
DynamicLoading dynamicLoading;
WorldTiming timings;

bool showCrashInfo = false;
bool shouldReloadShaders = false;
float lastReloadTime = 0.f;

TimePlayback timePlayback(
  statistics.initialTime, 
  [](float currentTime, float _, float elapsedTime) -> void {
    tickAnimations(world, timings, currentTime);
  }, 
  []() -> void {}
); 


std::unordered_map<std::string, std::string>& getTemplateValues(){
  static std::unordered_map<std::string, std::string> templateValues {
    { "LIGHT_BUFFER_SIZE", "64" },
    { "BONES_BUFFER", "100" },
  };

  VoxelLightingData& lightingData = getVoxelLightingData();
  auto numCellsDim = lightingData.numCellsDim;
  int totalSize = (numCellsDim * numCellsDim * numCellsDim);
  templateValues["VOXEL_ARR_SIZE"] = std::to_string(totalSize);
  templateValues["NUM_CELLS_DIM"] = std::to_string(numCellsDim);
  templateValues["LIGHTS_PER_VOXEL"] = std::to_string(lightingData.lightsPerVoxel);

  return templateValues;
}

bool updateTime(bool fpsFixed, float fixedDelta, float speedMultiplier, int timetoexit, bool hasFramelimit, float minDeltaTime, float fpsLag){
  static float currentFps = 0.f;

  statistics.frameCount++;
  statistics.totalFrames++;

  fpscountstart:
  statistics.now = fpsFixed ? (fixedDelta * (statistics.totalFrames - 1)) :  (speedMultiplier * glfwGetTime()); // this is weird, time starts so much larger? 
  statistics.deltaTime = statistics.now - statistics.previous;   

  if (timetoexit != 0){
    float timeInSeconds = timetoexit / 1000.f;
    if (statistics.now > timeInSeconds){
      std::cout << "INFO: TIME TO EXIST EXPIRED" << std::endl;
      return true;
    }
  }
 //if (hasFramelimit &&  (statistics.deltaTime < minDeltaTime)){
  //  goto fpscountstart;
  //}
  //if (statistics.deltaTime < fpsLag){
  //  goto fpscountstart; 
  //}

  statistics.previous = statistics.now;

  if (statistics.frameCount == 60){
    statistics.frameCount = 0;
    float timedelta = statistics.now - statistics.last60;
    statistics.last60 = statistics.now;
    currentFps = floor((60.f/(timedelta) + 0.5f));
    statistics.currentFps = currentFps;
  }

  return false;
}


void registerStatistics(){
  statistics.numDrawCalls = numberOfDrawCallsThisFrame;
  numberOfDrawCallsThisFrame = 0;

  int numObjects = getNumberOfObjects(world.sandbox);
  registerStat(statistics.numObjectsStat, numObjects);

  int numRigidBodies = getNumberOfRigidBodies(world);
  registerStat(statistics.rigidBodiesStat, numRigidBodies);

  registerStat(statistics.scenesLoadedStat, getNumberScenesLoaded(world.sandbox));
  logBenchmarkTick(benchmark, statistics.deltaTime, numObjects, statistics.numTriangles);

  registerStat(statistics.fpsStat, statistics.currentFps);

}


struct IdAtCoords {
  float ndix;
  float ndiy;
  bool onlyGameObjId;
  std::optional<objid> result;
  glm::vec2 resultUv;
  std::optional<objid> textureId;
  std::function<void(std::optional<objid>, glm::vec2)> afterFrame;
};

std::vector<IdAtCoords> idCoordsToGet;
void idAtCoordAsync(float ndix, float ndiy, bool onlyGameObjId, std::optional<objid> textureId, std::function<void(std::optional<objid>, glm::vec2)> afterFrame){
  idCoordsToGet.push_back(IdAtCoords {
    .ndix = ndix,
    .ndiy = ndiy,
    .onlyGameObjId = onlyGameObjId,
    .result = std::nullopt,
    .resultUv = glm::vec2(0.f, 0.f),
    .textureId = textureId,
    .afterFrame = afterFrame,
  });
}

void renderScreenspaceShapes(Texture& texture, Texture texture2, bool shouldClear, glm::vec4 clearColor, std::optional<unsigned int> clearTextureId){
  auto texSize = getTextureSizeInfo(texture);
  auto texSize2 = getTextureSizeInfo(texture2);
  modassert(texSize.width == texSize2.width && texSize.height == texSize2.height, "screenspace - invalid tex sizes, texsize = " + print(glm::vec2(texSize.width, texSize.height)) + ", texsize2 = " + print(glm::vec2(texSize2.width, texSize2.height)));

  glViewport(0, 0, texSize.width, texSize.height);
  updateDepthTexturesSize(&renderingResources.framebuffers.textureDepthTextures.at(0), renderingResources.framebuffers.textureDepthTextures.size(), texSize.width, texSize.height); // wonder if this would be better off preallocated per gend texture?
  setActiveDepthTexture(renderingResources.framebuffers.fbo, &renderingResources.framebuffers.textureDepthTextures.at(0), 0);

  glBindFramebuffer(GL_FRAMEBUFFER, renderingResources.framebuffers.fbo);
  GLenum buffers_to_render[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, buffers_to_render);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.textureId, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,  texture2.textureId, 0);

  modassert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "framebuffer incomplete");

  glUseProgram(*renderingResources.uiShaderProgram);
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  //modassert(false, "todo - make this work with setUniformData");

  if (shouldClear){ 
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |  GL_STENCIL_BUFFER_BIT);
  }

  shaderSetUniform(*renderingResources.uiShaderProgram, "projection", ndiOrtho);
  shaderSetUniform(*renderingResources.uiShaderProgram, "encodedid", getColorFromGameobject(0));

  if (shouldClear && clearTextureId.has_value()){
    shaderSetUniform(*renderingResources.uiShaderProgram, "model", glm::scale(glm::mat4(1.0f), glm::vec3(2.f, 2.f, 2.f)));
    shaderSetUniformBool(*renderingResources.uiShaderProgram, "forceTint", false);
    shaderSetUniform(*renderingResources.uiShaderProgram, "tint", clearColor);
    drawMesh(*defaultResources.defaultMeshes.unitXYRect, *renderingResources.uiShaderProgram, clearTextureId.value());
  }

  shaderSetUniform(*renderingResources.uiShaderProgram, "model", glm::mat4(1.f));
  shaderSetUniformBool(*renderingResources.uiShaderProgram, "forceTint", true);
  shaderSetUniform(*renderingResources.uiShaderProgram, "tint", glm::vec4(1.f, 1.f, 1.f, 1.f));
  drawAllLines(lineData, *renderingResources.uiShaderProgram, texture.textureId);

  glEnable(GL_BLEND);
  glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunci(1, GL_ONE, GL_ZERO);


  drawShapeData(lineData, *renderingResources.uiShaderProgram, ndiOrtho, fontFamilyByName, texture.textureId,  texSize.height, texSize.width, *defaultResources.defaultMeshes.unitXYRect, getTextureId, false);


  for (auto &idCoordToGet : idCoordsToGet){
    if (!idCoordToGet.textureId.has_value()){
      continue;
    }
    if (idCoordToGet.textureId.value() != texture.textureId){
      continue;
    }

    auto pixelCoord = ndiToPixelCoord(glm::vec2(idCoordToGet.ndix, idCoordToGet.ndiy), glm::vec2(texSize.width, texSize.height));
    auto id = getIdFromPixelCoordAttachment1(pixelCoord.x, pixelCoord.y);
    if (id == -16777216){  // this is kind of shitty, this is black so represents no object.  However, theoretically could be an id, should make this invalid id
    }else if (idCoordToGet.onlyGameObjId && !idExists(world.sandbox, id)){
      //modassert(false, std::string("id does not exist: ") + std::to_string(id));
    }else{
      idCoordToGet.result = id;
    }
    auto uvCoordWithTex = getUVCoordAndTextureId(pixelCoord.x, pixelCoord.y);
    auto uvCoord = toUvCoord(uvCoordWithTex);
    idCoordToGet.resultUv = glm::vec2(uvCoord.x, uvCoord.y);
  }
}

bool selectItem(objid selectedId, int layerSelectIndex, int groupId, bool showCursor){
  std::cout << "SELECT ITEM CALLED!" << std::endl;
  bool shouldCallBindingOnObjectSelected = false;
  modlog("selection", (std::string("select item called") + ", selectedId = " + std::to_string(selectedId) + ", layerSelectIndex = " + std::to_string(layerSelectIndex)).c_str());
  if (!showCursor){
    return shouldCallBindingOnObjectSelected;
  }
  auto idToUse = state.groupSelection ? groupId : selectedId;
  auto selectedSubObj = getGameObject(world, selectedId);
  auto selectedObject =  getGameObject(world, idToUse);

  if (layerSelectIndex >= 0 && state.inputMode == ENABLED){
    onManipulatorSelectItem(state.manipulatorState, idToUse, selectedSubObj.name);
  }
  if (idToUse == getManipulatorId(state.manipulatorState)){
    return shouldCallBindingOnObjectSelected;
  }

  if (state.inputMode != ENABLED){
    return shouldCallBindingOnObjectSelected;
  }
  textureToPaint = textureForId(world, selectedId);
  shouldCallBindingOnObjectSelected = true;

  if (layerSelectIndex >= 0){
    setSelectedIndex(state.editor, idToUse, !state.multiselect);
    state.selectedName = selectedObject.name + "(" + std::to_string(selectedObject.id) + ")";  
  }
  setActiveObj(state.editor, idToUse);
  return shouldCallBindingOnObjectSelected;
}

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos, glm::vec3 normal, float force){
  auto obj1Id = getIdForCollisionObject(world, obj1);
  auto obj2Id = getIdForCollisionObject(world, obj2);
  if (!gameobjExists(obj1Id.value()) || !gameobjExists(obj2Id.value())){
    return;
  }
  modassert(gameobjExists(obj1Id.value()), std::string("on object enter, obj1Id does not exist - rigidbody") + print((void*)obj1));
  modassert(gameobjExists(obj2Id.value()), std::string("on object enter, obj2Id does not exist - rigidbody") + print((void*)obj2));
  maybeTeleportObjects(world, obj1Id.value(), obj2Id.value());
  cBindings.onCollisionEnter(obj1Id.value(), obj2Id.value(), contactPos, normal, normal * glm::vec3(-1.f, -1.f, -1.f), force); 
}
void onObjectLeave(const btCollisionObject* obj1, const btCollisionObject* obj2){
  auto obj1Id = getIdForCollisionObject(world, obj1);
  auto obj2Id = getIdForCollisionObject(world, obj2);
  if (!gameobjExists(obj1Id.value()) || !gameobjExists(obj2Id.value())){
    return;
  }
  modassert(gameobjExists(obj1Id.value()), std::string("on object enter, obj1Id does not exist - rigidbody") + print((void*)obj1));
  modassert(gameobjExists(obj2Id.value()), std::string("on object enter, obj2Id does not exist - rigidbody") + print((void*)obj2));
  cBindings.onCollisionExit(obj1Id.value(), obj2Id.value());
}


std::vector<std::string> listOctreeTextures(){
  std::vector<std::string> octreeTextures { 
    "./res/textures/grid.png", 
    "../gameresources/build/textures/clean/pebbles2.png",

    "../gameresources/build/textures/clean/grass.jpg", 
    "../gameresources/build/textures/clean/tunnel_road.jpg", 

    "../gameresources/build/textures/clean/cherrybark.jpg",
    "../gameresources/build/textures/clean/foliage2.png",
    "../gameresources/build/textures/clean/hardwood.jpg",

    "../gameresources/build/textures/clean/stonydirt.jpg",
    "../gameresources/build/textures/clean/metal_scifi.png",
    "../gameresources/build/textures/clean/tex_Ice.jpg",

    "../gameresources/build/textures/clean/stonewall.jpg",
    "../gameresources/build/textures/metal_grating.jpg",
    "../gameresources/build/textures/moonman.jpg",

    "../gameresources/build/uncategorized/bluetransparent.png",
    "../gameresources/build/textures/const_fence.png",

    "/home/brad/Desktop/test3.png",

  };
  return octreeTextures;
}

// This is wasteful, as obviously I shouldn't be loading in all the textures on load, but ok for now. 
// This shoiuld really just be creating a list of names, and then the cycle above should cycle between possible textures to load, instead of what is loaded 
void loadAllTextures(std::string& textureFolderPath){
  loadTextureWorld(world, "./res/models/box/grid.png", -1);
  loadTextureWorld(world, "./res/textures/wood.jpg", -1);
  for (auto texturePath : listFilesWithExtensions(textureFolderPath, { "png", "jpg" })){
    loadTextureWorld(world, texturePath, -1);
  }

  /*for (auto texturePath : listFilesWithExtensions("/home/brad/automate/mosttrusted/gameresources/build/", { "png", "jpg" })){
    loadTextureWorld(world, texturePath, -1);
  }*/

  std::string flatSurfaceNormalTexture = "../gameresources/build/textures/clean/cherrybark.normal.jpg";
  /*std::vector<std::string> textures { 
    "./res/textures/grid.png", 
    "../gameresources/build/textures/clean/tunnel_road.jpg", 
    "../gameresources/build/textures/clean/grass.jpg", 
    "../gameresources/build/textures/clean/pebbles2.png"
  };*/

  std::vector<std::string> octreeTextures = listOctreeTextures();

  std::vector<std::string> normalTextures;
  for (auto &texture : octreeTextures){
    auto normalTexture = lookupNormalTexture(world, texture);
    if (normalTexture.has_value()){
      normalTextures.push_back(normalTexture.value());
    }else{
      normalTextures.push_back(flatSurfaceNormalTexture);
    }
  }

  PROFILE("TEXTURES-LOAD-OCTREEATLAS",
    auto start = std::chrono::steady_clock::now();
    loadTextureAtlasWorld(world, "octree-atlas:normal", normalTextures, "/home/brad/Desktop/test_atlas_normal.png", -1);
    loadTextureAtlasWorld(world, "octree-atlas:main",   octreeTextures, "/home/brad/Desktop/test_atlas.png", -1);
    setAtlasDimensions(AtlasDimensions {
      .textureNames = octreeTextures,
    });
    auto end = std::chrono::steady_clock::now();
    std::chrono::milliseconds timeSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    modlog("texture octree loading time(ms)", std::to_string(timeSeconds.count())); // eventually just cleanup perf to allow PROFILE to give this info generically
  )
}

// Kind of crappy since the uniforms don't unset their values after rendering, but order should be deterministic so ... ok
void setRenderUniformData(unsigned int shader, RenderUniforms& uniforms){
  for (auto &uniform : uniforms.intUniforms){
    shaderSetUniformInt(shader, uniform.uniformName.c_str(), uniform.value);
  }
  for (auto &uniform : uniforms.floatUniforms){
    shaderSetUniform(shader, uniform.uniformName.c_str(), uniform.value);
  }
  for (auto &uniform : uniforms.vec3Uniforms){
    shaderSetUniform(shader, uniform.uniformName.c_str(), uniform.value);
  }
  for (auto &uniform : uniforms.floatArrUniforms){
    for (int i = 0; i < uniform.value.size(); i++){
      shaderSetUniform(shader,  (uniform.uniformName + "[" + std::to_string(i) + "]").c_str(), uniform.value.at(i));
    }
  }
  for (auto &uniform : uniforms.builtInUniforms){  // todo -> avoid string comparisons
    if (uniform.builtin == "resolution"){
      shaderSetUniformIVec2(shader, uniform.uniformName.c_str(), state.resolution);
    }else{
      std::cout << "uniform not supported: " << uniform.builtin << std::endl;
      assert(false);
    }
  }
}

std::vector<UniformData> getDefaultShaderUniforms(std::optional<glm::mat4> projview, glm::vec3 cameraPosition, std::vector<LightInfo>& lights, bool enableLighting){
  std::vector<UniformData> uniformData;
  uniformData.push_back(UniformData {
    .name = "maintexture",
    .value = Sampler2D {
      .textureUnitId = 0,
    },
  });
  /* obviously texture id an maintexture shouldn't be same here */
  uniformData.push_back(UniformData { 
    .name = "textureid",
    .value = Sampler2D {
      .textureUnitId = 0,
    },
  });

  uniformData.push_back(UniformData {
    .name = "emissionTexture",
    .value = Sampler2D {
      .textureUnitId = 1,
    },
  });
  uniformData.push_back(UniformData {
    .name = "opacityTexture",
    .value = Sampler2D {
      .textureUnitId = 2,
    },
  });
  uniformData.push_back(UniformData {
    .name = "lightDepthTexture",
    .value = Sampler2D {
      .textureUnitId = 3,
    },
  });
  uniformData.push_back(UniformData {
    .name = "cubemapTexture",
    .value = SamplerCube {
      .textureUnitId = 4,
    },
  });
  uniformData.push_back(UniformData {
    .name = "roughnessTexture",
    .value = Sampler2D {
      .textureUnitId = 5,
    },
  });
  uniformData.push_back(UniformData {
    .name = "normalTexture",
    .value = Sampler2D {
      .textureUnitId = 6,
    },
  });
  if (projview.has_value()){
    uniformData.push_back(UniformData {
      .name = "projview",
      .value = projview.value(),
    });
  }
  uniformData.push_back(UniformData {
    .name = "showBoneWeight",
    .value = state.showBoneWeight,
  });
  uniformData.push_back(UniformData {
    .name = "useBoneTransform",
    .value = state.useBoneTransform,
  });
  uniformData.push_back(UniformData {
    .name = "enableDiffuse",
    .value = state.enableDiffuse,
  });
  uniformData.push_back(UniformData {
    .name = "enableLighting",
    .value = enableLighting,
  });
  uniformData.push_back(UniformData {
    .name = "enablePBR",
    .value = state.enablePBR,
  });
  uniformData.push_back(UniformData {
    .name = "enableSpecular",
    .value = state.enableSpecular,
  });
  uniformData.push_back(UniformData {
    .name = "enableVoxelLighting",
    .value = state.enableVoxelLighting,
  });
  uniformData.push_back(UniformData {
    .name = "ambientAmount",
    .value = glm::vec3(state.ambient),
  });
  uniformData.push_back(UniformData {
    .name = "bloomThreshold",
    .value = state.bloomThreshold,
  });
  uniformData.push_back(UniformData {
    .name = "enableAttenutation",
    .value = state.enableAttenuation,
  });
  uniformData.push_back(UniformData {
    .name = "cameraPosition",
    .value = cameraPosition,
  });
  uniformData.push_back(UniformData {
    .name = "shadowIntensity",
    .value = state.shadowIntensity,
  });
  uniformData.push_back(UniformData {
    .name = "enableShadows",
    .value = state.enableShadows,
  });
  uniformData.push_back(UniformData {
    .name = "numlights",
    .value = static_cast<int>(lights.size()),
  });
  uniformData.push_back(UniformData {
    .name = "time",
    .value = getTotalTimeGame(),
  });
  uniformData.push_back(UniformData {
    .name = "realtime",
    .value = getTotalTime(),
  });
  return uniformData;  
}

int getLightsArrayIndex(std::vector<LightInfo>& lights, objid lightId){
  for (int i = 0; i < lights.size(); i++){
    if (lights.at(i).id == lightId){
      return i;
    }
  }
  if (lightId == -2){
    return -2;
  }
  return -1;
}

void setShaderWorld(GLint shader, std::vector<LightInfo>& lights, std::vector<glm::mat4> lightProjview, glm::vec3 cameraPosition, RenderUniforms& uniforms){
  //std::cout << "set shader data world" << std::endl; 
  glUseProgram(shader);
  std::vector<UniformData> uniformData = getDefaultShaderUniforms(std::nullopt, cameraPosition, lights, true);
  // notice this is kind of wrong, since it sets it for multiple shader types here
  setUniformData(shader, uniformData, { 
    "textureid", "bones[0]", "encodedid", "hasBones", "model", "discardTexAmount", 
    "emissionAmount", 
    "hasCubemapTexture", "hasDiffuseTexture", "hasEmissionTexture", "hasNormalTexture", "hasOpacityTexture",
    "lights[0]", "lightsangledelta[0]", "lightsatten[0]", "lightscolor[0]", "lightsdir[0]", "lightsisdir[0]", "lightsmaxangle[0]", "voxelindexs2[0]", "colors[0]", "voxelcellwidth",
    "lightsprojview", "textureOffset", "textureSize", "textureTiling", "tint", "projview"
  });
 

  if (shader != *renderStages.selection.shader){
    auto lightUpdates = getLightUpdates();
    for (auto &lightUpdate : lightUpdates){
      //std::cout << "voxel lighting : " << lightUpdate.index << std::endl;
      int lightArrayIndex = getLightsArrayIndex(lights, lightUpdate.lightIndex);
      modassert(lightUpdate.index < 512, "lightUpdate.index too large");
      updateBufferData(renderingResources.voxelLighting , sizeof(glm::vec4) * lightUpdate.index, sizeof(int), &lightArrayIndex);
    }
    shaderSetUniformInt(shader, "voxelcellwidth", getLightingCellWidth());
  }


  for (int i = 0; i < lights.size(); i++){
    if (shader != *renderStages.selection.shader){
      glm::vec3 position = lights.at(i).transform.position;
      auto& light = lights.at(i); 
      shaderSetUniform(shader, ("lights[" + std::to_string(i) + "]").c_str(), position);
      if (!light.light.disabled){
        shaderSetUniform(shader, ("lightscolor[" + std::to_string(i) + "]").c_str(), light.light.color);
      }else{
        shaderSetUniform(shader, ("lightscolor[" + std::to_string(i) + "]").c_str(), glm::vec3(0.f, 0.f, 0.f));
      }
      shaderSetUniform(shader, ("lightsdir[" + std::to_string(i) + "]").c_str(), directionFromQuat(light.transform.rotation));
      shaderSetUniform(shader, ("lightsatten[" + std::to_string(i) + "]").c_str(), light.light.attenuation);
      shaderSetUniform(shader,  ("lightsmaxangle[" + std::to_string(i) + "]").c_str(), light.light.type == LIGHT_SPOTLIGHT ? light.light.maxangle : -10.f);
      shaderSetUniform(shader,  ("lightsangledelta[" + std::to_string(i) + "]").c_str(), light.light.angledelta);
      shaderSetUniformBool(shader,  ("lightsisdir[" + std::to_string(i) + "]").c_str(), light.light.type == LIGHT_DIRECTIONAL);
    }

    if (lightProjview.size() > i){
      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.depthTextures.at(1));
      shaderSetUniform(shader, "lightsprojview", lightProjview.at(i));
    }
  }
  glActiveTexture(GL_TEXTURE0); 
  setRenderUniformData(shader, uniforms);
}
void setShaderDataObject(GLint shader, glm::vec3 color, objid id, glm::mat4 projview){
  //std::cout << "set shader data object" << std::endl; 
  if (shader != *renderStages.selection.shader){
    shaderSetUniform(shader, "tint", glm::vec4(color.x, color.y, color.z, 1.f));
  }
  if (shader == *renderStages.selection.shader){
    shaderSetUniform(shader, "encodedid", getColorFromGameobject(id));
  }
  shaderSetUniform(shader, "projview", projview);
}
void setShaderData(GLint shader, glm::mat4 proj, glm::mat4 view, std::vector<LightInfo>& lights, bool orthographic, glm::vec3 color, objid id, std::vector<glm::mat4> lightProjview, glm::vec3 cameraPosition, RenderUniforms& uniforms){
  auto projview = (orthographic ? glm::ortho(-1.f, 1.f, -1.f, 1.f, 0.f, 100.0f) : proj) * view;
  setShaderWorld(shader, lights, lightProjview, cameraPosition, uniforms);
  setShaderDataObject(shader, color, id, projview);
}



int renderWorld(World& world,  GLint shaderProgram, bool allowShaderOverride, glm::mat4* projection, glm::mat4 view,  glm::mat4 model, std::vector<LightInfo>& lights, std::vector<PortalInfo> portals, std::vector<glm::mat4> lightProjview, glm::vec3 cameraPosition, bool textBoundingOnly){
  glUseProgram(shaderProgram);
  int numTriangles = 0;

  std::optional<GLint> lastShaderId = std::nullopt;
  traverseSandboxByLayer(world.sandbox, [&world, shaderProgram, allowShaderOverride, projection, view, &portals, &lights, &lightProjview, &numTriangles, &cameraPosition, textBoundingOnly, &lastShaderId](int32_t id, glm::mat4 modelMatrix, LayerInfo& layer, std::string shader) -> void {
    modassert(id >= 0, "unexpected id render world");
    auto proj = projection == NULL ? projectionFromLayer(layer) : *projection;

     // This could easily be moved to reduce opengl context switches since the onObject sorts on layers (so just have to pass down).  
    if (state.depthBufferLayer != layer.depthBufferLayer){
      state.depthBufferLayer = layer.depthBufferLayer;
      modassert(state.depthBufferLayer < renderingResources.framebuffers.depthTextures.size(), std::string("invalid layer index: ") + std::to_string(state.depthBufferLayer) + std::string(" [") + layer.name + std::string("]"));
      setActiveDepthTexture(renderingResources.framebuffers.fbo, &renderingResources.framebuffers.depthTextures.at(0), layer.depthBufferLayer);
      glClear(GL_DEPTH_BUFFER_BIT);
    }

    auto newShader = (shader == "" || !allowShaderOverride) ? shaderProgram : getShaderByShaderString(shader, shaderFolderPath, interface.readFile, getTemplateValues);
    if (!lastShaderId.has_value() || newShader != lastShaderId.value()){
      lastShaderId = newShader;
      //sendAlert(std::string("loaded shader: ") + shader);
      setShaderWorld(newShader, lights, lightProjview, cameraPosition, layer.uniforms);
    }
    
    bool objectSelected = idInGroup(world, id, selectedIds(state.editor));

    setShaderDataObject(newShader, getTintIfSelected(objectSelected), id, (layer.orthographic ?  glm::ortho(-1.f, 1.f, -1.f, 1.f, 0.f, 100.0f) :  proj) * (layer.disableViewTransform ? glm::mat4(1.f) : view));

    // bounding code //////////////////////
    auto meshObj = std::get_if<GameObjectMesh>(&world.objectMapping.at(id)); 
    if (meshObj != NULL && meshObj -> meshesToRender.size() > 0){
      // @TODO i use first mesh to get sizing for bounding box, obviously that's questionable
      auto bounding = getBoundRatio(world.meshes.at("./res/models/boundingbox/boundingbox.obj").mesh.boundInfo, meshObj -> meshesToRender.at(0).boundInfo);
      shaderSetUniform(newShader, "model", glm::scale(getMatrixForBoundRatio(bounding, modelMatrix), glm::vec3(1.01f, 1.01f, 1.01f)));
      if (objectSelected){
        drawMesh(world.meshes.at("./res/models/boundingbox/boundingbox.obj").mesh, newShader);
      }
    }

    shaderSetUniform(newShader, "model", (layer.scale ? calculateScaledMatrix(view, modelMatrix, layer.fov) : modelMatrix));

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

    if (layer.visible && id != 0){
      //std::cout << "render object: " << getGameObject(world, id).name << std::endl;
      RenderObjApi api {
        .drawLine = interface.drawLine,
        .drawSphere = [&modelMatrix, &newShader](glm::vec3 pos) -> int {
          auto sphereLines = drawSphere();
          const float sphereScale = 0.05f;
          for (auto &line : sphereLines){
            // can use addShaepData next cycle with selection id if we want to be able to drag these around
            interface.drawLine(line.fromPos * sphereScale + pos, line.toPos * sphereScale + pos, glm::vec4(0.f, 0.f, 1.f, 1.f));
          }
          modassert(false, "did not return id");
          return 0;
        },
        .drawWord = drawWord,
        .isBone = [&world](objid id) -> bool {
          return getGameObject(world.sandbox, id).isBone;
        },
        .getParentId = [&world](objid id) -> std::optional<objid> {
          return getGameObjectH(world.sandbox, id).parentId;
        },
        .getTransform = [&world](objid id) -> Transformation {
          return fullTransformation(world.sandbox, id);
        },
      };

      auto trianglesDrawn = renderObject(
        newShader, 
        newShader == *renderStages.selection.shader,
        id, 
        world.objectMapping, 
        state.showDebug ? state.showDebugMask : 0,
        (isPortal && portalTextureInCache &&  !isPerspectivePortal) ? portalIdCache.at(id) : -1,
        state.navmeshTextureId.has_value() ? state.navmeshTextureId.value() : -1,
        modelMatrix,
        state.drawPoints,
        defaultResources.defaultMeshes,
        textBoundingOnly,
        state.showBones,
        api
      );
      numTriangles = numTriangles + trianglesDrawn;
    }
  
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    if (isPortal && portalTextureInCache && isPerspectivePortal){
      glUseProgram(*renderingResources.framebufferProgram); 
      glDisable(GL_DEPTH_TEST);
      glBindVertexArray(defaultResources.quadVAO);
      glBindTexture(GL_TEXTURE_2D,  portalIdCache.at(id));
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glEnable(GL_DEPTH_TEST);
      glUseProgram(newShader); 
    }
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glClear(GL_STENCIL_BUFFER_BIT);
  });
  
  auto maxExpectedClears = numUniqueDepthLayers(world.sandbox.layers);
  //modassert(numDepthClears <= maxExpectedClears, std::string("numDepthClears = ") + std::to_string(numDepthClears) + std::string(", expected = ") + std::to_string(maxExpectedClears));
 
  return numTriangles;
}

void renderVector(GLint shaderProgram, glm::mat4 view,  int numChunkingGridCells){
  glUseProgram(shaderProgram);
  glDisable(GL_DEPTH_TEST);

  auto projection = projectionFromLayer(world.sandbox.layers.at(0));
  std::vector<LightInfo> lights;
  std::vector<UniformData> uniformData = getDefaultShaderUniforms(projection * view, glm::vec3(0.f, 0.f, 0.f), lights, false);
  uniformData.push_back(UniformData { .name = "tint",  .value = glm::vec4(0.05, 1.f, 0.f, 1.f) });
  uniformData.push_back(UniformData { .name = "hasBones",  .value = false });
  uniformData.push_back(UniformData { .name = "hasCubemapTexture",  .value = false });
  uniformData.push_back(UniformData { .name = "hasDiffuseTexture",  .value = false });
  uniformData.push_back(UniformData { .name = "hasEmissionTexture",  .value = false });
  uniformData.push_back(UniformData { .name = "hasNormalTexture",  .value = false });
  uniformData.push_back(UniformData { .name = "hasOpacityTexture",  .value = false });
  uniformData.push_back(UniformData { .name = "hasRoughnessTexture",  .value = false });
  uniformData.push_back(UniformData { .name = "discardTexAmount",  .value = 0.f });
  uniformData.push_back(UniformData { .name = "emissionAmount",  .value = glm::vec3(0.f, 0.f, 0.f) });
  uniformData.push_back(UniformData { .name = "model",  .value = glm::mat4(1.f) });
  uniformData.push_back(UniformData { .name = "lightsprojview",  .value = glm::mat4(1.f) });
  uniformData.push_back(UniformData { .name = "textureOffset",  .value = glm::vec2(1.f, 1.f) });
  uniformData.push_back(UniformData { .name = "textureSize",  .value = glm::vec2(1.f, 1.f) });
  uniformData.push_back(UniformData { .name = "textureTiling",  .value = glm::vec2(1.f, 1.f) });
  setUniformData(shaderProgram, uniformData, { "bones[0]", "lights[0]", "lightsangledelta[0]", "lightsatten[0]", "lightscolor[0]", "lightsdir[0]", "lightsisdir[0]", "lightsmaxangle[0]", "voxelindexs2[0]", "colors[0]", "voxelcellwidth" });

  // Draw grid for the chunking logic if that is specified, else lots draw the snapping translations
  if (state.showDebug && numChunkingGridCells > 0){
    float offset = ((numChunkingGridCells % 2) == 0) ? (dynamicLoading.mappingInfo.chunkSize / 2) : 0;
    drawGrid3D(numChunkingGridCells, dynamicLoading.mappingInfo.chunkSize, offset, offset, offset);
  }

  if (state.visualizeVoxelLightingCells){
    drawGrid3D(4, getLightingCellWidth(), 0.f, 0.f, 0.f);
  }

  if (state.manipulatorMode == TRANSLATE && state.showGrid && (state.inputMode == ENABLED)){
    for (auto id : selectedIds(state.editor)){
      auto selectedObj = id;
      if (selectedObj != -1){
        auto snapCoord = getSnapTranslateSize(state.easyUse);
        float snapGridSize = snapCoord.size;
        if (snapGridSize > 0){
          auto position = getGameObjectPosition(selectedObj, false);
          if (state.manipulatorAxis == XAXIS){
            drawGridXY(state.gridSize, state.gridSize, snapGridSize, position.x, position.y, position.z, snapCoord.orientation);  
          }else if (state.manipulatorAxis == YAXIS){
            drawGridXZ(state.gridSize, state.gridSize, snapGridSize, position.x, position.y, position.z, snapCoord.orientation);  
          }else if (state.manipulatorAxis == ZAXIS){
            drawGridYZ(state.gridSize, state.gridSize, snapGridSize, position.x, position.y, position.z, snapCoord.orientation);  
          }else{
            drawGrid3D(state.gridSize, snapGridSize, position.x, position.y, position.z);  
          }
        }
      }
    }    
  }

  shaderSetUniform(shaderProgram, "tint", glm::vec4(0.f, 0.f, 1.f, 1.f));     
  if (state.showDebug){
    drawCoordinateSystem(100.f);
  }
  drawAllLines(lineData, shaderProgram, std::nullopt);

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
  setShaderData(shaderProgram, projection, value, lights, false, glm::vec3(state.skyboxcolor.x, state.skyboxcolor.y, state.skyboxcolor.z), 0, lightProjView, cameraPosition, noUniforms);
  shaderSetUniform(shaderProgram, "model", glm::mat4(1.f));
  drawMesh(world.meshes.at("skybox").mesh, shaderProgram); 
}

void renderUI(Color pixelColor){
  glUseProgram(*renderingResources.uiShaderProgram);
  std::vector<UniformData> uniformData;
  uniformData.push_back(UniformData {
    .name = "projection",
    .value = ndiOrtho,
  });
  uniformData.push_back(UniformData {
    .name = "forceTint",
    .value = false,
  });
  uniformData.push_back(UniformData {
    .name = "textureData",
    .value = Sampler2D {
      .textureUnitId = 0,
    },
  });
  uniformData.push_back(UniformData {
    .name = "time",
    .value = getTotalTimeGame(),
  });
  uniformData.push_back(UniformData {
    .name = "realtime",
    .value = getTotalTime(),
  });
  setUniformData(*renderingResources.uiShaderProgram, uniformData, { "model", "encodedid", "tint", "time" });
  glEnable(GL_BLEND);
  glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunci(1, GL_ONE, GL_ZERO);
  
  const float offsetPerLineMargin = 0.02f;
  float offsetPerLine = -1 * (state.fontsize / 500.f + offsetPerLineMargin);
  float uiYOffset = (1.f + 3 * offsetPerLine) + state.infoTextOffset.y;
  float uiXOffset = (-1.f - offsetPerLine) + state.infoTextOffset.x;

  static std::string nameForStat  = args.find("stat") != args.end() ? args.at("stat") : std::string("");
  if (nameForStat != ""){
    auto stat = statValue(statName(nameForStat));
    drawTextNdi(std::string(nameForStat) + ": " + print(stat), uiXOffset, uiYOffset + offsetPerLine * -2, state.fontsize * 2);
  }
  
  if (!state.showDebug){
    return;
  }

  auto currentFramerate = static_cast<int>(unwrapStat<float>(statValue(statistics.fpsStat)));
  //std::cout << "offsets: " << uiXOffset << " " << uiYOffset << std::endl;
  std::string additionalText =  "     <" + std::to_string((int)(255 * state.hoveredItemColor.r)) + ","  + std::to_string((int)(255 * state.hoveredItemColor.g)) + " , " + std::to_string((int)(255 * state.hoveredItemColor.b)) + ">  " + " --- " + state.selectedName;
  drawTextNdi(std::to_string(currentFramerate) + additionalText, uiXOffset, uiYOffset + offsetPerLine, state.fontsize + 1);

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
  drawTextNdi("position: " + print(defaultResources.defaultCamera.transformation.position), uiXOffset, uiYOffset + offsetPerLine * 3, state.fontsize);
  drawTextNdi("rotation: " + print(defaultResources.defaultCamera.transformation.rotation), uiXOffset, uiYOffset + offsetPerLine * 4, state.fontsize);

  float ndiX = 2 * (state.cursorLeft / (float)state.resolution.x) - 1.f;
  float ndiY = -2 * (state.cursorTop / (float)state.resolution.y) + 1.f;

  drawTextNdi("cursor: (" + std::to_string(ndiX) + " | " + std::to_string(ndiY) + ") - " + std::to_string(state.cursorLeft) + " / " + std::to_string(state.cursorTop)  + "(" + std::to_string(state.resolution.x) + "||" + std::to_string(state.resolution.y) + ")", uiXOffset, uiYOffset + offsetPerLine * 5, state.fontsize);
  
  std::string position = "n/a";
  std::string scale = "n/a";
  std::string rotation = "n/a";

  auto selectedValue = latestSelected(state.editor);
  if (selectedValue.has_value()){
    auto selectedIndex = selectedValue.value();
    auto transformation = gameobjectTransformation(world, selectedIndex, false);
    position = print(transformation.position);
    scale = print(transformation.scale);
    rotation = serializeQuat(transformation.rotation);
  }

  drawTextNdi("position: " + position, uiXOffset, uiYOffset + offsetPerLine * 6, state.fontsize);
  drawTextNdi("scale: " + scale, uiXOffset, uiYOffset + offsetPerLine * 7, state.fontsize);
  drawTextNdi("rotation: " + rotation, uiXOffset, uiYOffset + offsetPerLine * 8, state.fontsize);
    
  drawTextNdi("pixel color: " + std::to_string(pixelColor.r) + " " + std::to_string(pixelColor.g) + " " + std::to_string(pixelColor.b), uiXOffset, uiYOffset + offsetPerLine * 9, state.fontsize);
  drawTextNdi("showing color: " + std::string(state.showBoneWeight ? "bone weight" : "bone indicies") , uiXOffset, uiYOffset + offsetPerLine * 10, state.fontsize);

  auto idExists = gameobjExists(state.currentHoverIndex);
  std::string name = idExists ? getGameObjectName(state.currentHoverIndex).value() : "[none]";
  drawTextNdi("hovered id: " + std::to_string(state.currentHoverIndex) + " - " + name, uiXOffset, uiYOffset + offsetPerLine * 11, state.fontsize);


  drawTextNdi(std::string("animation info: ") + (timePlayback.isPaused() ? "paused" : "playing"), uiXOffset, uiYOffset + offsetPerLine * 12, state.fontsize);
  drawTextNdi("using animation: " + std::to_string(-1) + " / " + std::to_string(-1) , uiXOffset, uiYOffset + offsetPerLine * 13, state.fontsize);
  drawTextNdi("using object id: -1" , uiXOffset, uiYOffset + offsetPerLine * 14, state.fontsize);

  drawTextNdi(std::string("triangles: ") + std::to_string(statistics.numTriangles), uiXOffset, uiYOffset + offsetPerLine * 15, state.fontsize);
  drawTextNdi(std::string("draw calls: ") + std::to_string(statistics.numDrawCalls), uiXOffset, uiYOffset + offsetPerLine * 16, state.fontsize);
  drawTextNdi(std::string("num gameobjects: ") + std::to_string(unwrapStat<int>(statValue(statistics.numObjectsStat))), uiXOffset, uiYOffset + offsetPerLine * 17, state.fontsize);
  drawTextNdi(std::string("num rigidbodys: ") + std::to_string(unwrapStat<int>(statValue(statistics.rigidBodiesStat))), uiXOffset, uiYOffset + offsetPerLine * 18, state.fontsize);
  drawTextNdi(std::string("num scenes loaded: ") + std::to_string(unwrapStat<int>(statValue(statistics.scenesLoadedStat))), uiXOffset, uiYOffset + offsetPerLine * 19, state.fontsize);
  drawTextNdi(std::string("render mode: ") + renderModeAsStr(state.renderMode), uiXOffset, uiYOffset + offsetPerLine * 20, state.fontsize);
  drawTextNdi(std::string("time: ") + std::to_string(timeSeconds(false)), uiXOffset, uiYOffset + offsetPerLine * 21, state.fontsize);
  drawTextNdi(std::string("realtime: ") + std::to_string(timeSeconds(true)), uiXOffset, uiYOffset + offsetPerLine * 22, state.fontsize);
}


void onClientMessage(std::string message){
  cBindings.onTcpMessage(message);
}

bool signalHandlerCalled = false;
void signalHandler(int signum) {
  return;
  if (!showCrashInfo || signalHandlerCalled){
    return;
  }
  signalHandlerCalled = true;
  std::cout << "signal handler called" << std::endl;
  auto debugInfo = dumpDebugInfo();
  std::cout << debugInfo << std::endl;
  if (state.showDebug){
    auto crashFile = "./build/crash.info";
    std::cout << "wrote crash file: " << crashFile << std::endl;
    saveFile(crashFile, debugInfo);
    printBacktrace();
  }
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
    glUseProgram(*renderStep.shader);
    setRenderUniformData(*renderStep.shader, renderStep.uniforms);
    for (int i = 0; i < renderStep.textures.size(); i++){
      auto &textureData = renderStep.textures.at(i);
      int activeTextureOffset = 7 + i; // this is funny, but basically other textures before this use up to 5, probably should centralize these values
      shaderSetUniformInt(*renderStep.shader, textureData.nameInShader.c_str(), activeTextureOffset);
      glActiveTexture(GL_TEXTURE0 + activeTextureOffset);
      if (textureData.type == RENDER_TEXTURE_REGULAR){
        glBindTexture(GL_TEXTURE_2D, world.textures.at(textureData.textureName).texture.textureId);
      }else{
        glBindTexture(GL_TEXTURE_2D, textureData.framebufferTextureId);
      }
    }
    glActiveTexture(GL_TEXTURE0);

    setActiveDepthTexture(renderingResources.framebuffers.fbo, &renderingResources.framebuffers.depthTextures.at(0), renderStep.depthTextureIndex);
    glBindFramebuffer(GL_FRAMEBUFFER, renderStep.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderStep.colorAttachment0, 0);
    if (renderStep.hasColorAttachment1){
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderStep.colorAttachment1, 0);
    }

    glClearColor(0.0, 0.0, 0.0, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |  GL_STENCIL_BUFFER_BIT);

    if (state.showSkybox && renderStep.renderSkybox){
      glDepthMask(GL_FALSE);
      renderSkybox(*renderStep.shader, context.view, context.cameraTransform.position);  // Probably better to render this at the end 
      glDepthMask(GL_TRUE);    
    }
    glEnable(GL_DEPTH_TEST);
    if (renderStep.blend){
      glEnable(GL_BLEND);
      glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glBlendFunci(1, GL_ONE, GL_ZERO);
    }else{
      glDisable(GL_BLEND);
    }

    if (renderStep.renderQuad3D){
      std::vector<LightInfo> lights = {};
      std::vector<glm::mat4> lightProjview = {};
      RenderUniforms uniforms { };
      setShaderData(*renderStep.shader, ndiOrtho, glm::mat4(1.f), lights, false, glm::vec3(1.f, 1.f, 1.f), 0, lightProjview, glm::vec3(0.f, 0.f, 0.f), uniforms);
      glActiveTexture(GL_TEXTURE0); 
      glBindTexture(GL_TEXTURE_2D, renderStep.quadTexture);
      glBindVertexArray(defaultResources.quadVAO3D);
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
      auto worldTriangles = renderWorld(context.world, *renderStep.shader, renderStep.allowShaderOverride, projection, context.view, glm::mat4(1.0f), context.lights, context.portals, context.lightProjview, context.cameraTransform.position, renderStep.textBoundingOnly);
      triangles += worldTriangles;
    }

    glDisable(GL_STENCIL_TEST);

    if (renderStep.renderQuad){
      glBindTexture(GL_TEXTURE_2D, renderStep.quadTexture);
      glBindVertexArray(defaultResources.quadVAO);
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

float getViewspaceDepth(glm::mat4& transView, objid elementId){
  auto viewPosition = transView * fullModelTransform(world.sandbox, elementId);
  return getTransformationFromMatrix(viewPosition).position.z;
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
      modassert(elements.size() == 1, std::string("elements size = ") + std::to_string(elements.size()));
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


void onGLFWEerror(int error, const char* description){
  std::cerr << "Error: " << description << std::endl;
}

void setSelected(std::optional<std::set<objid>> ids){
  clearSelectedIndexs(state.editor);
  for (auto id : ids.value()){
    if (getManipulatorId(state.manipulatorState) == id){
      continue;
    }
    setSelectedIndex(state.editor, id, !state.multiselect);
  }
}

struct RequestMovingObject {
  glm::vec3 initialPos;
  glm::vec3 finalPos;
  float initialTime;
  float duration;
};
std::unordered_map<objid, RequestMovingObject> requestMovingObjects;
void moveCameraTo(objid cameraId, glm::vec3 position, std::optional<float> duration){
  if (!duration.has_value()){
    setGameObjectPosition(cameraId, position, true);
    return;
  }
  requestMovingObjects[cameraId] = RequestMovingObject {
    .initialPos = getGameObjectPosition(cameraId, true),
    .finalPos = position,
    .initialTime = timePlayback.currentTime,
    .duration = duration.value(),
  };
}
void handleMovingObjects(float currTime){
  std::vector<objid> idsToRemove;
  for (auto &[id, movingObject] : requestMovingObjects){
    float elapsedTime = currTime - movingObject.initialTime;
    float percentage = elapsedTime / movingObject.duration;
    bool finished = percentage >= 1.f;
    percentage = percentage > 1.f ? 1.f : percentage;
    auto distance = percentage * (movingObject.finalPos - movingObject.initialPos);
    auto newPosition = movingObject.initialPos + distance;
    setGameObjectPosition(id, newPosition, true);
    if (finished){
      idsToRemove.push_back(id);
    }
  }
  for (auto id : idsToRemove){
    requestMovingObjects.erase(id);
  }
}


std::optional<unsigned int> shaderByName(std::string name){
  if (shaderstringToId.find(name) == shaderstringToId.end()){
    return std::nullopt;
  }
  return shaderstringToId.at(name); 
}

std::optional<unsigned int> getTextureSamplerId(std::string& texturepath){
  return world.textures.at(texturepath).texture.textureId;
}


GLFWwindow* window = NULL;
GLFWmonitor* monitor = NULL;
const GLFWvidmode* mode = NULL;

int main(int argc, char* argv[]){
  auto start = std::chrono::steady_clock::now();

  signal(SIGABRT, signalHandler);  

  std::string argsString = "";
  for (int i = 0; i < argc; i++){
    argsString += std::string(argv[i]) + " ";
  }
  modlog("command", argsString);

  cxxopts::Options cxxoption("ModEngine", "ModEngine is a game engine for hardcore fps");
  cxxoption.add_options()
   ("s,shader", "Folder path of default shader", cxxopts::value<std::string>()->default_value("./res/shaders/default"))
   ("t,texture", "Additional textures folder to use", cxxopts::value<std::string>()->default_value("./res"))
   ("x,scriptpath", "Script file to use", cxxopts::value<std::vector<std::string>>()->default_value(""))
   ("u,uishader", "Shader to use for ui", cxxopts::value<std::string>()->default_value("./res/shaders/ui"))
   ("fps", "Framerate limit", cxxopts::value<int>()->default_value("0"))
   ("fps-fixed", "Whether to guarantee the framerate, which means values do not occur in realtime", cxxopts::value<bool>()->default_value("false"))
   ("fps-lag", "Extra lag to induce in each frame in ms", cxxopts::value<int>()->default_value("-1"))
   ("f,fullscreen", "Enable fullscreen mode", cxxopts::value<bool>()->default_value("false"))
   ("i,info", "Show debug info", cxxopts::value<bool>()->default_value("false"))
   ("crashinfo", "On crash show info", cxxopts::value<bool>()->default_value("false"))
   ("k,skiploop", "Skip main game loop", cxxopts::value<bool>()->default_value("false"))
   ("b,bootstrapper", "Run the server in bootstrapper only", cxxopts::value<bool>()->default_value("false"))
   ("n,noinput", "Disable default input (still allows custom input handling in scripts)", cxxopts::value<bool>()->default_value("false"))
   ("g,grid", "Size of grid chunking grid used for open world streaming, default to zero (no grid)", cxxopts::value<int>()->default_value("0"))
   ("w,world", "Use streaming chunk system", cxxopts::value<std::string>()->default_value(""))
   ("r,rawscene", "Rawscene file to use.  Set tags by adding =tag1,tag2,tag3 (optionally) and then :name to name", cxxopts::value<std::vector<std::string>>() -> default_value(""))
   ("a,args", "Args to provide to cscript", cxxopts::value<std::vector<std::string>>() -> default_value(""))
   ("m,mapping", "Key mapping file to use", cxxopts::value<std::string>()->default_value(""))
   ("l,benchmark", "Benchmark file to write results", cxxopts::value<std::string>()->default_value(""))
   ("e,timetoexit", "Time to run the engine before exiting in ms", cxxopts::value<int>()->default_value("0"))
   ("q,headlessmode", "Hide the window of the game engine", cxxopts::value<bool>()->default_value("false"))
   ("z,layers", "Layers file to specify render layers", cxxopts::value<std::string>() -> default_value("./res/layers.layerinfo"))
   ("test-unit", "Run unit tests", cxxopts::value<bool>()->default_value("false"))
   ("test-integ", "Run integration tests", cxxopts::value<bool>()->default_value("false"))
   ("test-feature", "Run feature scene", cxxopts::value<std::string>()->default_value(""))
   ("rechunk", "Rechunk the world", cxxopts::value<int>()->default_value("0"))
   ("mods", "List of mod folders", cxxopts::value<std::vector<std::string>>() -> default_value(""))
   ("font", "Default font to use", cxxopts::value<std::vector<std::string>>()->default_value("./res/textures/fonts/gamefont"))
   ("log", "List of logs to display", cxxopts::value<std::vector<std::string>>() -> default_value(""))
   ("loglevel", "Log level", cxxopts::value<int>()->default_value("0"))
   ("sqlshell", "Launch into sql shell", cxxopts::value<bool>()->default_value("false"))
   ("watch", "Watch file system for resource changes", cxxopts::value<std::string>()->default_value(""))
   ("reload", "Reload shaders", cxxopts::value<bool>()->default_value("false"))
   ("h,help", "Print help")
  ;        

  const auto result = cxxoption.parse(argc, argv);

  auto levels = result["log"].as<std::vector<std::string>>();
  modlogSetEnabled(levels.size() > 0, static_cast<MODLOG_LEVEL>(result["loglevel"].as<int>()), levels);

  auto benchmarkFile = result["benchmark"].as<std::string>();
  auto shouldBenchmark = benchmarkFile != "";
  setShouldProfile(shouldBenchmark);
  initializeStatistics();
  benchmark = createBenchmark(shouldBenchmark);

  auto runUnitTests = result["test-unit"].as<bool>();
  if (runUnitTests){
    auto returnVal = runTests();
    exit(returnVal);
  }

  auto shouldRunIntegrationTests = result["test-integ"].as<bool>();
  auto featureTest = result["test-feature"].as<std::string>();
  if (featureTest == "help"){
    printFeatureSceneHelp();
    exit(0);
  }

  bool headlessmode = result["headlessmode"].as<bool>();
  int numChunkingGridCells = result["grid"].as<int>();

  showCrashInfo = result["crashinfo"].as<bool>();
  shouldReloadShaders = result["reload"].as<bool>();

  std::string worldfile = result["world"].as<std::string>();
  bool useChunkingSystem = worldfile != "";

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

  if (args.find("sqldir") != args.end()){
    sqlDirectory = args.at("sqldir");
  }
  if (result["sqlshell"].as<bool>()){
    return loopSqlShell(sqlDirectory);
  }

  auto filewatch = watchFiles(result["watch"].as<std::string>(), 1.f);

  interface = SysInterface {
    .loadCScript = [](std::string script, objid id, objid sceneId) -> void {
      loadCScript(id, script.c_str(), sceneId, bootStrapperMode, false);
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
    .modlayerFileExists = modlayerFileExists,
    .fontFamilyByName = fontFamilyByName,
    .drawLine = [](glm::vec3 fromPos, glm::vec3 toPos, glm::vec4 color) -> void {
      addLineToNextCycleTint(lineData, fromPos, toPos, false, -1, color, std::nullopt, std::nullopt);
    },
  };

  auto mods = result["mods"].as<std::vector<std::string>>();
  for (auto mod : mods){
    installMod(mod);
  }

  auto layers = parseLayerInfo(result["layers"].as<std::string>(), interface.readFile);
  auto rawScenes = result["rawscene"].as<std::vector<std::string>>();

  keyMapper = readMapping(result["mapping"].as<std::string>(), inputFns, interface.readFile);

  if (result["help"].as<bool>()){
    std::cout << cxxoption.help() << std::endl;
    return 0;
  }
  bootStrapperMode = result["bootstrapper"].as<bool>();

  shaderFolderPath = result["shader"].as<std::string>();
  auto textureFolderPath = result["texture"].as<std::string>();
  const std::string framebufferShaderPath = "./res/shaders/framebuffer";
  const std::string uiShaderPath = result["uishader"].as<std::string>();

  auto timetoexit = result["timetoexit"].as<int>();

  std::cout << "LIFECYCLE: program starting" << std::endl;

  state.fullscreen = result["fullscreen"].as<bool>(); // merge flags and world.state concept

  // have this before createing the state since depends on debuggerDrawer
  BulletDebugDrawer drawer(addLineNextCyclePhysicsDebug);
  debuggerDrawer = &drawer;
  debuggerDrawer -> setDebugMode(0);

  setInitialState(state, "./res/world.state", statistics.now, interface.readFile, result["noinput"].as<bool>()); 

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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

  renderingResources.framebuffers = generateFramebuffers(state.resolution.x, state.resolution.y);
  glBindFramebuffer(GL_FRAMEBUFFER, renderingResources.framebuffers.fbo);
  setActiveDepthTexture(renderingResources.framebuffers.fbo, &renderingResources.framebuffers.depthTextures.at(0), 0);

  renderingResources.voxelLighting = generateUniformBuffer(sizeof(glm::vec4) * 512);
  {
    int zeroBuffer[512] = {};
    for (int i = 0; i < 512; i++){
      zeroBuffer[i] = -1;
    }
   // std::cout << "voxel lighting light : ";
    for (int i = 0; i < 512; i++){
      updateBufferData(renderingResources.voxelLighting, sizeof(glm::vec4) * i, sizeof(int), &zeroBuffer[i]);
    //  std::cout << zeroBuffer[i] << " ";
    }
    //std::cout << std::endl;
  }


  if (!glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE){
    std::cerr << "ERROR: framebuffer incomplete" << std::endl;
    return -1;
  }

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
     updateFramebufferWindowSizeChange(renderingResources.framebuffers, state.resolution.x, state.resolution.y);
  }; 

  onFramebufferSizeChange(window, state.currentScreenWidth, state.currentScreenHeight);
  glfwSetFramebufferSizeCallback(window, onFramebufferSizeChange); 
  glfwSetWindowSizeCallback(window, windowSizeCallback);
  glfwSetWindowPosCallback(window, windowPositionCallback);

  glPointSize(10.f);

  modlog("shaders", std::string("shader file path is ") + shaderFolderPath);
  unsigned int* shaderProgram = loadShaderIntoCache("default", shaderFolderPath + "/vertex.glsl", shaderFolderPath + "/fragment.glsl", interface.readFile, getTemplateValues());
  
  modlog("shaders", std::string("framebuffer file path is ") + framebufferShaderPath);
  renderingResources.framebufferProgram = loadShaderIntoCache("framebuffer", framebufferShaderPath + "/vertex.glsl", framebufferShaderPath + "/fragment.glsl", interface.readFile, getTemplateValues());

  std::string depthShaderPath = "./res/shaders/depth";
  modlog("shaders", std::string("depth file path is ") + depthShaderPath);
  unsigned int* depthProgram = loadShaderIntoCache("depth", depthShaderPath + "/vertex.glsl", depthShaderPath + "/fragment.glsl", interface.readFile, getTemplateValues());

  modlog("shaders", std::string("ui shader file path is ") + uiShaderPath);
  renderingResources.uiShaderProgram = loadShaderIntoCache("ui", uiShaderPath + "/vertex.glsl",  uiShaderPath + "/fragment.glsl", interface.readFile, getTemplateValues());

  std::string selectionShaderPath = "./res/shaders/selection";
  modlog("shaders", std::string("selection shader path is ") + selectionShaderPath);
  unsigned int* selectionProgram = loadShaderIntoCache("selection", selectionShaderPath + "/vertex.glsl", selectionShaderPath + "/fragment.glsl", interface.readFile, getTemplateValues());

  std::string blurShaderPath = "./res/shaders/blur";
  modlog("shaders", std::string("blur shader path is: ") + blurShaderPath);
  unsigned int* blurProgram = loadShaderIntoCache("blur", blurShaderPath + "/vertex.glsl", blurShaderPath + "/fragment.glsl", interface.readFile, getTemplateValues());

  mainShaders = RenderShaders {
    .blurProgram = blurProgram,
    .selectionProgram = selectionProgram,
    .uiShaderProgram = renderingResources.uiShaderProgram,
    .shaderProgram = shaderProgram,
  };
  renderStages = loadRenderStages(
    renderingResources.framebuffers.fbo, 
    renderingResources.framebuffers.framebufferTexture, renderingResources.framebuffers.framebufferTexture2, renderingResources.framebuffers.framebufferTexture3, renderingResources.framebuffers.framebufferTexture4,
    &renderingResources.framebuffers.depthTextures.at(0), renderingResources.framebuffers.depthTextures.size(),
    &renderingResources.framebuffers.portalTextures.at(0), renderingResources.framebuffers.portalTextures.size(),
    mainShaders,
    interface.readFile,
    getTemplateValues()
  );

  CustomApiBindings pluginApi{
    .listSceneId = listSceneId,
    .loadScene = loadScene,
    .unloadScene = unloadScene,
    .resetScene = resetScene,
    .listScenes = listScenes,
    .listSceneFiles = listSceneFiles,
    .sceneIdByName = sceneIdByName,
    .sceneNameById = sceneNameById,
    .listObjAndDescInScene = listObjAndDescInScene,
    .getChildrenIdsAndParent = getChildrenIdsAndParent,
    .rootSceneId = rootSceneId,
    .scenegraph = scenegraph,
    .sendLoadScene = sendLoadScene,
    .createScene = createScene,
    .deleteScene = deleteScene,
    .getCameraTransform = getCameraTransform,
    .moveCameraTo = moveCameraTo,
    .idsInGroupById = idsInGroupById,
    .groupId = groupId,
    .removeObjectById = removeObjectById,
    .removeByGroupId = removeByGroupId,
    .getObjectsByType = getObjectsByType,
    .getObjectsByAttr = getObjectsByAttr,
    .setActiveCamera = setActiveCamera,
    .getActiveCamera = getActiveCamera,
    .getView = getView,
    .drawText = drawText,
    .getTextDimensionsNdi = getTextDimensionsNdi,
    .drawRect = drawRect,
    .drawLine2D = drawLine2D,
    .drawLine = addLineNextCycle,
    .freeLine = [](objid lineId) -> void { freeLine(lineData, lineId); } ,
    .shaderByName = shaderByName,
    .loadShader = [](std::string name, std::string path) -> unsigned int* {
      return loadShaderIntoCache(name, path + "/vertex.glsl", path + "/fragment.glsl", interface.readFile, getTemplateValues());
    },
    .setShaderUniform = setUniformData,
    .getTextureSamplerId = getTextureSamplerId,

    .getGameObjNameForId = getGameObjectName,
    .setGameObjectAttr = setGameObjectAttr,
    .setSingleGameObjectAttr = setSingleGameObjectAttr,
    .getGameObjectPos = getGameObjectPosition,
    .setGameObjectPosition = setGameObjectPosition,
    .getGameObjectRotation = getGameObjectRotation,
    .setGameObjectRot = setGameObjectRotation,
    .getGameObjectScale = getGameObjectScale2,
    .setGameObjectScale = setGameObjectScale,
    .setFrontDelta = setFrontDelta,
    .moveRelative = moveRelative,
    .moveRelativeVec = moveRelative,
    .orientationFromPos = orientationFromPos,
    .getGameObjectByName = getGameObjectByName,
    .applyImpulse = applyImpulse,
    .applyImpulseRel = applyImpulseRel,
    .clearImpulse = clearImpulse,
    .applyForce = applyForce,
    .applyTorque = applyTorque,
    .getModAABB = getModAABB,
    .listAnimations = listAnimations,
    .playAnimation = playAnimation,
    .stopAnimation = stopAnimation,
    .disableAnimationIds = disableAnimationIds,
    .setAnimationPose = setAnimationPose,
    .listClips = listSounds,
    .playClip = playSoundState,
    .playClipById = playSoundState,
    .stopClip = stopSoundState,
    .listResources = listResources,
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
    .recordingLength = recordingLength,
    .recordingState = recordingState,
    .createRecording = createRecording,
    .saveRecording = saveRecording,
    .makeObjectAttr = makeObjectAttr,
    .makeParent = makeParent,
    .getParent = getParent,
    .raycast = raycastW,
    .contactTest = contactTest,
    .contactTestShape = contactTestShape,
    .saveScreenshot = takeScreenshot,
    .setState = setState,
    .setFloatState = setFloatState,
    .setIntState = setIntState,
    .navPosition = navPosition,
    .emit = emit,
    .loadAround = addLoadingAround,
    .rmLoadAround = removeLoadingAround,
    .generateMesh = createGeneratedMesh,
    .generateMeshRaw = createGeneratedMeshRaw,
    .getArgs = getArgs,
    .lock = lock,
    .unlock = unlock,
    .debugInfo = debugInfo,
    .setWorldState = setWorldState,
    .getWorldState = getWorldState,
    .setLayerState = setLayerState,
    .createTexture = createTexture,
    .freeTexture = freeTexture,
    .clearTexture = clearTexture,
    .runStats = statValue,
    .statValue = statValue,
    .stat = statName,
    .logStat = registerStat,
    .installMod = installMod,
    .uninstallMod = uninstallMod,
    .listMods = listMods,
    .compileSqlQuery = sql::compileSqlQuery,
    .executeSqlQuery = executeSqlQuery,
    .selected = []() -> std::vector<objid> {
      return selectedIds(state.editor);
    },
    .setSelected = setSelected,
    .click = dispatchClick,
    .moveMouse = moveMouse,
    .schedule = schedule,
    .getFrameInfo = getFrameInfo,
    .getCursorInfoWorld = getCursorInfoWorld,
    .idAtCoordAsync = idAtCoordAsync,
    .gameobjExists = gameobjExists,
    .prefabId = prefabId,
    .positionToNdi = positionToNdi,
    .setLogEndpoint = setLogEndpoint,
    .getClipboardString = getClipboardString,
    .setClipboardString = setClipboardString,

    .saveState = saveState,
    .loadState = loadState,

    .getVoxelLightingData = getVoxelLightingData,
    .dumpDebugInfo = dumpDebugInfo,
  };


  mainApi = &pluginApi;

  std::vector<CScriptBinding> pluginBindings = { 
    cscriptCreatePerformanceGraphBinding(pluginApi),
    cscriptCreatePerfVisualizeBinding(pluginApi),
  };


  if (featureTest != ""){
    FeatureScene& featureScene = getFeatureScene(featureTest);
    if (featureScene.createBinding.has_value()){
      pluginBindings.push_back(featureScene.createBinding.value()(pluginApi));
    }
  }


  #ifdef ADDITIONAL_SRC_HEADER
    auto userBindings = getUserBindings(pluginApi);
    for (auto userBinding : userBindings){
      pluginBindings.push_back(userBinding);
    }
  #endif
  registerAllBindings(pluginBindings);
  cBindings = getCScriptBindingCallbacks();

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
    "./res/models/controls/unitxy.obj",  
    "../gameresources/build/objtypes/emitter.gltf",  
    "../gameresources/build/objtypes/sound.gltf",
    "../gameresources/build/objtypes/light.gltf",
  };

  std::vector<std::string> allTexturesToLoad = {  "./res/textures/crosshairs/crosshair029.png", "./res/textures/crosshairs/crosshair008.png" };

  world = createWorld(
    onObjectEnter, 
    onObjectLeave, 
    [](GameObject& obj) -> void {
      netObjectUpdate(world, obj, netcode, bootStrapperMode);
    }, 
    [](GameObject& obj) -> void {
      netObjectCreate(world, obj, netcode, bootStrapperMode);
      cBindings.onObjectAdded(obj.id);
    },
    [](objid id, bool isNet) -> void {
      std::cout << "deleted obj id: " << id << std::endl;
      maybeResetCamera(id);
      unsetSelectedIndex(state.editor, id, true);
      if (getSelectedOctreeId().has_value() && getSelectedOctreeId().value() == id){
        setSelectedOctreeId(std::nullopt);
      }
      removeScheduledTaskByOwner({ id });
      netObjectDelete(id, isNet, netcode, bootStrapperMode);
      cBindings.onObjectRemoved(id);
      freeTexture(id);
    }, 
    debuggerDrawer, 
    layers,
    interface,
    defaultMeshesToLoad,
    allTexturesToLoad
  );
  loadAllTextures(textureFolderPath);
  if (state.skybox != ""){
    loadSkybox(world, state.skybox); 
  }

  bool fpsFixed = result["fps-fixed"].as<bool>();
  statistics.initialTime = fpsFixed  ? 0 : glfwGetTime();
  timings = createWorldTiming(statistics.initialTime);

  auto fontPaths = result["font"].as<std::vector<std::string>>();
  std::cout << "INFO: FONT: loading font paths (" << fontPaths.size() <<") - " << print(fontPaths) << std::endl;
  defaultResources = DefaultResources {
    .defaultCamera = GameObject {
      .id = -1,
      .name = "defaultCamera",
      .transformation = Transformation {
        .position = glm::vec3(0.f, 0.f, 0.f),
        .scale = glm::vec3(1.0f, 1.0f, 1.0f),
        .rotation = parseQuat(glm::vec4(0.f, 0.f, -1.f, 0.f)),
      }  // default resource
    },
    .quadVAO = loadFullscreenQuadVAO(),
    .quadVAO3D = loadFullscreenQuadVAO3D(),
    .fontFamily = loadFontMeshes(readFontFile(fontPaths), world.textures.at("./res/textures/wood.jpg").texture),
    .defaultMeshes = DefaultMeshes{
      .nodeMesh = &world.meshes.at("./res/models/ui/node.obj").mesh,
      .portalMesh = &world.meshes.at("./res/models/box/plane.dae").mesh,
      .cameraMesh = &world.meshes.at("../gameresources/build/objtypes/camera.gltf").mesh, 
      .voxelCubeMesh = &world.meshes.at("./res/models/unit_rect/unit_rect.obj").mesh,
      .unitXYRect = &world.meshes.at("./res/models/controls/unitxy.obj").mesh,
      .soundMesh = &world.meshes.at("../gameresources/build/objtypes/sound.gltf").mesh,
      .lightMesh = &world.meshes.at("../gameresources/build/objtypes/light.gltf").mesh,
      .emitter = &world.meshes.at("../gameresources/build/objtypes/emitter.gltf").mesh,
      .nav = &world.meshes.at("./res/models/ui/node.obj").mesh,
    }
  };

  dynamicLoading = createDynamicLoading(worldfile, interface.readFile);
  if (result["rechunk"].as<int>()){
    rechunkAllObjects(world, dynamicLoading, result["rechunk"].as<int>());
    return 0;
  }

  for (auto script : result["scriptpath"].as<std::vector<std::string>>()){
    loadCScript(getUniqueObjId(), script.c_str(), -1, bootStrapperMode, true);
  }
  
  std::cout << "INFO: # of intitial raw scenes: " << rawScenes.size() << std::endl;
  for (auto parsedScene : parseSceneArgs(rawScenes)){
    loadScene(parsedScene.sceneToLoad, {}, parsedScene.sceneFileName, parsedScene.tags);
  }

  GLFWimage images[1]; 
  images[0].pixels = stbi_load(state.iconpath.c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels 
  glfwSetWindowIcon(window, 1, images);
  stbi_image_free(images[0].pixels); 

  glfwSetCursorPosCallback(window, onMouseEvents); 
  glfwSetMouseButtonCallback(window, onMouseCallback);

  if (glfwRawMouseMotionSupported()){
    // glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  }else{
    modassert(false, "raw mouse not supported");
  }

  glfwSetScrollCallback(window, onScrollCallback);
  glfwSetKeyCallback(window, keyCallback);
  glfwSetCharCallback(window, keyCharCallback);
  glfwSetDropCallback(window, drop_callback);
  glfwSetJoystickCallback(joystickCallback);
  glfwSwapInterval(state.swapInterval);
  toggleFullScreen(state.fullscreen);
  toggleCursor(state.cursorBehavior); 

  std::cout << "INFO: render loop starting" << std::endl;
  GLenum buffers_to_render[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, buffers_to_render);

  int frameratelimit = result["fps"].as<int>();
  bool hasFramelimit = frameratelimit != 0;
  float minDeltaTime = !hasFramelimit ? 0 : (1.f / frameratelimit);

  float fixedFps = 60.f;
  float fixedDelta = 1.f / fixedFps;
  float fpsLag = (result["fps-lag"].as<int>()) / 1000.f;
  std::cout << "speed multiplier: "  << state.engineSpeed << std::endl;

  assert(!hasFramelimit || !fpsFixed);
  assert(fpsLag < 0 || !fpsFixed);
  assert(!hasFramelimit || aboutEqual(state.engineSpeed, 1.f));
  assert(fpsLag < 0 || aboutEqual(state.engineSpeed, 1.f));

  const char* vendor = (const char*)(glGetString(GL_VENDOR)); // Returns the vendor
  const char* renderer = (const char*)(glGetString(GL_RENDERER)); // Returns a hint to the mode

  modlog("gpu info vendor", std::string(vendor));
  modlog("gpu info renderer", std::string(renderer));

  if (featureTest != ""){
    runFeatureScene(featureTest);
  }

  bool shouldQuitControl = false;
  if (result["skiploop"].as<bool>()){
    goto cleanup;
  }

  state.cullEnabled ? glEnable(GL_CULL_FACE) : glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunci(1, GL_ONE, GL_ZERO);

  {
    auto firstFrame = std::chrono::steady_clock::now();
    std::chrono::milliseconds timeSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(firstFrame - start);
    modlog("loading time first frame (ms)", std::to_string(timeSeconds.count()));
  }

  PROFILE("MAINLOOP",
  while (!glfwWindowShouldClose(window)){
  PROFILE("FRAME",
    auto shouldExit = updateTime(fpsFixed, fixedDelta, state.engineSpeed, timetoexit, hasFramelimit, minDeltaTime, fpsLag);
    if (shouldExit || shouldQuitControl){
      goto cleanup;
    }
    resetReservedId();
    disposeTempBufferedData(lineData);
    doRemoveQueuedRemovals();
    doUnloadScenes();
    registerStatistics();
    if (shouldReloadShaders && ((getTotalTime() - lastReloadTime) > 5.f)){
      reloadShaders(interface.readFile, getTemplateValues());
      lastReloadTime = getTotalTime();
    }

    for (auto &idCoordToGet : idCoordsToGet){
      idCoordToGet.afterFrame(idCoordToGet.result, idCoordToGet.resultUv);
    }
    idCoordsToGet = {};

    if (!state.worldpaused){
      timePlayback.setElapsedTime(statistics.deltaTime);  // tick animations here
    }

    tickRecordings(getTotalTimeGame());
    tickScheduledTasks();

    glfwPollEvents();
    handleInput(window);
    handleMouseEvents();

    cBindings.onFrame();

    onNetCode(world, netcode, onClientMessage, bootStrapperMode);
    { 
      auto forward = calculateRelativeOffset(viewTransform.rotation, {0, 0, -1 }, false);
      auto up  = calculateRelativeOffset(viewTransform.rotation, {0, 1, 0 }, false);
      setListenerPosition(
        viewTransform.position.x, viewTransform.position.y, viewTransform.position.z,
        { forward.x, forward.y, forward.z},
        { up.x, up.y, up.z }
      );
      setVolume(state.muteSound ? 0.f : state.volume);
    }

    onWorldFrame(world, statistics.deltaTime, timePlayback.currentTime, state.enablePhysics, state.worldpaused, viewTransform, state.inputMode == ENABLED);
    handleMovingObjects(timePlayback.currentTime);

    cBindings.onFrameAfterUpdate();

    handleChangedResourceFiles(pollChangedFiles(filewatch, glfwGetTime()));
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


    if (shouldRunIntegrationTests){
      static TestRunInformation integrationTests = createIntegrationTest();
      auto testingComplete = runIntegrationTests(integrationTests);
      if (testingComplete){
        auto testingComplete = runIntegrationTests(integrationTests);
        std::cout << print(testResultsStr(integrationTests.testResults.value())) << std::endl;
        shouldQuitControl = true;
      }
    }

    auto adjustedCoords = pixelCoordsRelativeToViewport(state.cursorLeft, state.cursorTop, state.currentScreenHeight, state.viewportSize, state.viewportoffset, state.resolution);

    bool selectItemCalledThisFrame = selectItemCalled;
    selectItemCalled = false;  // reset the state
    auto selectTargetId = state.forceSelectIndex == 0 ? state.currentHoverIndex : state.forceSelectIndex;
    auto shouldSelectItem = selectItemCalledThisFrame || (state.forceSelectIndex != 0);
    state.forceSelectIndex = 0; // stateupdate

    bool shouldCallBindingOnObjectSelected = false;
    if ((selectTargetId != getManipulatorId(state.manipulatorState)) && shouldSelectItem){
      std::cout << "INFO: select item called" << std::endl;

      std::cout << "select target id: " << selectTargetId << std::endl;
      if (idExists(world.sandbox, selectTargetId)){
        std::cout << "INFO: select item called -> id in scene!" << std::endl;
        auto layerSelectIndex = getLayerForId(selectTargetId).selectIndex;

        auto layerSelectNegOne = layerSelectIndex == -1;
        std::cout << "cond1 = " << (layerSelectNegOne ? "true" : "false") << ", condtwo = " << ", selectindex " << layerSelectIndex <<  std::endl;
        if (!(layerSelectNegOne) && !state.selectionDisabled){
          shouldCallBindingOnObjectSelected = selectItem(selectTargetId, layerSelectIndex, getGroupId(world.sandbox, selectTargetId), state.cursorBehavior != CURSOR_HIDDEN || state.showCursor );
        }
      }else if (isReservedObjId(selectTargetId)){
        onObjectSelected(selectTargetId);
      }else{
        std::cout << "INFO: select item called -> id not in scene! - " << selectTargetId<< std::endl;
        onObjectUnselected();
        cBindings.onObjectUnselected();
      }
    }


    if (shouldCallBindingOnObjectSelected){
      auto id = state.groupSelection ? getGroupId(world.sandbox, selectTargetId) : selectTargetId;
      modassert(idExists(world.sandbox, id), "id does not exist for objectSelected");
      cBindings.onObjectSelected(id, state.hoveredColor.value());
    }

    if (state.lastHoverIndex != state.currentHoverIndex){  
      if (idExists(world.sandbox, state.lastHoverIndex)){
        cBindings.onObjectHover(state.lastHoverIndex, false);
      }
      if (idExists(world.sandbox, state.currentHoverIndex)){
        cBindings.onObjectHover(state.currentHoverIndex, true);
      }
    }
    
    static auto manipulatorLayer = layerByName("");
    onManipulatorUpdate(
      state.manipulatorState, 
      projectionFromLayer(manipulatorLayer),
      view, 
      state.manipulatorMode, 
      state.manipulatorAxis,
      state.offsetX, 
      state.offsetY,
      glm::vec2(adjustedCoords.x, adjustedCoords.y),
      glm::vec2(state.resolution.x, state.resolution.y),
      ManipulatorOptions {
         .manipulatorPositionMode = state.manipulatorPositionMode,
         .relativePositionMode = state.relativePositionMode,
         .translateMirror = state.translateMirror,
         .rotateMode = state.rotateMode,
         .scalingGroup = state.scalingGroup,
         .snapManipulatorScales = state.snapManipulatorScales,
         .preserveRelativeScale = state.preserveRelativeScale,
      },
      tools,
      !(state.inputMode == ENABLED)
    );      
    

    if (state.shouldToggleCursor){
      modlog("toggle cursor", std::to_string(state.cursorBehavior));
      toggleCursor(state.cursorBehavior);
      state.shouldToggleCursor = false;
    }  

    afterFrameForScripts();

    while (!channelMessages.empty()){
      auto message = channelMessages.front();
      channelMessages.pop();
      if (message.strTopic == "copy-object"){  // should we have any built in messages supported like this?
        handleClipboardSelect();
        handleCopy();
      }
      cBindings.onMessage(message.strTopic, message.strValue);
    }

    viewTransform = getCameraTransform();
    view = renderView(viewTransform.position, viewTransform.rotation);
    std::vector<LightInfo> lights = getLightInfo(world);
    std::vector<PortalInfo> portals = getPortalInfo(world);
    assert(portals.size() <= renderingResources.framebuffers.portalTextures.size());

    if (state.visualizeNormals){
      forEveryGameobj(world.sandbox, [](objid id, GameObject& gameobj) -> void {
        if (id == getGroupId(world.sandbox, id)){
          auto transform = fullTransformation(world.sandbox, id);
          auto toPosition = transform.position + (transform.rotation * glm::vec3(0.f, 0.f, -1.f));
          auto leftArrow = transform.position + (transform.rotation * glm::vec3(-0.2f, 0.f, -0.8f));
          auto rightArrow = transform.position + (transform.rotation * glm::vec3(0.2f, 0.f, -0.8f));

          interface.drawLine(transform.position, toPosition, glm::vec4(1.f, 0.f, 0.f, 1.f));
          interface.drawLine(leftArrow, toPosition, glm::vec4(1.f, 0.f, 0.f, 1.f));
          interface.drawLine(rightArrow, toPosition, glm::vec4(1.f, 0.f, 0.f, 1.f));
        }
      });
    }

    //////////////////////// rendering code below ///////////////////////
    
    RenderContext renderContext {
      .world = world,
      .view = view,
      .lights = lights,
      .portals = portals,
      .lightProjview = {},
      .cameraTransform = viewTransform,
      .projection = std::nullopt,
    };

    bool depthEnabled = false;
    auto dofInfo = getDofInfo(&depthEnabled);
    updateRenderStages(renderStages, dofInfo);
    glViewport(0, 0, state.currentScreenWidth, state.currentScreenHeight);


    PROFILE("SELECTION",
      // outputs to FBO unique colors based upon ids. This eventually passed in encodedid to all the shaders which is how color is determined
      renderWithProgram(renderContext, renderStages.selection);

      shaderSetUniform(*renderStages.selection.shader, "projview", ndiOrtho);
      glDisable(GL_DEPTH_TEST);
      drawShapeData(lineData, *renderStages.selection.shader, ndiOrtho, fontFamilyByName, std::nullopt,  state.currentScreenHeight, state.currentScreenWidth, *defaultResources.defaultMeshes.unitXYRect, getTextureId, true);
      glEnable(GL_DEPTH_TEST);

    )

    //std::cout << "adjusted coords: " << print(adjustedCoords) << std::endl;
    auto uvCoordWithTex = getUVCoordAndTextureId(adjustedCoords.x, adjustedCoords.y);
    auto uvCoord = toUvCoord(uvCoordWithTex);
    Color hoveredItemColor = getPixelColor(adjustedCoords.x, adjustedCoords.y);
    objid hoveredId = getIdFromColor(hoveredItemColor);

    state.lastHoverIndex = state.currentHoverIndex; // stateupdate
    state.currentHoverIndex = hoveredId; // stateupdate
    state.hoveredItemColor = glm::vec3(hoveredItemColor.r, hoveredItemColor.g, hoveredItemColor.b); // stateupdate

    for (auto &idCoordToGet : idCoordsToGet){
      if (idCoordToGet.textureId.has_value()){
        continue;
      }
      auto pixelCoord = ndiToPixelCoord(glm::vec2(idCoordToGet.ndix, idCoordToGet.ndiy), state.resolution);

      auto id = getIdFromPixelCoord(pixelCoord.x, pixelCoord.y);
      if (id == -16777216){  // this is kind of shitty, this is black so represents no object.  However, theoretically could be an id, should make this invalid id
      }else if (idCoordToGet.onlyGameObjId && !idExists(world.sandbox, id)){
        //modassert(false, std::string("id does not exist: ") + std::to_string(id));
      }else{
        idCoordToGet.result = id;
      }

      auto uvCoordWithTex = getUVCoordAndTextureId(pixelCoord.x, pixelCoord.y);
      auto uvCoord = toUvCoord(uvCoordWithTex);
      idCoordToGet.resultUv = glm::vec2(uvCoord.x, uvCoord.y);
    }

    // depth buffer from point of view SMf 1 light source (all eventually, but 1 for now)
    std::vector<glm::mat4> lightMatrixs;
    if (state.enableShadows){
      PROFILE(
        "RENDERING-SHADOWMAPS",
        lightMatrixs = renderShadowMaps(renderContext);
      )
    }

    renderContext.lightProjview = lightMatrixs;

    assert(portals.size() <= renderingResources.framebuffers.portalTextures.size());
    PROFILE("PORTAL_RENDERING", 
      portalIdCache = renderPortals(renderContext);
    )
    //std::cout << "cache size: " << portalIdCache.size() << std::endl;

    statistics.numTriangles = renderWithProgram(renderContext, renderStages.main);
    Color colorFromSelection = getPixelColor(adjustedCoords.x, adjustedCoords.y);
    renderVector(*shaderProgram, view, numChunkingGridCells);

    Color pixelColor = getPixelColor(adjustedCoords.x, adjustedCoords.y);
    state.hoveredColor = glm::vec3(pixelColor.r, pixelColor.g, pixelColor.b);

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
    PROFILE("USER-TEXTURES",
      for (auto userTexture : screenspaceTextureIds){
        Texture tex {
          .textureId = userTexture.id,
        };
        Texture tex2 {
          .textureId = userTexture.selectionTextureId,
        };
        renderScreenspaceShapes(tex, tex2, userTexture.shouldClear || userTexture.autoclear, userTexture.clearColor, userTexture.clearTextureId);
      }
      markUserTexturesCleared();  // not really rendering, should pull this out
    )

    LayerInfo& layerInfo = layerByName("");
    float near = layerInfo.nearplane;
    float far = layerInfo.farplane;
    // depth buffer pass 
    {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      auto finalProgram = *depthProgram;
      glUseProgram(finalProgram); 
      glClearColor(0.f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      std::vector<UniformData> uniformData;
      uniformData.push_back(UniformData {
        .name = "near",
        .value = near,
      });
      uniformData.push_back(UniformData {
        .name = "far",
        .value = far,
      });
      uniformData.push_back(UniformData {
        .name = "depthTexture",
        .value = Sampler2D {
          .textureUnitId = 2,
        }
      });
      setUniformData(finalProgram, uniformData, {});
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.depthTextures.at(0));
      glViewport(state.viewportoffset.x, state.viewportoffset.y, state.viewportSize.x, state.viewportSize.y);
      glBindVertexArray(defaultResources.quadVAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);

      Color hoveredItemColor = getPixelColor(adjustedCoords.x, adjustedCoords.y);
      auto distanceComponent = hoveredItemColor.r;
      float distance = (distanceComponent * (far - near)) + near;
   
      //std::cout << "depth: " << distance << ", near = " << near << ", far = " << far << std::endl;
      state.currentCursorDepth = distance;

    }
    /////////////////////////////////////

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    auto finalProgram = (state.renderMode == RENDER_DEPTH) ? *depthProgram : *renderingResources.framebufferProgram;
    glUseProgram(finalProgram); 
    glClearColor(0.f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::vector<UniformData> uniformData;
    uniformData.push_back(UniformData {
      .name = "bloomAmount",
      .value = state.bloomAmount,
    });
    uniformData.push_back(UniformData {
      .name = "enableBloom",
      .value = state.enableBloom,
    });
    uniformData.push_back(UniformData {
      .name = "enableExposure",
      .value = state.enableExposure,
    });
    state.exposure = exposureAmount();
    uniformData.push_back(UniformData {
      .name = "exposure",
      .value = state.exposure,
    });
    uniformData.push_back(UniformData {
      .name = "enableFog",
      .value = state.enableFog,
    });
    uniformData.push_back(UniformData {
      .name = "enableGammaCorrection",
      .value = state.enableGammaCorrection,
    });
    uniformData.push_back(UniformData {
      .name = "near",
      .value = near,
    });
    uniformData.push_back(UniformData {
      .name = "far",
      .value = far,
    });
    uniformData.push_back(UniformData {
      .name = "fogColor",
      .value = state.fogColor,
    });
    uniformData.push_back(UniformData {
      .name = "mincutoff",
      .value = state.fogMinCutoff,
    });
    uniformData.push_back(UniformData {
      .name = "maxcuttoff",
      .value = state.fogMaxCutoff,
    });
    uniformData.push_back(UniformData {
      .name = "bloomTexture",
      .value = Sampler2D {
        .textureUnitId = 1,
      }
    });
    uniformData.push_back(UniformData {
      .name = "depthTexture",
      .value = Sampler2D {
        .textureUnitId = 2,
      }
    });
    uniformData.push_back(UniformData {
      .name = "framebufferTexture",
      .value = Sampler2D {
        .textureUnitId = 0,
      }
    });

    setUniformData(finalProgram, uniformData, {});

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.depthTextures.at(0));

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.framebufferTexture2);

    glActiveTexture(GL_TEXTURE0);

    if (state.borderTexture != ""){
      glViewport(0, 0, state.currentScreenWidth, state.currentScreenHeight);
      glBindTexture(GL_TEXTURE_2D, world.textures.at(state.borderTexture).texture.textureId);
      glDrawArrays(GL_TRIANGLES, 0, 6);
      glClear(GL_DEPTH_BUFFER_BIT);
    }

    if (state.renderMode == RENDER_FINAL){
      glBindTexture(GL_TEXTURE_2D, finalRenderingTexture(renderStages));
    }else if (state.renderMode == RENDER_PORTAL){
      assert(state.textureIndex <= renderingResources.framebuffers.portalTextures.size() && state.textureIndex >= 0);
      glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.portalTextures.at(state.textureIndex));  
    }else if (state.renderMode == RENDER_SELECTION){
      glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.framebufferTexture4);  
    }else if (state.renderMode == RENDER_PAINT){
      //glBindTexture(GL_TEXTURE_2D, textureToPaint);
      glBindTexture(GL_TEXTURE_2D, world.textures.at("gentexture-scenegraph_selection_texture").texture.textureId);
    }else if (state.renderMode == RENDER_DEPTH){
      assert(state.textureIndex <=  renderingResources.framebuffers.depthTextures.size() && state.textureIndex >= 0);
      glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.depthTextures.at(state.textureIndex));
    }else if (state.renderMode == RENDER_BLOOM){
      glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.framebufferTexture2);
    }else if (state.renderMode == RENDER_GRAPHS){
      if (screenspaceTextureIds.size() > state.textureIndex && state.textureIndex >= 0){
        glBindTexture(GL_TEXTURE_2D, screenspaceTextureIds.at(state.textureIndex).id);
      }else{
        modlog("rendering", (std::string("cannot display graph texture index: ") + std::to_string(state.textureIndex)).c_str());
      }
    }else if (state.renderMode == RENDER_TEXTURE){
       glBindTexture(GL_TEXTURE_2D, world.textures.at("gentexture-ingame-ui-texture-test").texture.textureId);  
    }
    glViewport(state.viewportoffset.x, state.viewportoffset.y, state.viewportSize.x, state.viewportSize.y);
    glBindVertexArray(defaultResources.quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, state.currentScreenWidth, state.currentScreenHeight);

  
    if (state.renderMode == RENDER_FINAL){
      renderUI(pixelColor);

      auto shader = *renderingResources.uiShaderProgram;

      // below and render screepspace lines can probably be consoliated
      glUseProgram(shader);
      std::vector<UniformData> uniformData;
      uniformData.push_back(UniformData {
        .name = "projection",
        .value = ndiOrtho,
      });
      uniformData.push_back(UniformData {
        .name = "forceTint",
        .value = false,
      });
      uniformData.push_back(UniformData {
        .name = "textureData",
        .value = Sampler2D {
          .textureUnitId = 0,
        },
      });
      setUniformData(shader, uniformData, { "model", "encodedid", "tint", "time" });
      glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glBlendFunci(1, GL_ONE, GL_ZERO);
      
      drawShapeData(lineData, shader /*renderingResources.uiShaderProgram*/, ndiOrtho, fontFamilyByName, std::nullopt,  state.currentScreenHeight, state.currentScreenWidth, *defaultResources.defaultMeshes.unitXYRect, getTextureId, false);
    }
    glEnable(GL_DEPTH_TEST);

    if (state.takeScreenshot){
      state.takeScreenshot = false;

      glBindTexture(GL_TEXTURE_2D, renderStages.main.colorAttachment0);
      saveScreenshot(state.screenshotPath);
    }

    std::cout << "frame time: " << (glfwGetTime() - statistics.now) << std::endl;
    std::cout << "frame draw calls: " << numberOfDrawCallsThisFrame << std::endl;
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
