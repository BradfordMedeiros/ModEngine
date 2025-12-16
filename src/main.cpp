// TODO
// TODO STATIC
// TODO PEROBJECT
// TODO PERF
// TODO CRITICAL FIX

#include <csignal>
#include <cxxopts.hpp>

#include "./main_input.h"
#include "./scene/common/vectorgfx.h"
#include "./scene/objtypes/lighting/scene_lighting.h"
#include "./network/modnet.h"
#include "./main_util.h"
#include "./cscript/cscripts/plugins/perf-visualize.h"
#include "./cscript/cscripts/plugins/performance_graph.h"
#include "./cscript/cscripts/plugins/tools.h"
#include "./scene/common/textures_gen.h"
#include "./sql/shell.h"
#include "./common/watch_file.h"
#include "./tests/main_test.h"
#include "./world_tasks.h"
#include "./shaderstate.h"
#include "./package.h"
#include "./resources.h"
#include "./scene/common/map.h"

#include "./scene/objtypes/emitter/effekseer.h"


#ifdef ADDITIONAL_SRC_HEADER
  #include STR(ADDITIONAL_SRC_HEADER)
#endif
CustomApiBindings* mainApi;

extern int numberOfDrawCallsThisFrame;

RenderingResources renderingResources { };
RenderShaders mainShaders {};

// long lived app resources
DefaultResources defaultResources {};
std::string shaderFolderPath;
std::string sqlDirectory = "./res/data/sql/";
bool bootStrapperMode = false;
bool strictResourceMode = false;
std::unordered_map<std::string, std::string> args;
extern std::vector<InputDispatch> inputFns;     

// per frame variable data 
bool selectItemCalled = false;
extern Stats statistics;
LineData lineData = createLines();
std::queue<StringAttribute> channelMessages;

std::unordered_map<objid, unsigned int> portalIdCache;

Transformation viewTransform {
  .position = glm::vec3(0.f, 0.f, 0.f),
  .scale = glm::vec3(1.f, 1.f, 1.f),
  .rotation = quatFromDirection(glm::vec3(0.f, 0.f, -1.f)),
};
Transformation cullingViewTransform = viewTransform;
glm::mat4 view;
glm::mat4 ndiOrtho = glm::ortho(-1.f, 1.f, -1.f, 1.f, -1.0f, 1.0f);  

struct ShaderToUpdate {
  unsigned int* shader; // TODO reload shouldnt this be a pointer if reload? 
  bool isUiShader;
  std::string name;
};
std::vector<ShaderToUpdate> extraShadersToUpdate; // TODO STATIC get rid of this, but useful given at render shader loading
std::unordered_map<unsigned int, std::vector<ShaderTextureBinding>> textureBindings; 
unsigned int* waterShader = NULL;

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
std::vector<IdAtCoords> idCoordsToGet;
std::vector<DepthAtCoord> depthsAtCoords;
std::string dataPath;

int transparencyLayer = -1;
int currentTick = 0;
bool showCrashInfo = false;
float lastReloadTime = 0.f;

TimePlayback timePlayback(statistics.initialTime); 
std::vector<ViewportSettings> viewports;


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

void renderScreenspaceShapes(Texture& texture, Texture texture2, bool shouldClear, glm::vec4 clearColor, std::optional<unsigned int> clearTextureId){
  auto texSize = getTextureSizeInfo(texture);
  auto texSize2 = getTextureSizeInfo(texture2);
  modassert(texSize.width == texSize2.width && texSize.height == texSize2.height, "screenspace - invalid tex sizes, texsize = " + print(glm::vec2(texSize.width, texSize.height)) + ", texsize2 = " + print(glm::vec2(texSize2.width, texSize2.height)));

  glViewport(0, 0, texSize.width, texSize.height);
  updateDepthTexturesSize(&renderingResources.framebuffers.textureDepthTextures.at(0), renderingResources.framebuffers.textureDepthTextures.size(), texSize.width, texSize.height); // wonder if this would be better off preallocated per gend texture?
  setActiveDepthTexture(renderingResources.framebuffers.fbo, &renderingResources.framebuffers.textureDepthTextures.at(0), 0);

  glBindFramebuffer(GL_FRAMEBUFFER, renderingResources.framebuffers.fbo);
  GLenum buffers_to_render[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
  glDrawBuffers(4, buffers_to_render);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.textureId, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,  texture2.textureId, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,  0, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,  0, 0);

  modassert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "framebuffer incomplete");

  glUseProgram(*renderingResources.uiShaderProgram);
  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  //modassert(false, "todo - make this work with setUniformData");

  if (shouldClear){ 
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |  GL_STENCIL_BUFFER_BIT);
  }

  if (shouldClear && clearTextureId.has_value()){
    auto model = glm::scale(glm::mat4(1.0f), glm::vec3(2.f, 2.f, 2.f));

    MeshUniforms meshUniforms {
      .model = model,
      .customTextureId = clearTextureId.value(),
      .tint = clearColor,
    };
    drawMesh(*defaultResources.defaultMeshes.unitXYRect, *renderingResources.uiShaderProgram, false, meshUniforms);
  }


  auto lineModelMatrix = glm::mat4(1.f);
  drawAllLines(lineData, *renderingResources.uiShaderProgram, texture.textureId, lineModelMatrix);

  glEnable(GL_BLEND);
  glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunci(1, GL_ONE, GL_ZERO);
  glBlendFunci(2, GL_ONE, GL_ZERO);
  glBlendFunci(3, GL_ONE, GL_ZERO);


  drawShapeData(lineData, *renderingResources.uiShaderProgram, fontFamilyByName, texture.textureId,  texSize.height, texSize.width, *defaultResources.defaultMeshes.unitXYRect, getTextureId, false);

  for (auto &idCoordToGet : idCoordsToGet){
    if (!idCoordToGet.textureId.has_value()){
      continue;
    }
    if (idCoordToGet.textureId.value() != texture.textureId){
      continue;
    }

    auto pixelCoord = ndiToPixelCoord(glm::vec2(idCoordToGet.ndix, idCoordToGet.ndiy), glm::vec2(texSize.width, texSize.height));
    auto id = getIdFromColor(getPixelColorAttachment0(pixelCoord.x, pixelCoord.y));
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

void onObjectEnter(const btCollisionObject* obj1, const btCollisionObject* obj2, glm::vec3 contactPos, glm::vec3 normal, float force){
  auto obj1Id = getIdForCollisionObject(world, obj1);
  auto obj2Id = getIdForCollisionObject(world, obj2);
  if (!gameobjExists(obj1Id.value()) || !gameobjExists(obj2Id.value())){
    return;
  }
  modassert(gameobjExists(obj1Id.value()), std::string("on object enter, obj1Id does not exist - rigidbody") + print((void*)obj1));
  modassert(gameobjExists(obj2Id.value()), std::string("on object enter, obj2Id does not exist - rigidbody") + print((void*)obj2));
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

std::string getRuntimePath(std::string path){
  return dataPath + path;
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

    "./res/textures/wood.jpg",

  };
  return octreeTextures;
}

std::vector<std::string> listLightTextures(){
  std::vector<std::string> lightTextures { 
    "./res/textures/grid.png", 
    "../gameresources/build/textures/arcade/invaders/background.png",

    "../gameresources/build/textures/clean/grass.jpg", 
    "../gameresources/build/textures/iguana.jpg", 
    "./res/textures/wood.jpg",
    "../gameresources/textures/backgrounds/test3.png",
    "../gameresources/textures/backgrounds/stainedglass.jpg",

  };
  return lightTextures; 
}

// This is wasteful, as obviously I shouldn't be loading in all the textures on load, but ok for now. 
// This shoiuld really just be creating a list of names, and then the cycle above should cycle between possible textures to load, instead of what is loaded 
void loadAllTextures(std::string& textureFolderPath){
  loadTextureWorld(world, "./res/models/box/grid.png", -1);
  loadTextureWorld(world, "./res/textures/wood.jpg", -1);
  loadTextureWorld(world, resources::GRID_TEXTURE, -1);

  for (auto texturePath : listFilesWithExtensionsFromPackage(textureFolderPath, { "png", "jpg" })){
    loadTextureWorld(world, texturePath, -1);
  }

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

  std::vector<std::string> lightTextures = listLightTextures();

  PROFILE("TEXTURES-LOAD-OCTREEATLAS",
    auto start = std::chrono::steady_clock::now();
    loadTextureAtlasWorld(world, "light-atlas", lightTextures, getRuntimePath("light_textures_atlas.png"), -1);
    loadTextureAtlasWorld(world, "octree-atlas:normal", normalTextures, getRuntimePath("test_atlas_normal.png"), -1);
    loadTextureAtlasWorld(world, "octree-atlas:main",   octreeTextures, getRuntimePath("test_atlas.png"), -1);
    setAtlasDimensions(AtlasDimensions {
      .textureNames = octreeTextures,
    });
    auto end = std::chrono::steady_clock::now();
    std::chrono::milliseconds timeSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    modlog("texture octree loading time(ms)", std::to_string(timeSeconds.count())); // eventually just cleanup perf to allow PROFILE to give this info generically
  )
}

// make this better, set more consistently
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
}

RenderObjApi api {};

 // very gross and coupled to game code, and just generally horrible
// but simple enough until have a better fix eg extended file syntax, naming convention, or templated shaders
bool checkIfUiShader(std::string& value){ 
  std::cout << "is ui shader: " << value << std::endl;
  if (value == "storyboard"){
    return true;
  }
  if (value == "arcade"){
    return true;
  }
  return false;
}


void visualizeFrustum(ViewFrustum& viewFrustum, Transformation& viewTransform, ViewportSettings& viewport){
  auto nearPlanePoint = viewTransform.position + viewFrustum.near.point;
  auto farPlanePoint = viewTransform.position + viewFrustum.far.point;

  std::vector<glm::vec3> points {
    nearPlanePoint,
    farPlanePoint,
  };


  auto angles = calcFovAngles(world.sandbox.layers.at(0),  getLayerFov(world.sandbox.layers.at(0)), calcViewportSize(viewport));

  auto position = viewTransform.position;  // viewFrustum
  {
    float widthXNear = glm::length(viewFrustum.near.point) * glm::tan(angles.x);
    float widthYNear = glm::length(viewFrustum.near.point) * glm::tan(angles.y);
    auto nearLeft = viewTransform.position + (viewTransform.rotation * (viewFrustum.near.point + glm::vec3(widthXNear, 0.f, 0.f)));
    auto nearRight = viewTransform.position + (viewTransform.rotation * (viewFrustum.near.point + glm::vec3(-widthXNear, 0.f, 0.f)));
    auto nearUp = viewTransform.position + (viewTransform.rotation * (viewFrustum.near.point + glm::vec3(0.f, widthYNear, 0.f)));
    auto nearDown = viewTransform.position + (viewTransform.rotation * (viewFrustum.near.point + glm::vec3(0.f, -widthYNear, 0.f)));

    float widthXFar = glm::length(viewFrustum.far.point) * glm::tan(angles.x);
    float widthYFar = glm::length(viewFrustum.far.point) * glm::tan(angles.y);
    auto farLeft = viewTransform.position + (viewTransform.rotation * (viewFrustum.far.point + glm::vec3(widthXFar, 0.f, 0.f)));
    auto farRight = viewTransform.position + (viewTransform.rotation * (viewFrustum.far.point + glm::vec3(-widthXFar, 0.f, 0.f)));
    auto farUp = viewTransform.position + (viewTransform.rotation * (viewFrustum.far.point + glm::vec3(0.f, widthYFar, 0.f)));
    auto farDown = viewTransform.position + (viewTransform.rotation * (viewFrustum.far.point + glm::vec3(0.f, -widthYFar, 0.f)));


    addLineToNextCycleTint(lineData, position, nearLeft, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, position, nearRight, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, position, nearUp, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, position, nearDown, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, nearLeft, nearUp, false, -1, glm::vec4(1.f, 1.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, nearUp, nearRight, false, -1, glm::vec4(1.f, 1.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, nearRight, nearDown, false, -1, glm::vec4(1.f, 1.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, nearDown, nearLeft, false, -1, glm::vec4(1.f, 1.f, 0.f, 1.f), std::nullopt, std::nullopt);


    addLineToNextCycleTint(lineData, position, farLeft, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, position, farRight, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, position, farUp, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, position, farDown, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, farLeft, farUp, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, farUp, farRight, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, farRight, farDown, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
    addLineToNextCycleTint(lineData, farDown, farLeft, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  }

  /*
  std::vector<FrustumPlane> planes {
    viewFrustum.near,
    viewFrustum.far,
    viewFrustum.top,
    viewFrustum.bottom,
    viewFrustum.left,
    viewFrustum.right,
  };
  8?
  //float widthXFar = glm::length(viewFrustum.far.point) * glm::tan(angleX);
  //addLineToNextCycleTint(lineData, position, position + viewFrustum.far.point + glm::vec3(widthXFar, 0.f, 0.f), false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  //addLineToNextCycleTint(lineData, position, position + viewFrustum.far.point + glm::vec3(-widthXFar, 0.f, 0.f), false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);

  //float length = 0.1f;
  //for (auto &plane : planes){
  //  break;
  //  auto normalVec = glm::normalize(quatFromDirection(plane.normal) * glm::vec3(0.f, 0.f, -1.f));
  //  //auto dirOnPlane = glm::cross(point.normal, normalVec);
  //  addLineToNextCycleTint(lineData, plane.point, plane.point + (length * normalVec), false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
  //
  //  auto perpendicularVector = glm::normalize(glm::cross(normalVec, normalVec + glm::vec3(1.f, 0.f, 0.f)));  // cross product and two vectors is perp
  //  addLineToNextCycleTint(lineData, plane.point, plane.point + (length * perpendicularVector), false, -1, glm::vec4(0.f, 1.f, 0.f, 1.f), std::nullopt, std::nullopt);
//
//  //  auto basisVector = glm::normalize(glm::cross(normalVec, perpendicularVector));
//  //  addLineToNextCycleTint(lineData, plane.point, plane.point + (length * basisVector), false, -1, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, std::nullopt);
  //}

 // for (auto &point : points){
 //   auto normalVec = quatFromDirection(point.normal) * glm::vec3(0.f, 0.f, -1.f);
 //   auto dirOnPlane = glm::cross(point.normal, normalVec);
 //   addLineToNextCycleTint(lineData, point, point + normalVec, false, -1, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, std::nullopt);
 //   addLineToNextCycleTint(lineData, point, point + dirOnPlane, false, -1, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, std::nullopt);
 // }*/
}

struct traversalData {
  objid id;
  objid directIndex;
  glm::mat4 modelMatrix;
};

objid directIndex(objid objectId){
  for (int i = 0; i < world.sandbox.mainScene.gameobjects.size(); i++){
    GameObjectBuffer& buffer = world.sandbox.mainScene.gameobjects.at(i);
    if (!buffer.inUse){
      continue;
    }
    if (buffer.gameobj.id == objectId){
      return i;
    }
  }
  modassert(false, "no fresh gameobject for this id");
  return 0;
}

bool sameLayer(std::string& layer1, std::string& layer2){
  return layer1 == layer2;
}

Mesh getTestMesh(){
  return world.meshes.at(resources::MODEL_NODE).mesh;
}

Texture getWaterTexture(){
  return world.textures.at(resources::TEXTURE_WATER).texture;
}

LayerInfo layerInfoForViewport(LayerInfo& layer, ViewportSettings& viewport){
  auto newLayer = layer;
  for (auto& layerOverride : viewport.layerOverride){
    if (layer.name == layerOverride.name){
      if (layerOverride.fovRaw.has_value()){
        newLayer.fovRaw = layerOverride.fovRaw.value();
      }
      if (layerOverride.visible.has_value()){
        newLayer.visible = layerOverride.visible.value();
      }
      break;
    }
  }
  return newLayer;
}

int renderWorld(World& world,  unsigned int* shaderProgram, bool allowShaderOverride, glm::mat4* projection, glm::mat4 view, std::vector<PortalInfo> portals, bool textBoundingOnly, ViewportSettings& viewport, bool renderParticles){
  glUseProgram(*shaderProgram);
  int numTriangles = 0;

  GLenum bufs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
  glDrawBuffers(4, bufs);

  auto viewFrustum = cameraToViewFrustum(world.sandbox.layers.at(0), getLayerFov(world.sandbox.layers.at(0)), calcViewportSize(viewport));

  if (state.cullingObject.has_value() && state.visualizeFrustum){
    visualizeFrustum(viewFrustum, cullingViewTransform, viewport);
  }

  std::optional<GLint> lastShaderId = std::nullopt;

  std::vector<traversalData> datum;
  datum.resize(world.sandbox.mainScene.idToDirectIndex.size());  // large # of elements, showed up in perf

  int i = 0;
  for (int directIndex = 0; directIndex < world.sandbox.mainScene.gameobjects.size(); directIndex++){
    auto &gameobjBuffer = world.sandbox.mainScene.gameobjects.at(directIndex);
    if (gameobjBuffer.inUse){
      datum[i] = traversalData{
        .id = gameobjBuffer.gameobj.id,
        .directIndex = directIndex,
        .modelMatrix = gameobjBuffer.absoluteTransform.matrix,
      };
      i++;
    }
  }
  modassert(i == world.sandbox.mainScene.idToDirectIndex.size(), "unexpect size mismatch");


  auto mainOctreeId = getMainOctreeId();
  for (auto& realLayer : world.sandbox.layers){      // @TODO could organize this before to not require pass on each frame
    auto layer = layerInfoForViewport(realLayer, viewport);

    std::cout << "layer fov: " << layer.fovRaw << std::endl;

    auto proj = projection == NULL ? projectionFromLayer(layer, viewport) : *projection;
    auto newProjView = (layer.orthographic ?  glm::ortho(-1.f, 1.f, -1.f, 1.f, 0.f, 100.0f) :  proj) * (layer.disableViewTransform ? glm::mat4(1.f) : view);

    bool isTransparencyLayer = layer.symbol == transparencyLayer;
    for (auto& data : datum){
      auto& gameobjBuffer = world.sandbox.mainScene.gameobjects.at(data.directIndex);
      GameObject& gameobject = gameobjBuffer.gameobj; 

      if (gameobject.layerSymbol == layer.symbol || (isTransparencyLayer && mainOctreeId.has_value() &&  mainOctreeId.value() == data.id)) {  // TODO PERF rm this string comparison 
        int32_t id = data.id; 
        glm::mat4& modelMatrix = data.modelMatrix; 
        std::string& shader = gameobject.shader;
        if (id < 0){
          modassert(false, "unexpected id render world");
        }

        // This could easily be moved to reduce opengl context switches since the onObject sorts on layers (so just have to pass down).  
        if (state.depthBufferLayer != layer.depthBufferLayer){
          state.depthBufferLayer = layer.depthBufferLayer;
          if(state.depthBufferLayer >= renderingResources.framebuffers.depthTextures.size()){
            modassert(false, std::string("invalid layer index: ") + std::to_string(state.depthBufferLayer) + std::string(" [") + layer.name + std::string("]"));
          }
          setActiveDepthTexture(renderingResources.framebuffers.fbo, &renderingResources.framebuffers.depthTextures.at(0), layer.depthBufferLayer);
          glClear(GL_DEPTH_BUFFER_BIT);
        }

        bool loadedNewShader = false;
        auto newShaderPtr = (shader == "" || !allowShaderOverride) ? shaderProgram : getShaderByShaderString(shader, shaderFolderPath, interface.readFile, getTemplateValues, &loadedNewShader);
        auto newShader = *newShaderPtr;
        if (loadedNewShader){
          auto isUiShader = checkIfUiShader(shader);
          extraShadersToUpdate.push_back(ShaderToUpdate {
            .shader = newShaderPtr,
            .isUiShader = checkIfUiShader(shader),
            .name = shader,
          });
          if(isUiShader){
            initUiShader(newShader);
          }else{
            initDefaultShader(newShader);
          }
        }
        if (!lastShaderId.has_value() || newShader != lastShaderId.value()){
          lastShaderId = newShader;
          glUseProgram(newShader);

          setRenderUniformData(newShader, layer.uniforms);
        }
        glProgramUniform1i(newShader, glGetUniformLocation(newShader, "enableLighting"), layer.lighting); // TODO - add type safety and stuff to this

        shaderSetUniform(newShader, "projview", newProjView);

        shaderSetUniform(*waterShader, "projview", newProjView);

        static glm::mat4 scaledModelMatrix(1.f); // copy assignent showed up in profiling, so just using static here so can prevent copy in most cases
        glm::mat4& finalModelMatrix = modelMatrix;
        if (layer.scale){
          scaledModelMatrix = calculateScaledMatrix(view, modelMatrix, getLayerFov(layer));
          finalModelMatrix = scaledModelMatrix;
        }
        bool isPortal = false;
        bool isPerspectivePortal = false;

        for (auto& portal : portals){
          if (id == portal.id){
            isPortal = true;
            isPerspectivePortal = portal.perspective;
            break;
          }
        }

        bool portalTextureInCache = portalIdCache.find(id) != portalIdCache.end();
        if (isPortal){
          glStencilMask(0xFF);
        }

        if (layer.visible && id != 0){
          //std::cout << "render object: " << getGameObject(world, id).name << std::endl;
          bool shouldDraw = true;
          if (state.enableFrustumCulling){
            auto aabb = getModAABBModel(id);
            if (aabb.has_value()){
              shouldDraw = passesFrustumCulling(viewFrustum, cullingViewTransform, aabb.value());
            }
          }
     
          if (shouldDraw){
            if (false && world.textures.find(resources::TEXTURE_WATER) != world.textures.end()){
              glActiveTexture(GL_TEXTURE0 + 7);
              glBindTexture(GL_TEXTURE_2D, getWaterTexture().textureId);
            }else{
              glActiveTexture(GL_TEXTURE0 + 7);
              glBindTexture(GL_TEXTURE_2D,  world.textures.at("light-atlas").texture.textureId);
            }


            auto trianglesDrawn = renderObject(  // slow 
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
              finalModelMatrix,
              gameobjBuffer.lookup,
              *waterShader,
              isTransparencyLayer,
              viewport.index
            );
            numTriangles = numTriangles + trianglesDrawn;
          }
        }
  
        if (isPortal && portalTextureInCache && isPerspectivePortal){
          glStencilFunc(GL_EQUAL, 1, 0xFF);
          glUseProgram(*renderingResources.framebufferProgram); 
          glDisable(GL_DEPTH_TEST);
          glBindVertexArray(defaultResources.quadVAO);
          glBindTexture(GL_TEXTURE_2D,  portalIdCache.at(id));
          glDrawArrays(GL_TRIANGLES, 0, 6);
          glEnable(GL_DEPTH_TEST);
          glUseProgram(newShader); 
          glStencilFunc(GL_ALWAYS, 1, 0xFF);
          glClear(GL_STENCIL_BUFFER_BIT);

        }
        if (isPortal){
          glStencilMask(0x00);
        }
      }
    }  
  }

  if (false && renderParticles){
    GLenum bufs[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, bufs);

    auto viewportDim = calcViewportSize(viewport);
    auto& layer = world.sandbox.layers.at(0);
    auto fov = getLayerFov(layer);
    PROFILE("EFFEK",
      static bool createdDummy = false;
      static GLuint dummyVAO = 0;
      if (!createdDummy){
        glGenVertexArrays(1, &dummyVAO);
        glBindVertexArray(dummyVAO);
        createdDummy = true;
      }
      glBindVertexArray(dummyVAO);
      
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, 0, 0);

      onEffekSeekerRender(static_cast<float>(viewportDim.x), static_cast<float>(viewportDim.y), glm::radians(fov), viewTransform.position, viewTransform.rotation,  layer.nearplane, layer.farplane);
    )    
  }

  auto maxExpectedClears = numUniqueDepthLayers(world.sandbox.layers);
  //modassert(numDepthClears <= maxExpectedClears, std::string("numDepthClears = ") + std::to_string(numDepthClears) + std::string(", expected = ") + std::to_string(maxExpectedClears));
 
  return numTriangles;
}

void renderVector(glm::mat4 view,  int numChunkingGridCells, ViewportSettings& viewport){
  glUseProgram(*mainShaders.shaderProgram);
  glDisable(GL_DEPTH_TEST);

  auto projection = projectionFromLayer(world.sandbox.layers.at(0), viewport);
  shaderSetUniform(*mainShaders.shaderProgram, "projview", (projection * view));

  // Draw grid for the chunking logic if that is specified, else lots draw the snapping translations
  if (state.showDebug && numChunkingGridCells > 0){
    float offset = ((numChunkingGridCells % 2) == 0) ? (dynamicLoading.mappingInfo.chunkSize / 2) : 0;
    auto lines = drawGrid3D(numChunkingGridCells, dynamicLoading.mappingInfo.chunkSize, offset, offset, offset);
    for (auto &line : lines){
      addLineNextCycle(line.fromPos, line.toPos, false, -1, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, std::nullopt);
    }
  }

  if (state.showDebug){
    auto lines = drawCoordinateSystem(100.f);
    for (auto &line : lines){
      addLineNextCycle(line.fromPos, line.toPos, false, -1, glm::vec4(0.f, 0.f, 1.f, 1.f), std::nullopt, std::nullopt);
    }
  }

  auto lineModelMatrix = glm::mat4(1.f);
  drawAllLines(lineData, *mainShaders.shaderProgram , std::nullopt, lineModelMatrix);
}

void renderSkybox(GLint shaderProgram, glm::mat4 view, ViewportSettings& viewport){
  auto projection = projectionFromLayer(world.sandbox.layers.at(0), viewport);

  auto value = glm::mat3(view);  // Removes last column aka translational component --> thats why when you move skybox no move!
  auto projview = projection * glm::mat4(value);

  glUseProgram(shaderProgram);
  shaderSetUniform(shaderProgram, "projview", projview);

  auto model = glm::mat4(1.f);
  MeshUniforms meshUniforms {
    .model = model,
    .tint = glm::vec4(state.skyboxcolor.x, state.skyboxcolor.y, state.skyboxcolor.z, 1.f),
  };
  drawMesh(world.meshes.at("skybox").mesh, shaderProgram, false, meshUniforms); 
}

void renderDebugUi(Color pixelColor){  
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


  auto ids = state.editor.selectedObjs;
  std::string selectedName = "no object selected";
  if (ids.size() > 0 && gameobjExists(ids.at(0))){
    auto selectedObject = getGameObject(world, ids.at(0));
    selectedName = selectedObject.name + "(" + std::to_string(selectedObject.id) + ")";  
  }


  std::string additionalText =  "     <" + std::to_string((int)(255 * state.hoveredItemColor.r)) + ","  + std::to_string((int)(255 * state.hoveredItemColor.g)) + " , " + std::to_string((int)(255 * state.hoveredItemColor.b)) + ">  " + " --- " + selectedName;
  drawTextNdi(std::to_string(currentFramerate) + additionalText, uiXOffset, uiYOffset + offsetPerLine, state.fontsize + 1);

  drawTextNdi("position: " + print(defaultResources.defaultCamera.transformation.position), uiXOffset, uiYOffset + offsetPerLine * 3, state.fontsize);
  drawTextNdi("rotation: " + print(defaultResources.defaultCamera.transformation.rotation), uiXOffset, uiYOffset + offsetPerLine * 4, state.fontsize);

  float ndiX = 2 * (state.cursorLeft / (float)state.resolution.x) - 1.f;
  float ndiY = -2 * (state.cursorTop / (float)state.resolution.y) + 1.f;

  drawTextNdi("cursor: (" + std::to_string(ndiX) + " | " + std::to_string(ndiY) + ") - " + std::to_string(state.cursorLeft) + " / " + std::to_string(state.cursorTop)  + "(" + std::to_string(state.resolution.x) + "||" + std::to_string(state.resolution.y) + ")", uiXOffset, uiYOffset + offsetPerLine * 5, state.fontsize);
  
  std::string position = "n/a";
  std::string worldPosition= "n/a";
  std::string scale = "n/a";
  std::string worldScale = "n/a";
  std::string rotation = "n/a";
  std::string worldRotation = "n/a";

  auto selectedValue = latestSelected(state.editor);
  if (selectedValue.has_value()){
    auto selectedIndex = selectedValue.value();
    auto transformation = gameobjectTransformation(world, selectedIndex, false, "renderDebugUI display info relative");
    position = print(transformation.position);
    scale = print(transformation.scale);
    rotation = serializeQuat(transformation.rotation);

    auto worldTransformation = gameobjectTransformation(world, selectedIndex, true, "renderDebugUI display info world");
    worldPosition = print(worldTransformation.position);
    worldScale = print(worldTransformation.scale);
    worldRotation = print(worldTransformation.rotation);
  }

  drawTextNdi("position: " + position + " / " + worldPosition, uiXOffset, uiYOffset + offsetPerLine * 6, state.fontsize);
  drawTextNdi("scale: " + scale + " / " + worldScale, uiXOffset, uiYOffset + offsetPerLine * 7, state.fontsize);
  drawTextNdi("rotation: " + rotation + worldRotation, uiXOffset, uiYOffset + offsetPerLine * 8, state.fontsize);
    

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
  drawTextNdi(std::string("time: ") + std::to_string(timeSeconds(false)), uiXOffset, uiYOffset + offsetPerLine * 21, state.fontsize);
  drawTextNdi(std::string("realtime: ") + std::to_string(timeSeconds(true)), uiXOffset, uiYOffset + offsetPerLine * 22, state.fontsize);
}


void onClientMessage(std::string message){

  
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
    realfiles::saveFile(crashFile, debugInfo);
    printBacktrace();
  }
}


struct RenderContext {
  glm::mat4 view;
  std::vector<PortalInfo> portals;
  std::optional<glm::mat4> projection;
};

int renderWithProgram(RenderContext& context, RenderStep& renderStep, ViewportSettings& viewport){
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
    if (renderStep.colorAttachment1.has_value()){
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, renderStep.colorAttachment1.value(), 0);
    }
    if (renderStep.colorAttachment2.has_value()){
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, renderStep.colorAttachment2.value(), 0);
    }
    if (renderStep.colorAttachment3.has_value()){
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, renderStep.colorAttachment3.value(), 0);
    }

    glClearColor(0.0, 0.0, 0.0, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |  GL_STENCIL_BUFFER_BIT);

    if (state.showSkybox && renderStep.renderSkybox){
       glDepthMask(GL_FALSE);
       renderSkybox(*renderStep.shader, context.view, viewport);  // Probably better to render this at the end 
       glDepthMask(GL_TRUE);    
    }


    glEnable(GL_DEPTH_TEST);
    if (renderStep.blend){
      glEnable(GL_BLEND);
      glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glBlendFunci(1, GL_ONE, GL_ZERO);
      glBlendFunci(2, GL_ONE, GL_ZERO);
      glBlendFunci(3, GL_ONE, GL_ZERO);
    }else{
      glDisable(GL_BLEND);
    }

    if (renderStep.renderQuad3D){
      std::vector<LightInfo> lights = {};
      glUseProgram(*renderStep.shader);
      // TODO - incomplete shader
      shaderSetUniform(*renderStep.shader, "tint", glm::vec4(1.f, 1.f, 1.f, 1.f));  
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
      auto worldTriangles = renderWorld(world, renderStep.shader, renderStep.allowShaderOverride, projection, context.view, context.portals, renderStep.textBoundingOnly, viewport, renderStep.renderParticles);
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

std::unordered_map<objid, unsigned int> renderPortals(RenderContext& context, Transformation cameraTransform, ViewportSettings& viewport){
  std::unordered_map<objid, unsigned int> nextPortalCache;
  for (int i = 0; i < context.portals.size(); i++){
    auto portal = context.portals.at(i);
    auto portalViewMatrix = renderPortalView(portal, cameraTransform);
    renderStagesSetPortal(renderStages, i);
    RenderContext portalRenderContext {
      .view = portalViewMatrix,
      .portals = context.portals,
      .projection = context.projection,
    };
    //std::cout << "portal transform:  " << i << " " << print(portal.cameraTransform.position) << std::endl;
    renderWithProgram(portalRenderContext, renderStages.portal, viewport);
    nextPortalCache[portal.id] = renderStages.portal.colorAttachment0;
  }
  //std::cout << std::endl;
  return nextPortalCache;
}

std::vector<glm::mat4> calcShadowMapViews(std::vector<LightInfo>& lights){
  std::vector<glm::mat4> lightMatrixs;
  for (int i = 0; i < lights.size(); i++){
    auto light = lights.at(i);
    auto lightView = renderView(light.transform.position, light.transform.rotation);
    glm::mat4 lightProjection = glm::ortho<float>(-2000, 2000,-2000, 2000, 1.f, 3000);  // need to choose these values better
    auto lightProjview = lightProjection * lightView;
    lightMatrixs.push_back(lightProjview);
  }
  return lightMatrixs;
}


void renderShadowMaps(RenderContext& context, std::vector<LightInfo>& lights, ViewportSettings& viewport){
  for (int i = 0; i < lights.size(); i++){
    auto light = lights.at(i);
    auto lightView = renderView(light.transform.position, light.transform.rotation);
    glm::mat4 lightProjection = glm::ortho<float>(-2000, 2000,-2000, 2000, 1.f, 3000);  // need to choose these values better
    auto lightProjview = lightProjection * lightView;

    RenderContext lightRenderContext {
      .view = lightView,
      .portals = context.portals,
      .projection = lightProjection,
    };
    renderStagesSetShadowmap(renderStages, i);
    renderWithProgram(lightRenderContext, renderStages.shadowmap, viewport);
  }
}

struct IdAndUv {
  objid result;
  glm::vec2 resultUv;
};
struct SelectionResult {
  std::optional<objid> id;
  std::optional<glm::vec3> color;
  std::vector<std::optional<IdAndUv>> uvResults;
};


SelectionResult readSelectionFromBuffer(bool readSelectionShader, glm::vec2 adjustedCoords){
  glBindFramebuffer(GL_FRAMEBUFFER, renderingResources.framebuffers.fbo);
  GLenum bufs[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
  glDrawBuffers(4, bufs);

  SelectionResult selectionResult{};
      
  std::vector<std::optional<IdAndUv>> uvResults;
  Color hoveredItemColor = readSelectionShader ? getPixelColorAttachment0(adjustedCoords.x, adjustedCoords.y) : getPixelColorAttachment2(adjustedCoords.x, adjustedCoords.y);
  objid hoveredId = getIdFromColor(hoveredItemColor);

  if (!readSelectionShader){
    std::cout << "selection result raw id: " << hoveredId << ", exists = " << idExists(world.sandbox, hoveredId) <<  std::endl; 
  }
  if (hoveredId != 0 && hoveredId != -16777216){
    selectionResult.id = hoveredId;
    selectionResult.color = glm::vec3(hoveredItemColor.r, hoveredItemColor.g, hoveredItemColor.b);
  }

  for (auto &idCoordToGet : idCoordsToGet){
    if (idCoordToGet.textureId.has_value()){
      uvResults.push_back(std::nullopt);
      continue;
    }

    std::optional<objid> result;
    glm::vec2 resultUv(0.f, 0.f);

    auto pixelCoord = ndiToPixelCoord(glm::vec2(idCoordToGet.ndix, idCoordToGet.ndiy), state.resolution);
    auto id = getIdFromColor(readSelectionShader ? getPixelColorAttachment0(pixelCoord.x, pixelCoord.y) : getPixelColorAttachment2(pixelCoord.x, pixelCoord.y));
    if (id == -16777216){  // this is kind of shitty, this is black so represents no object.  However, theoretically could be an id, should make this invalid id
    }else if (idCoordToGet.onlyGameObjId && !idExists(world.sandbox, id)){
      //modassert(false, std::string("id does not exist: ") + std::to_string(id));
    }else{
      result = id;
    }

    auto uvCoordWithTex = getUVCoordAndTextureId(pixelCoord.x, pixelCoord.y); //     auto uvCoordWithTex = getUVCoordAndTextureId(adjustedCoords.x, adjustedCoords.y);
    auto uvCoord = toUvCoord(uvCoordWithTex);
    resultUv = glm::vec2(uvCoord.x, uvCoord.y);

    if (!result.has_value()){
      uvResults.push_back(std::nullopt);
    }else{
      uvResults.push_back(IdAndUv {
        .result = result.value(),
        .resultUv = resultUv,
      });            
    }
  }

  selectionResult.uvResults = uvResults;
  return selectionResult;
}


std::optional<Texture> getTestCubemap(){
  std::cout << "loading cubemap sky: " << state.skybox << std::endl;
  if (world.textures.find(state.skybox) == world.textures.end()){
    return std::nullopt;
  }
  return world.textures.at(state.skybox).texture;
}

bool textureLoaded(std::string& texturepath){
  return textureLoaded(world, texturepath);
}

void setVoxelLighting2(int voxelCellWidth, glm::vec3 worldOffset){
  setVoxelLighting(voxelCellWidth, worldOffset);
  recalculateLighting(world);
}


void onFramebufferSizeChange(GLFWwindow* window, int width, int height){
  std::cout << "EVENT: framebuffer resized:  new size-  " << "width("<< width << ")" << " height(" << height << ")" << std::endl;
  state.currentScreenWidth = width;
  state.currentScreenHeight = height;
  if (state.nativeResolution){
    state.resolution = glm::ivec2(width, height);
  }
  updateFramebufferWindowSizeChange(renderingResources.framebuffers, state.resolution.x, state.resolution.y);
}; 


std::vector<glm::vec3> debugPoints;
std::vector<std::optional<glm::vec3>> debugPointsTo;
std::vector<glm::vec3> debugPointsColors;
void setDebugPoints(std::vector<glm::vec3> points, std::vector<std::optional<glm::vec3>> pointsTo, std::vector<glm::vec3> colors){
  debugPoints = points;
  debugPointsTo = pointsTo,
  debugPointsColors = colors;
  modassert(debugPoints.size() == debugPointsColors.size(), "setDebugPoints differing sizes");
}

void onGLFWEerror(int error, const char* description){
  std::cerr << "Error: " << description << std::endl;
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

  std::vector<char*> newArgv;
  for (int i = 0; i < argc; i++){
    newArgv.push_back(argv[i]);
  }

  cxxopts::Options cxxoption("ModEngine", "ModEngine is a game engine for hardcore fps");
  cxxoption.add_options()
   ("s,shader", "Folder path of default shader", cxxopts::value<std::string>()->default_value("./res/shaders/default"))
   ("t,texture", "Additional textures folder to use", cxxopts::value<std::string>()->default_value("./res/textures/fonts/gamefont"))
   ("x,scriptpath", "Script file to use", cxxopts::value<std::vector<std::string>>()->default_value(""))
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
   ("data", "Directory to store temporary data", cxxopts::value<std::string>()->default_value("./build/res/data/"))
   ("validate", "Validate resource files instead of running the game", cxxopts::value<std::vector<std::string>>() -> default_value(""))
   ("strict", "Assert the existance of resources during runtime", cxxopts::value<bool>()->default_value("true"))
   ("shell", "Packaging shell",  cxxopts::value<bool>()->default_value("false"))
   ("mount", "Mod package to mount instead of using the file system", cxxopts::value<std::string>()->default_value(""))
   ("package", "Package files to a .mod file instead of running the game", cxxopts::value<std::string>()->default_value(""))
   ("p,pak", "Package files to include", cxxopts::value<std::vector<std::string>>() -> default_value(""))
   ("compile", "Path of map file to compile", cxxopts::value<std::string>() -> default_value(""))
   ("h,help", "Print help")
  ;        

  const auto initialResult = cxxoption.parse(argc, argv);
  auto mount = initialResult["mount"].as<std::string>();
  if (mount == ""){
    if (realfiles::fileExists("./game.mod")){
      mount = "./game.mod";
    }
  }
  if (mount != ""){
    mountPackage(mount.c_str());
  }

  auto startOptions = split(readFileOrPackage("./res/start.options"), '\n');
  for (auto& option : startOptions){
    newArgv.push_back((char*)(option.c_str()));
  }
  int newArgc = newArgv.size();
  auto dataPtr = newArgv.data();
  const auto result = cxxoption.parse(newArgc, dataPtr);

  auto levels = result["log"].as<std::vector<std::string>>();
  modlogSetEnabled(levels.size() > 0, static_cast<MODLOG_LEVEL>(result["loglevel"].as<int>()), levels);

  dataPath = result["data"].as<std::string>();

  auto pathsToValidate = result["validate"].as<std::vector<std::string>>();
  if (pathsToValidate.size() > 0){
    // I could plugin to the actual data types here and do true validation, but this is pragmatic 
    // to find missing files 
    bool missingFiles = false;
    std::vector<std::string> allFiles;
    for (auto &path : pathsToValidate){
      auto files = listFilesWithExtensionsFromPackage(path, { "rawscene" });
      for (auto &file : files){
        allFiles.push_back(file);
      }
    }

    for (auto &file : allFiles){
      std::cout << "validating file: " << file << std::endl;
      auto tokens = parseFormat(readFileOrPackage(file));
      for (auto &token : tokens){
        if (token.attribute == "texture"){
          std::cout << "texture: " << token.payload << std::endl;

          if (!fileExistsFromPackage(token.payload)){
            missingFiles = true;
            std::cout << "\033[31mError:  " << token.payload << ", file = " << file << "\033[0m" << std::endl;
          }
        }
        if (token.attribute == "mesh"){
          std::cout << "mesh: " << token.payload << std::endl;
          if (!fileExistsFromPackage(token.payload)){
            missingFiles = true;
            std::cout << "\033[31mError:  " << token.payload << ", file = " << file << "\033[0m" << std::endl;
          }
        }
      }
    }

    for (auto file : resources::coreResources){
      std::cout << "validating file: " << file << std::endl;
      if (!fileExistsFromPackage(file)){
        missingFiles = true;
        std::cout << "\033[31mError:  " << " hardcoded resource core reference missing - file = " << file << "\033[0m" << std::endl;
      }
    }    

    #ifdef ADDITIONAL_SRC_HEADER
      for (auto file : getAdditionalPathsToValidate()){
        std::cout << "validating file: " << file << std::endl;
        if (!fileExistsFromPackage(file)){
          missingFiles = true;
          std::cout << "\033[31mError:  " << " hardcoded resource additional reference missing - file = " << file << "\033[0m" << std::endl;
        }
      }      
    #endif

    exit(missingFiles ? 1 : 0);
  }

  auto compileMapFile = result["compile"].as<std::string>();
  if (compileMapFile != ""){
    /*auto mapData = parseMapData("../afterworld/scenes/levels/trench/balls/testexport.map");
    auto playerStarts = getEntitiesByClassName(mapData, "player_start");
    for (auto& playerStart : playerStarts){
      auto origin = getValue(*playerStart, "origin");
      modassert(origin.has_value(), "player origin does not have a value");
      std::cout << "player start: " << *(origin.value()) << std::endl;
    }*/

    std::string filepath =  "./build/temp.map.rawscene";

    std::cout << "starting to compile: " << compileMapFile << std::endl;

    compileRawScene(filepath, "../afterworld/scenes/levels/ball.rawscene", compileMapFile, [](Entity& entity, bool* shouldWrite, std::vector<GameobjAttributeOpts>& attributes, std::string* modelName) -> void {
      auto origin = getValue(entity, "origin");
      auto className = getValue(entity, "classname");

      modassert(className.has_value(), std::string("no className index = ") + std::to_string(entity.index));
      std::cout << "compile index: " << entity.index << std::endl;
      std::cout << "origin: " << (origin.has_value() ? *origin.value() : "no origin") << std::endl;
  

      if (*className.value() == "player_start"){
        *modelName = "playerspawn";
        *shouldWrite = true;
      }else if (*className.value() == "powerup_jump"){
        *shouldWrite = false;
      }else if (*className.value() == "vertical_bound_plane"){
        *shouldWrite = true;

        double yValueSum = 0;
        int totalPoints = 0;

        for (auto& brush : entity.brushes){
          for (auto &brushFace : brush.brushFaces){
            yValueSum += brushFace.point1.y;
            yValueSum += brushFace.point2.y;
            yValueSum += brushFace.point3.y;
            totalPoints += 3;
          }
        }
        
        modassert(totalPoints > 0, "invalid ballplane no faces");
        auto average = yValueSum / totalPoints;
        modassert(average > -10000 && average < 10000, "invalid ballplane"); // arbitrary numbers to guard against weird

        attributes.push_back(GameobjAttributeOpts {
          .field = "ballplane",
          .attributeValue = "true",
        });
        attributes.push_back(GameobjAttributeOpts {
          .field = "position", 
          .attributeValue = glm::vec3(0.f, average, 0.f),
        });
      }else if (*className.value() == "worldspawn"){
          *shouldWrite = true;
          attributes.push_back(GameobjAttributeOpts {   // probably not great to attach it to this
            .field = "mesh",
            .attributeValue = "./build/temp.brush",
          });

          attributes.push_back(GameobjAttributeOpts {
            .field = "scale",
            .attributeValue = glm::vec3(0.2f, 0.2f, 0.2f),
          });

          attributes.push_back(GameobjAttributeOpts {
            .field = "position",
            .attributeValue = glm::vec3(68.32f, -22.1f, 166.6f),
          });

          attributes.push_back(GameobjAttributeOpts {
            .field = "physics_shape",
            .attributeValue = "shape_exact",
            .submodel = "entity0_brush9",
          });
          attributes.push_back(GameobjAttributeOpts {
            .field = "physics",
            .attributeValue = "enabled",
            .submodel = "entity0_brush9",
          });

      }else{
        std::cout << "compile map unrecognized type: " << *className.value() << std::endl;
        *shouldWrite = false;
      }
      /*
        playerspawn:teleport:true
 
      //for (auto& [key, value] : entity.keyValues){
      //  origin
      //  std::optional<std::string*> getValue(Entity& entity, const char* key);
      //}*/
    });

    std::cout << "compiled: " << compileMapFile << std::endl;
    return 0;
  }

  strictResourceMode = result["strict"].as<bool>();

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
  bool shouldReloadShaders = result["reload"].as<bool>();

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


  if (result["shell"].as<bool>()){
    loopPackageShell();
  }

  auto package = result["package"].as<std::string>();
  if (package != ""){
    auto filesToInclude = result["pak"].as<std::vector<std::string>>();
    std::cout << "creating package: " << package << std::endl;
    for (auto &file : filesToInclude){
      std::cout << "include: " << file << std::endl;
    }
    packageDirectory(package.c_str(), filesToInclude);
    exit(0);
  }

  auto filewatch = watchFiles(result["watch"].as<std::string>(), std::nullopt /* this is debounce, broken */);

  interface = SysInterface {
    .loadCScript = [](std::string script, objid id, objid sceneId) -> void {
      loadCScript(id, script.c_str(), sceneId, bootStrapperMode, false);
    },
    .unloadCScript = [](std::string scriptpath, objid id) -> void {
      unloadCScript(id);
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
    .saveFile = [](std::string filepath, std::string& data) -> void {
      auto modpath = modlayerPath(filepath);
      realfiles::saveFile(modpath, data);
    },
  };

  api = RenderObjApi  {
    .drawLine = interface.drawLine,
    .drawSphere = [](glm::vec3 pos) -> int {
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
    .isBone = [](objid id) -> bool {
      return getGameObject(world.sandbox, id).isBone;
    },
    .getParentId = [](objid id) -> std::optional<objid> {
      return getGameObjectH(world.sandbox, id).parentId;
    },
    .getTransform = [](objid id) -> Transformation {
      return fullTransformation(world.sandbox, id);
    },
  };

  auto mods = result["mods"].as<std::vector<std::string>>();
  for (auto mod : mods){
    installMod(mod);
  }

  auto layers = parseLayerInfo(result["layers"].as<std::string>(), interface.readFile);

  for (auto &layer : layers){
    if (layer.name == "transparency"){
      transparencyLayer = layer.symbol;
    }

    std::cout << "parse layer: " << layer.name << ", lighting = " << layer.lighting << std::endl;
  }
  modassert(transparencyLayer != -1, "could not find transparency layer");

  auto rawScenes = result["rawscene"].as<std::vector<std::string>>();

  keyMapper = readMapping(result["mapping"].as<std::string>(), inputFns, interface.readFile);

  if (result["help"].as<bool>()){
    std::cout << cxxoption.help() << std::endl;
    return 0;
  }
  bootStrapperMode = result["bootstrapper"].as<bool>();

  shaderFolderPath = result["shader"].as<std::string>();
  auto textureFolderPath = result["texture"].as<std::string>();

  auto timetoexit = result["timetoexit"].as<int>();

  std::cout << "LIFECYCLE: program starting" << std::endl;

  state.fullscreen = result["fullscreen"].as<bool>(); // merge flags and world.state concept

  // have this before createing the state since depends on debuggerDrawer
  BulletDebugDrawer drawer(addLineNextCyclePhysicsDebug);
  debuggerDrawer = &drawer;
  debuggerDrawer -> setDebugMode(0);

  setInitialState(state, "./res/world.state", statistics.now, interface.readFile, result["noinput"].as<bool>()); 

  createViewport(0, state.viewportoffsetNdi.x, state.viewportoffsetNdi.y, state.viewportSizeNdi.x, state.viewportSizeNdi.y, DefaultBindingOption{}, {});

  //createViewport(0, 0.f, 0.f, 1.f, 0.5f, 5);
  //createViewport(1, 0.f, 0.5f, 0.5f, 0.5f, 0);
  //createViewport(2, 0.5f, 0.5f, 0.5f, 0.5f, 0);

  auto glfwInitReturn = glfwInit();
  if (glfwInitReturn != GLFW_TRUE){
    modlog("glfw", "glfw did not initialize successfully");
    return 1;
  }
  
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

  int numCellsDim = getLightingNumCellsDim();
  renderingResources.voxelLighting = generateShaderStorageBuffer(sizeof(int) * getLightingNumCellsTotal());
  {
    int* zeroBuffer = static_cast<int*>(malloc(sizeof(int) * getLightingNumCellsTotal()));
    for (int i = 0; i < numCellsDim; i++){
      zeroBuffer[i] = -1;
    }
   // std::cout << "voxel lighting light : ";
    for (int i = 0; i < numCellsDim; i++){
      updateBufferData(renderingResources.voxelLighting, sizeof(int) * i, sizeof(int), &zeroBuffer[i]);
    //  std::cout << zeroBuffer[i] << " ";
    }
    free(zeroBuffer);
    //std::cout << std::endl;
  }


  if (!glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE){
    std::cerr << "ERROR: framebuffer incomplete" << std::endl;
    return -1;
  }

  onFramebufferSizeChange(window, state.currentScreenWidth, state.currentScreenHeight);
  glfwSetFramebufferSizeCallback(window, onFramebufferSizeChange); 
  glfwSetWindowSizeCallback(window, windowSizeCallback);
  glfwSetWindowPosCallback(window, windowPositionCallback);

  glPointSize(10.f);

  modlog("shaders", std::string("shader file path is ") + shaderFolderPath);
  unsigned int* shaderProgram = loadShaderIntoCache("default", shaderFolderPath + "/vertex.glsl", shaderFolderPath + "/fragment.glsl", interface.readFile, getTemplateValues());
  initDefaultShader(*shaderProgram);

  std::string framebufferShaderPath = "./res/shaders/framebuffer";
  modlog("shaders", std::string("framebuffer file path is ") + framebufferShaderPath);
  renderingResources.framebufferProgram = loadShaderIntoCache("framebuffer", framebufferShaderPath + "/vertex.glsl", framebufferShaderPath + "/fragment.glsl", interface.readFile, getTemplateValues());
  initFramebufferShader(*renderingResources.framebufferProgram);

  std::string depthShaderPath = "./res/shaders/depth";
  modlog("shaders", std::string("depth file path is ") + depthShaderPath);
  unsigned int* depthProgram = loadShaderIntoCache("depth", depthShaderPath + "/vertex.glsl", depthShaderPath + "/fragment.glsl", interface.readFile, getTemplateValues());
  initDepthShader(*depthProgram);

  std::string uiShaderPath = "./res/shaders/ui";
  modlog("shaders", std::string("ui shader file path is ") + uiShaderPath);
  renderingResources.uiShaderProgram = loadShaderIntoCache("ui", uiShaderPath + "/vertex.glsl",  uiShaderPath + "/fragment.glsl", interface.readFile, getTemplateValues());
  initUiShader(*renderingResources.uiShaderProgram);

  std::string selectionShaderPath = "./res/shaders/selection";
  modlog("shaders", std::string("selection shader path is ") + selectionShaderPath);
  unsigned int* selectionProgram = loadShaderIntoCache("selection", selectionShaderPath + "/vertex.glsl", selectionShaderPath + "/fragment.glsl", interface.readFile, getTemplateValues());
  initSelectionShader(*selectionProgram);

  std::string blurShaderPath = "./res/shaders/blur";
  modlog("shaders", std::string("blur shader path is: ") + blurShaderPath);
  unsigned int* blurProgram = loadShaderIntoCache("blur", blurShaderPath + "/vertex.glsl", blurShaderPath + "/fragment.glsl", interface.readFile, getTemplateValues());
  initBlurShader(*blurProgram);

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
    .createScene = createScene,
    .deleteScene = deleteScene,
    .getCameraTransform = getCameraTransform,
    .moveCameraTo = moveCameraTo,
    .idsInGroupById = idsInGroupById,
    .groupId = groupId,
    .removeObjectById = removeObjectById,
    .removeByGroupId = removeByGroupId,
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
      auto shader = loadShaderIntoCache(name, path + "/vertex.glsl", path + "/fragment.glsl", interface.readFile, getTemplateValues());

      auto isUiShader = checkIfUiShader(name);
      extraShadersToUpdate.push_back(ShaderToUpdate {
        .shader = shader,
        .isUiShader = isUiShader,
        .name = name,
      });
      if (isUiShader){
        initUiShader(*shader);
      }else{
        initDefaultShader(*shader);
      }

      return shader;
    },
    .unloadShader = [](unsigned int shader){
      std::vector<ShaderToUpdate> remainingsShaders;
      for (auto &shaderToUpdate : extraShadersToUpdate){
        if (*shaderToUpdate.shader == shader){
          continue;
        }
        remainingsShaders.push_back(shaderToUpdate);
      }
      extraShadersToUpdate = remainingsShaders;
      textureBindings.erase(shader);
      unloadShader(shader);
    },
    .setShaderUniform = setUniformData,
    .getTextureSamplerId = getTextureSamplerId,
    .bindTexture = bindTexture,
    .unbindTexture = unbindTexture,
    .getGameObjNameForId = getGameObjectName,
    .setGameObjectAttr = setGameObjectAttr,
    .setSingleGameObjectAttr = setSingleGameObjectAttr,
    
    .getPhysicsVelocity = getPhysicsVelocity,
    .setPhysicsVelocity = setPhysicsVelocity,
    .setMeshEnabled = setMeshEnabled,

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
    .getModAABBModel = getModAABBModel,
    .getPhysicsInfo = getPhysicsInfo,
    .listAnimations = listAnimations,
    .playAnimation = playAnimation,
    .stopAnimation = stopAnimation,
    .disableAnimationIds = disableAnimationIds,
    .setAnimationPose = setAnimationPose,
    .clearAnimationPose = clearAnimationPose,
    .animationLength = animationLength,
    .listClips = listSounds,
    .playClip = playSoundState,
    .playClipById = playSoundState,
    .stopClip = stopSoundState,
    .stopClipById = stopSoundStateById,
    .listResources = listResources,
    .sendNotifyMessage = sendNotifyMessage,
    .timeSeconds = timeSeconds,
    .timeElapsed = timeElapsed,
    .currentFrame = currentFrame,
    .saveScene = saveScene,
    .connectServer = connectServer,
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
    .navPosition = navPosition,
    .emit = emit,
    .setParticleState = setParticleState,
    .loadAround = addLoadingAround,
    .rmLoadAround = removeLoadingAround,
    .generateMesh = createGeneratedMesh,
    .generateMeshRaw = createGeneratedMeshRaw,
    .getArgs = getArgs,
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
    .mountPackage = mountPackage,
    .compileSqlQuery = sql::compileSqlQuery,
    .executeSqlQuery = executeSqlQuery,
    .selected = []() -> std::vector<objid> {
      auto ids = state.editor.selectedObjs;
      for (auto id : ids){
        if (!idExists(world.sandbox, id)){
          modassert(false, "id does not exist but is in selected objs");
        }
      }
      return ids;
    },
    .setSelected = setSelected,
    .click = dispatchClick,
    .moveMouse = moveMouse,
    .schedule = schedule,
    .getFrameInfo = getFrameInfo,
    .getCursorInfoWorld = getCursorInfoWorld,
    .idAtCoordAsync = idAtCoordAsync,
    .depthAtCoordAsync = depthAtCoordAsync,
    .gameobjExists = gameobjExists,
    .prefabId = prefabId,
    .positionToNdi = positionToNdi,
    .setLogEndpoint = setLogEndpoint,
    .getClipboardString = getClipboardString,
    .setClipboardString = setClipboardString,

    .downloadFile = downloadFile,
    .downloadFileInMemory = downloadFileInMemory,
    .isServerOnline = isServerOnline,
    .getServerConfig = getServerConfig,
    .getProperty = getProperty,

    .saveState = saveState,
    .loadState = loadState,

    .getVoxelLightingData = getVoxelLightingData,
    .setGlobalLight = setGlobalLight,
    .setVoxelLighting = setVoxelLighting2,
    .getTag = getTag,
    .getAllTags = getAllTags,
    .getMaterial = getMaterial,
    .getMainOctreeId = getMainOctreeId,

    .createPhysicsBody = createPhysicsBody,
    .setPhysicsOptions = setPhysicsOptions,
    .createFixedConstraint = createFixedConstraint,
    .createPointConstraint = createPointConstraint,
    .createHingeConstraint = createHingeConstraint,

    .physicsLayer = physicsLayer,
    
    .saveToJsonFile = saveToJsonFile,
    .loadFromJsonFile = loadFromJsonFile,

    .createViewport = createViewport,
    .removeViewport = removeViewport,
    .listViewports = listViewports,

    .getControlInfo = getControlInfo2,

    .dumpDebugInfo = dumpDebugInfo,
  };


  mainApi = &pluginApi;

  std::vector<CScriptBinding> pluginBindings = { 
    cscriptCreatePerformanceGraphBinding(pluginApi),
    cscriptCreatePerfVisualizeBinding(pluginApi),
    cscriptCreateToolsBinding(pluginApi),
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

  netcode = initNetCode(bootStrapperMode, interface.readFile);
  
  modassert(modlayerFileExists(state.iconpath), std::string("icon file does not exist: ") + state.iconpath);
  GLFWimage images[1]; 
  stbi_set_flip_vertically_on_load(false);
  images[0].pixels = stbiloadImage(state.iconpath.c_str(), &images[0].width, &images[0].height, 0, 4); //rgba channels 
  glfwSetWindowIcon(window, 1, images);

  std::vector<std::string> defaultMeshesToLoad {
    resources::MODEL_NODE,
    "./res/models/unit_rect/unit_rect.obj",
    resources::MODEL_CAMERA,
    "./res/models/box/plane.dae",
    resources::MODEL_UNITXY,
    resources::MODEL_EMITTER,
    resources::MODEL_SOUND,
    resources::MODEL_LIGHT
  };

  std::vector<std::string> allTexturesToLoad = {  "./res/textures/crosshairs/crosshair029.png", "./res/textures/crosshairs/crosshair008.png" };

  bool enableNet = false;
  world = createWorld(
    onObjectEnter, 
    onObjectLeave, 
    [enableNet](objid id) -> void {
    }, 
    [enableNet](GameObject& obj) -> void {
      cBindings.onObjectAdded(obj.id);
    },
    [enableNet](objid id, bool isNet) -> void {
      std::cout << "deleted obj id: " << id << std::endl;
      for (auto& viewport : viewports){
        maybeResetCamera(id, viewport);
      }
      unsetSelectedIndex(state.editor, id, true);
      if (getSelectedOctreeId().has_value() && getSelectedOctreeId().value() == id){
        setSelectedOctreeId(std::nullopt);
      }
      removeScheduledTaskByOwner({ id });

      cBindings.onObjectRemoved(id);
      timings.disableAnimationIds.erase(id);
      freeTexture(id);
    }, 
    debuggerDrawer, 
    layers,
    interface,
    defaultMeshesToLoad,
    allTexturesToLoad
  );
  if (textureFolderPath != ""){
    loadAllTextures(textureFolderPath);
  }
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
      .nodeMesh = &world.meshes.at(resources::MODEL_NODE).mesh,
      .portalMesh = &world.meshes.at("./res/models/box/plane.dae").mesh,
      .cameraMesh = &world.meshes.at(resources::MODEL_CAMERA).mesh, 
      .voxelCubeMesh = &world.meshes.at("./res/models/unit_rect/unit_rect.obj").mesh,
      .unitXYRect = &world.meshes.at(resources::MODEL_UNITXY).mesh,
      .soundMesh = &world.meshes.at(resources::MODEL_SOUND).mesh,
      .lightMesh = &world.meshes.at(resources::MODEL_LIGHT).mesh,
      .emitter = &world.meshes.at(resources::MODEL_EMITTER).mesh,
      .nav = &world.meshes.at(resources::MODEL_NODE).mesh,
    }
  };

  dynamicLoading = createDynamicLoading(worldfile, interface.readFile);
  if (result["rechunk"].as<int>()){
    rechunkAllObjects(world, dynamicLoading, result["rechunk"].as<int>());
    return 0;
  }

  loadCScript(getUniqueObjId(), "native/tools", -1, bootStrapperMode, true);
  for (auto script : result["scriptpath"].as<std::vector<std::string>>()){
    loadCScript(getUniqueObjId(), script.c_str(), -1, bootStrapperMode, true);
  }
  afterFrameForScripts(); // call this so api called will be ready -- for the above script (eg for callbacks).  This moves scripts from staged to loaded

  
  std::cout << "INFO: # of intitial raw scenes: " << rawScenes.size() << std::endl;
  for (auto parsedScene : parseSceneArgs(rawScenes)){
    loadScene(parsedScene.sceneToLoad, {}, parsedScene.sceneFileName, parsedScene.tags);
  }

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
  GLenum buffers_to_render[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
  glDrawBuffers(4, buffers_to_render);

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

  waterShader = pluginApi.loadShader("water", "./res/shaders/water");

  PROFILE("MAINLOOP",
  while (!glfwWindowShouldClose(window)){
  PROFILE("FRAME",
    std::cout << inColor("hint - START FRAME", CONSOLE_COLOR_YELLOW) << std::endl;

    auto shouldExit = updateTime(fpsFixed, fixedDelta, state.engineSpeed, timetoexit, hasFramelimit, minDeltaTime, fpsLag);
    if (shouldExit || shouldQuitControl){
      goto cleanup;
    }

    onEffekSeekerFrame();

    resetReservedId();
    disposeTempBufferedData(lineData);

    for (int i = 0; i < debugPoints.size(); i++){
      auto point = debugPoints.at(i);
      auto pointTo = debugPointsTo.at(i);
      auto color = debugPointsColors.at(i);

      if (pointTo.has_value()){
        addLineNextCycle(point, pointTo.value(), false, -1, glm::vec4(color.r, color.g, color.b, 1.f), std::nullopt, std::nullopt);
      }else{
        addLineNextCycle(point, point + glm::vec3(0.f, 5.f, 0.f), false, -1, glm::vec4(color.r, color.g, color.b, 1.f), std::nullopt, std::nullopt);
        addLineNextCycle(point, point + glm::vec3(5.f, 0.f, 0.f), false, -1, glm::vec4(color.r, color.g, color.b, 1.f), std::nullopt, std::nullopt);
        addLineNextCycle(point, point + glm::vec3(0.f, 0.f, -5.f), false, -1, glm::vec4(color.r, color.g, color.b, 1.f), std::nullopt, std::nullopt);        
      }
    }

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

    for (auto &depthCoord : depthsAtCoords){
      depthCoord.afterFrame(depthCoord.resultDepth.value());
    }
    depthsAtCoords = {};

    if (!state.worldpaused){
      timePlayback.setTime(statistics.now);  // tick animations here
      tickAnimations(world, timings, timePlayback.getCurrentTime());
    }

    if (state.updateSkybox){
      loadSkybox(world, state.skybox); 
      state.updateSkybox = false;
    }
    tickRecordings(getTotalTimeGame());
    tickScheduledTasks();
    handleMovingObjects(timePlayback.getCurrentTime());

    glfwPollEvents();
    handleInput(window);
    handleMouseEvents();

    cBindings.onFrame();

    onNetCode(netcode, onClientMessage, bootStrapperMode);

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


    onWorldFrame(world, timePlayback.getDeltaTime(), timePlayback.getCurrentTime(), state.enablePhysics, state.worldpaused, viewTransform, state.inputMode == ENABLED, false);
    cBindings.onFrameAfterUpdate();
    onWorldFrame(world, timePlayback.getDeltaTime(), timePlayback.getCurrentTime(), false, state.worldpaused, viewTransform, state.inputMode == ENABLED, true);

    handleChangedResourceFiles(pollChangedFiles(filewatch, glfwGetTime()));
    if (useChunkingSystem){
      handleChunkLoading(
        dynamicLoading, 
        [](objid id) -> glm::vec3 { 
          return getGameObjectPosition(id, true, "chunk loading - get load around position");
        }, 
        [](std::string sceneFile, glm::vec3 offset, std::string parentNodeName) -> objid {
          modassert(false, "not implemented");
          return 0;
        }, 
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

    auto adjustedCoords = pixelCoordsRelativeToViewport(state.cursorLeft, state.cursorTop, state.currentScreenHeight, calcViewportSize(getDefaultViewport()), calcViewportOffset(getDefaultViewport()), state.resolution);
    state.adjustedCoords = adjustedCoords;

    bool selectItemCalledThisFrame = selectItemCalled;
    selectItemCalled = false;  // reset the state
    auto selectTargetId = state.forceSelectIndex == 0 ? state.currentHoverIndex : state.forceSelectIndex;
    auto shouldSelectItem = selectItemCalledThisFrame || (state.forceSelectIndex != 0);
    state.forceSelectIndex = 0; // stateupdate


    if (shouldSelectItem){
      auto objExists = idExists(world.sandbox, selectTargetId); 
      if (objExists){
        auto layerSelectIndex = getLayerForId(selectTargetId).selectIndex;
        if (!(layerSelectIndex == -1) && !state.selectionDisabled){
          modlog("selection", (std::string("select item called") + ", selectedId = " + std::to_string(selectTargetId) + ", layerSelectIndex = " + std::to_string(layerSelectIndex)).c_str());
          auto groupId = getGroupId(world.sandbox, selectTargetId);
          auto idToUse = state.groupSelection ? groupId : selectTargetId;

          auto isPrefab = prefabId(world, selectTargetId);
          std::cout << "prefab: is a prefab: ? " << (isPrefab.has_value() ? "true" : "false") << std::endl;
          if (isPrefab.has_value()){
            std::cout << "prefab: is a prefab: ? " << std::to_string(isPrefab.has_value()) << std::endl;
            idToUse = isPrefab.value();
          }

          if (layerSelectIndex >= 0 && state.inputMode == ENABLED){
            modassert(idExists(world.sandbox, idToUse), "id does not exist shouldSelectItem");
            setSelectedIndex(state.editor, idToUse, !state.multiselect);
          }
        }
        if((state.cursorBehavior != CURSOR_HIDDEN || state.showCursor) && state.inputMode == ENABLED){
          cBindings.onObjectSelected(selectTargetId, state.hoveredColor.value(), layerSelectIndex);        
        }
      }else if (isReservedObjId(selectTargetId)){
        onObjectSelected(selectTargetId);
      }else{
        std::cout << "INFO: select item called -> id not in scene! - " << selectTargetId<< std::endl;
        onObjectUnselected();
        cBindings.onObjectUnselected();
      }
    }

    if (state.lastHoverIndex != state.currentHoverIndex){  
      if (idExists(world.sandbox, state.lastHoverIndex)){
        cBindings.onObjectHover(state.lastHoverIndex, false);
      }
      if (idExists(world.sandbox, state.currentHoverIndex)){
        cBindings.onObjectHover(state.currentHoverIndex, true);
      }
    }
    
    if (state.shouldToggleCursor){
      modlog("toggle cursor", std::to_string(state.cursorBehavior));
      toggleCursor(state.cursorBehavior);
      state.shouldToggleCursor = false;
    }  

    afterFrameForScripts();

    while (!channelMessages.empty()){
      auto message = channelMessages.front();
      channelMessages.pop();
      cBindings.onMessage(message.strTopic, message.strValue);
    }



    //////////////////////// rendering code below ///////////////////////


    std::cout << "updateDefaultShaderPerFrame: ui" << std::endl;
    updateUiShaderPerFrame(*renderingResources.uiShaderProgram);

    modlog("shader to update size", std::to_string(extraShadersToUpdate.size()));
    for (auto shader : extraShadersToUpdate){
      if (shader.isUiShader){
        updateUiShaderPerFrame(*shader.shader);
      }
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

    //////////////////////// below is now viewport dependent

    //auto& defaultViewport = getDefaultViewport();

    for (auto& viewport : viewports){
      std::vector<LightInfo> lights = getLightInfo(world);
      viewTransform = getCameraTransform(viewport.index);
      cullingViewTransform = getCullingTransform(viewport.index);
      view = renderView(viewTransform.position, viewTransform.rotation);
      std::vector<PortalInfo> portals = getPortalInfo(world);
      assert(portals.size() <= renderingResources.framebuffers.portalTextures.size());
      std::vector<glm::mat4> lightMatrixs = calcShadowMapViews(lights);

      std::cout << "updateDefaultShaderPerFrame: main" << std::endl;
      updateDefaultShaderPerFrame(*mainShaders.shaderProgram, lights, false, viewTransform.position, lightMatrixs);
    

      for (auto shader : extraShadersToUpdate){
        if (!shader.isUiShader){
          std::cout << "updateDefaultShaderPerFrame2 extraShadersToUpdate: " << shader.name << std::endl;
          updateDefaultShaderPerFrame(*shader.shader, lights, false, viewTransform.position, lightMatrixs);
        }
      }


      std::cout << "updateDefaultShaderPerFrame: selection" << std::endl;
      updateSelectionShaderPerFrame(*mainShaders.selectionProgram, lights, viewTransform.position, lightMatrixs);


      RenderContext renderContext {
        .view = view,
        .portals = portals,
        .projection = std::nullopt,
      };

      bool depthEnabled = false;

      auto dofInfo = getDofInfo(world, &depthEnabled, viewport.activeCameraData, view);
      updateRenderStages(renderStages, dofInfo);
      glViewport(0, 0, state.currentScreenWidth, state.currentScreenHeight);

      getVoxelLightingData().needsUpdate = {};

      SelectionResult uiSelectionResult{};
      PROFILE("SELECTION",
      // outputs to FBO unique colors based upon ids. This eventually passed in encodedid to all the shaders which is how color is determined
        renderWithProgram(renderContext, renderStages.selection, viewport);
  
        shaderSetUniform(*renderStages.selection.shader, "projview", ndiOrtho);
        glDisable(GL_DEPTH_TEST);
        drawShapeData(lineData, *renderStages.selection.shader, fontFamilyByName, std::nullopt,  state.currentScreenHeight, state.currentScreenWidth, *defaultResources.defaultMeshes.unitXYRect, getTextureId, true);
        glEnable(GL_DEPTH_TEST);
  
        uiSelectionResult = readSelectionFromBuffer(true, adjustedCoords);
      )
    
      // depth buffer from point of view SMf 1 light source (all eventually, but 1 for now)
      if (state.enableShadows){
        PROFILE(
          "RENDERING-SHADOWMAPS",
          renderShadowMaps(renderContext, lights, viewport);
        )
      }

      assert(portals.size() <= renderingResources.framebuffers.portalTextures.size());
      PROFILE("PORTAL_RENDERING", 
        portalIdCache = renderPortals(renderContext, viewTransform, viewport);
      )
      //std::cout << "cache size: " << portalIdCache.size() << std::endl;

      Color pixelColor;
      PROFILE("MAIN",
        statistics.numTriangles = renderWithProgram(renderContext, renderStages.main, viewport);
    

        shaderLogDebug("read-selection");



        auto selectionResult = readSelectionFromBuffer(false, adjustedCoords);
        if (uiSelectionResult.id.has_value()){
          selectionResult.id = uiSelectionResult.id;
          selectionResult.color = uiSelectionResult.color;
          std::cout << "selection result: " << uiSelectionResult.id.value() << std::endl;
        }else{
          std::cout << "selection result: no value" << std::endl;
        }
        for (int i = 0; i < selectionResult.uvResults.size(); i++){
          if (uiSelectionResult.uvResults.at(i).has_value()){
            selectionResult.uvResults.at(i) = uiSelectionResult.uvResults.at(i);
          }
        }

        if (selectionResult.id.has_value()){
          state.lastHoverIndex = state.currentHoverIndex; // stateupdate
          state.currentHoverIndex = selectionResult.id.value(); // 
          auto hoveredItemColor = selectionResult.color.value();
          state.hoveredItemColor = glm::vec3(hoveredItemColor.r, hoveredItemColor.g, hoveredItemColor.b); // stateupdate     
        }
        for (int i = 0; i < selectionResult.uvResults.size(); i++){
          if (!selectionResult.uvResults.at(i).has_value()){
            continue;
          }
          idCoordsToGet.at(i).result = selectionResult.uvResults.at(i).value().result;
          idCoordsToGet.at(i).resultUv = selectionResult.uvResults.at(i).value().resultUv;
        }

        {
            //GLenum bufs[] = {GL_COLOR_ATTACHMENT0};
            //glDrawBuffers(1, bufs);

            auto viewportDim = calcViewportSize(viewport);
            auto& layer = world.sandbox.layers.at(0);
            auto fov = getLayerFov(layer);
            PROFILE("EFFEK",
              static bool createdDummy = false;
              static GLuint dummyVAO = 0;
              if (!createdDummy){
                glGenVertexArrays(1, &dummyVAO);
                glBindVertexArray(dummyVAO);
              createdDummy = true;
            }
            glBindVertexArray(dummyVAO);

            glDrawBuffers(1, buffers_to_render);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, 0, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, 0, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, 0, 0);

            onEffekSeekerRender(static_cast<float>(viewportDim.x), static_cast<float>(viewportDim.y), glm::radians(fov), viewTransform.position, viewTransform.rotation,  layer.nearplane, layer.farplane);
            
           )   
        }

        PROFILE("RENDER-VECTOR",
          renderVector(view, numChunkingGridCells, viewport);
        )

        shaderLogDebug("read-pixel-color");
        pixelColor = getPixelColorAttachment0(adjustedCoords.x, adjustedCoords.y);
        state.hoveredColor = glm::vec3(pixelColor.r, pixelColor.g, pixelColor.b);
      )

      if (state.enableBloom){
        PROFILE("BLOOM-RENDERING",
          renderWithProgram(renderContext, renderStages.bloom1, viewport);
          renderWithProgram(renderContext, renderStages.bloom2, viewport);
        )      
      }

      if (depthEnabled){
        PROFILE("DOF-RENDERING",
          renderWithProgram(renderContext, renderStages.dof1, viewport);
          renderWithProgram(renderContext, renderStages.dof2, viewport);
        )
      }

      LayerInfo& layerInfo = layerByName(world, "");
      float near = layerInfo.nearplane;
      float far = layerInfo.farplane;
    

      // depth buffer pass 
      // why does this require drawing this?  Couldn't I just read it from the attachment directly? Does it matter?
      {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(*depthProgram); 
        //glClearColor(0.f, 0.0f, 0.0f, 1.0f);
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
        updateDepthShaderPerFrame(*depthProgram, near, far);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.depthTextures.at(0));
        glViewport(calcViewportOffset(viewport).x, calcViewportOffset(viewport).y, calcViewportSize(viewport).x, calcViewportSize(viewport).y);
        glBindVertexArray(defaultResources.quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // The depth here doesn't seem to have really tight resolution. 
        Color hoveredItemColor = getPixelColorAttachment0(adjustedCoords.x, adjustedCoords.y);
        float distance = (hoveredItemColor.r * (far - near)) + near;
        std::cout << "depth: " << distance << ", near = " << near << ", far = " << far << std::endl;
        state.currentCursorDepth = distance;
        for (auto &depthCoord : depthsAtCoords){
          Color hoveredItemColor = getPixelColorAttachment0(depthCoord.ndix, depthCoord.ndiy);
          float distance = (hoveredItemColor.r * (far - near)) + near;
          depthCoord.resultDepth = distance;
        }
      }
      /////////////////////////////////////

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      //auto finalProgram = (state.renderMode == RENDER_DEPTH) ? *depthProgram : *renderingResources.framebufferProgram;
      auto finalProgram = *renderingResources.framebufferProgram;

      glUseProgram(finalProgram); 



      state.exposure = exposureAmount();
      updateFramebufferShaderFrame(finalProgram, near, far);

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.depthTextures.at(0));

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.framebufferTexture2);

      glActiveTexture(GL_TEXTURE0);


      modlog("viewports num: ", std::to_string(viewports.size()));
      
      {
        std::cout << "viewport: " << print(viewport.activeCameraObj) << ", name = " << (!viewport.activeCameraObj.has_value() ? std::string("no value") : getGameObjectName(viewport.activeCameraObj.value()).value()) <<  std::endl;
        glClear(GL_DEPTH_BUFFER_BIT);  // just disable depth test here? 

        bool shouldRender = true;
      
        static ViewportOption defaultOption = DefaultBindingOption{};
        ViewportOption* bindingOption = viewport.bindingOption.has_value() ? &viewport.bindingOption.value() : &defaultOption;

        DefaultBindingOption* defaultBindingOption = std::get_if<DefaultBindingOption>(bindingOption);
        BloomBindingOption* bloomBindingOption = std::get_if<BloomBindingOption>(bindingOption);
        PortalBindingOption* portalBindingOption = std::get_if<PortalBindingOption>(bindingOption);
        TextureBindingOption* textureBindingOption = std::get_if<TextureBindingOption>(bindingOption);
        UserTextureBindingOption* userTextureBindingOption = std::get_if<UserTextureBindingOption>(bindingOption);
        Unknown1BindingOption* unknown1BindingOption = std::get_if<Unknown1BindingOption>(bindingOption);
        Unknown2BindingOption* unknown2BindingOption = std::get_if<Unknown2BindingOption>(bindingOption);
        DepthBindingOption* depthBindingOption = std::get_if<DepthBindingOption>(bindingOption);

        shaderSetUniformBool(finalProgram, "flipCoords", shouldRender && textureBindingOption != NULL && textureBindingOption -> flipCoords ? true : false);
        shaderSetUniformBool(finalProgram, "enableDepthVisualization", shouldRender && (depthBindingOption != NULL) ? true : false);
        if (shouldRender) {
          if (defaultBindingOption){
            glBindTexture(GL_TEXTURE_2D, finalRenderingTexture(renderStages));
          }else if (portalBindingOption){ // portal
            assert(state.textureIndex <= renderingResources.framebuffers.portalTextures.size() && state.textureIndex >= 0);
            glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.portalTextures.at(state.textureIndex));  
          }else if (unknown1BindingOption){
            glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.framebufferTexture4);  
          }else if (textureBindingOption){ // texture
            shaderSetUniformBool(finalProgram, "flipCoords", textureBindingOption -> flipCoords);
            if (world.textures.find(textureBindingOption -> texture) != world.textures.end()){
              glBindTexture(GL_TEXTURE_2D, world.textures.at(textureBindingOption -> texture).texture.textureId);
            }else{
              glBindTexture(GL_TEXTURE_2D, world.textures.at(resources::GRID_TEXTURE).texture.textureId);
            }
          }else if (depthBindingOption){ // depth 
            assert(state.textureIndex <=  renderingResources.framebuffers.depthTextures.size() && state.textureIndex >= 0);
            glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.depthTextures.at(state.textureIndex));
          }else if (bloomBindingOption){ // bloom
            glBindTexture(GL_TEXTURE_2D, renderingResources.framebuffers.framebufferTexture2);
          }else if (userTextureBindingOption){ // user textures
            if (screenspaceTextureIds.size() > state.textureIndex && state.textureIndex >= 0){
              glBindTexture(GL_TEXTURE_2D, screenspaceTextureIds.at(state.textureIndex).id);
            }else{
              modlog("rendering", (std::string("cannot display graph texture index: ") + std::to_string(state.textureIndex)).c_str());
            }
          }else if (unknown2BindingOption){ 
             glBindTexture(GL_TEXTURE_2D, world.textures.at("gentexture-ingame-ui-texture-test").texture.textureId);  
          }else{
            modassert(false, "invalid binding option");
          }
        }else{
          std::cout << "viewport: not yet supported rendering from another camera" << std::endl;
          glBindTexture(GL_TEXTURE_2D, world.textures.at(resources::GRID_TEXTURE).texture.textureId);  
        }

        glViewport(calcViewportOffset(viewport).x, calcViewportOffset(viewport).y, calcViewportSize(viewport).x, calcViewportSize(viewport).y);
        glBindVertexArray(defaultResources.quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

    }


    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, state.currentScreenWidth, state.currentScreenHeight);
  
    {
      renderDebugUi(pixelColor);

      // below and render screepspace lines can probably be consoliated
      glUseProgram(*renderingResources.uiShaderProgram);
      glEnable(GL_BLEND);
      glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glBlendFunci(1, GL_ONE, GL_ZERO);
      
      drawShapeData(lineData, *renderingResources.uiShaderProgram, fontFamilyByName, std::nullopt,  state.currentScreenHeight, state.currentScreenWidth, *defaultResources.defaultMeshes.unitXYRect, getTextureId, false);
    }
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

    glClearColor(0.f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    currentTick++;
    
  )})

  modlog("lifecycle", "program exiting");

  cleanup:   
    if (shouldBenchmark){
      realfiles::saveFile(benchmarkFile, dumpDebugInfo());
    }
    deinitPhysics(world.physicsEnvironment); 
    stopSoundSystem();
    glfwTerminate(); 
    stbi_image_free(images[0].pixels);


  return 0;
}
