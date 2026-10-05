#include "./main_test.h"
#include "../scene/common/util/loadmodel.h"

extern CustomApiBindings* mainApi;

///Unit tests ///////////////
struct TestCase {
  const char* name;
  std::function<void()> test;
};

void sampleTest(){ 
  //throw std::logic_error("error loading buffer");
}

void mergeModelDataTest(){
  MeshData firstMesh;
  Bone firstBone{};
  firstBone.name = "root";
  firstBone.shortName = "root";
  firstMesh.bones.push_back(firstBone);
  ModelData first {
    .meshIdToMeshData = {{ 0, firstMesh }},
    .nodeToMeshId = {{ 0, { 0 } }, { 1, {} }},
    .childToParent = {{ 1, 0 }},
    .nodeTransform = {
      { 0, Transformation{ .position = glm::vec3(1.f, 0.f, 0.f), .scale = glm::vec3(1.f), .rotation = glm::quat(1.f, 0.f, 0.f, 0.f) } },
      { 1, Transformation{ .position = glm::vec3(0.f), .scale = glm::vec3(1.f), .rotation = glm::quat(1.f, 0.f, 0.f, 0.f) } },
    },
    .names = {{ 0, "root" }, { 1, "child" }},
    .bones = { 0 },
  };
  MeshData secondMesh;
  Bone secondBone{};
  secondBone.name = "root";
  secondBone.shortName = "root";
  secondMesh.bones.push_back(secondBone);
  ModelData second {
    .meshIdToMeshData = {{ 0, secondMesh }},
    .nodeToMeshId = {{ 0, { 0 } }},
    .nodeTransform = {
      { 0, Transformation{ .position = glm::vec3(0.f, 2.f, 0.f), .scale = glm::vec3(1.f), .rotation = glm::quat(1.f, 0.f, 0.f, 0.f) } },
    },
    .names = {{ 0, "root" }},
    .bones = { 0 },
  };
  ModelData empty;
  std::vector<ModelData> models = { first, second, empty };

  ModelData merged = mergeModelData(models);
  modassert(merged.nodeTransform.size() == 4 && merged.meshIdToMeshData.size() == 2,
    "merged model should contain the combined root and all source nodes and meshes");
  modassert(merged.names.at(0) == "model" && merged.names.at(1) == "0/root" && merged.names.at(3) == "1/root",
    "merged model node names should be namespaced");
  Transformation& mergedRootTransform = merged.nodeTransform.at(0);
  modassert(mergedRootTransform.position == glm::vec3(0.f) &&
    mergedRootTransform.scale == glm::vec3(1.f) &&
    mergedRootTransform.rotation == glm::quat(1.f, 0.f, 0.f, 0.f),
    "merged model root transform should be identity");
  modassert(merged.childToParent.at(1) == 0 &&
    merged.childToParent.at(2) == 1 &&
    merged.childToParent.at(3) == 0,
    "merged model hierarchy should preserve parents and connect source roots");
  modassert(merged.nodeToMeshId.at(1).at(0) != merged.nodeToMeshId.at(3).at(0),
    "merged model mesh IDs should be unique");
  modassert(merged.meshIdToMeshData.at(0).bones.at(0).name == "0/root" &&
    merged.meshIdToMeshData.at(1).bones.at(0).name == "1/root" &&
    merged.bones.size() == 2,
    "merged model should namespace bone names and remap bone node IDs");
}

void mergeCommonMeshesTest(){
  MeshData firstMesh{};
  firstMesh.vertices.resize(3);
  firstMesh.indices = { 0, 1, 2 };
  firstMesh.boundInfo = { .xMin = -1.f, .xMax = 1.f, .yMin = -2.f, .yMax = 2.f, .zMin = -3.f, .zMax = 3.f };
  MeshData secondMesh{};
  secondMesh.vertices.resize(3);
  secondMesh.indices = { 0, 1, 2 };
  secondMesh.boundInfo = { .xMin = -4.f, .xMax = 4.f, .yMin = -5.f, .yMax = 5.f, .zMin = -6.f, .zMax = 6.f };
  Transformation identity {
    .position = glm::vec3(0.f),
    .scale = glm::vec3(1.f),
    .rotation = glm::quat(1.f, 0.f, 0.f, 0.f),
  };
  Transformation firstTransform = identity;
  firstTransform.position = glm::vec3(2.f, 0.f, 0.f);
  Transformation secondTransform = identity;
  secondTransform.position = glm::vec3(10.f, 0.f, 0.f);
  ModelData first {
    .meshIdToMeshData = {{ 0, firstMesh }},
    .nodeToMeshId = {{ 0, { 0 } }},
    .nodeTransform = {{ 0, firstTransform }},
    .names = {{ 0, "root" }},
  };
  ModelData second {
    .meshIdToMeshData = {{ 0, secondMesh }},
    .nodeToMeshId = {{ 0, { 0 } }},
    .nodeTransform = {{ 0, secondTransform }},
    .names = {{ 0, "root" }},
  };
  std::vector<ModelData> models = { first, second };

  ModelData merged = mergeModelData(models);
  modassert(merged.meshIdToMeshData.size() == 1, "meshes with the same material should be merged");
  int mergedMeshId = merged.nodeToMeshId.at(0).at(0);
  const MeshData& mergedMesh = merged.meshIdToMeshData.at(mergedMeshId);
  modassert(mergedMesh.vertices.size() == 6 && mergedMesh.indices.size() == 6 &&
    mergedMesh.indices.at(0) == 0 && mergedMesh.indices.at(1) == 1 && mergedMesh.indices.at(2) == 2 &&
    mergedMesh.indices.at(3) == 3 && mergedMesh.indices.at(4) == 4 && mergedMesh.indices.at(5) == 5,
    "merged mesh should append vertices and adjust indices");
  modassert(mergedMesh.vertices.at(0).position.x == 2.f && mergedMesh.vertices.at(3).position.x == 10.f,
    "merged vertices should include their source node transforms");
  modassert(mergedMesh.boundInfo.xMin == 1.f && mergedMesh.boundInfo.xMax == 14.f &&
    mergedMesh.boundInfo.yMin == -5.f && mergedMesh.boundInfo.yMax == 5.f &&
    mergedMesh.boundInfo.zMin == -6.f && mergedMesh.boundInfo.zMax == 6.f,
    "merged mesh bounds should be the union of source bounds");
  modassert(merged.nodeTransform.size() == 1 && merged.names.size() == 1 &&
    merged.names.at(0) == "model" && merged.nodeToMeshId.size() == 1 &&
    merged.nodeToMeshId.at(0).size() == 1,
    "static merged models should collapse to one root node with the merged geometry");
}

std::vector<TestCase> tests = { 
  TestCase{
    .name = "sample_test",
    .test = sampleTest,
  },
  TestCase{
    .name = "merge_model_data",
    .test = mergeModelDataTest,
  },
  TestCase{
    .name = "merge_common_meshes",
    .test = mergeCommonMeshesTest,
  },
  /*TestCase {
    .name = "sandboxBasicDeserialization",
    .test = sandboxBasicDeserialization,
  },
  TestCase {
    .name = "sandboxParentPosition",
    .test = sandboxParentPosition,
  },
  TestCase {
    .name = "sandboxMakeParentPosition",
    .test = sandboxMakeParentPosition,
  },
  TestCase {
    .name = "sandboxUpdateParentRelative",
    .test = sandboxUpdateParentRelative,
  },
  TestCase {
    .name = "sandboxUpdateParentAbsolute",
    .test = sandboxUpdateParentAbsolute,
  },
  TestCase {
    .name = "sandboxUpdateParentAndChildRelative",
    .test = sandboxUpdateParentAndChildRelative,
  },
  TestCase {
    .name = "sandboxRelativeTransform",
    .test = sandboxRelativeTransform,
  },
  TestCase {
    .name = "sandboxRemoveSceneTest",
    .test = sandboxRemoveSceneTest,
  },
  TestCase {
    .name = "sandboxRemoveSceneParentTest",
    .test = sandboxRemoveSceneParentTest,
  },*/
  TestCase {
    .name = "moveRelativeIdentityTest",
    .test = moveRelativeIdentityTest,
  },
  TestCase {
    .name = "moveRelativeRotateRight",
    .test = moveRelativeRotateRight,
  },
  TestCase {
    .name = "calcLineIntersectionTest",
    .test = calcLineIntersectionTest,
  },
  TestCase {
    .name = "utilParseAndSerializeQuat",
    .test = utilParseAndSerializeQuatTest,
  },
  TestCase {
    .name = "orientationFromPosTest",
    .test = orientationFromPosTest,
  },
  TestCase {
    .name = "envSubstTest",
    .test = envSubstTest,
  },
  TestCase {
    .name = "isInFolderTest",
    .test = isInFolderTest,
  },
  TestCase {
    .name = "directionToQuatConversionTest",
    .test = directionToQuatConversionTest,
  },
  TestCase {
    .name = "modlayerPathTest",
    .test = modlayerPathTest,
  },
  TestCase {
    .name = "planeIntersectionTest",
    .test = planeIntersectionTest,
  },
};

int runTests(){
  int totalTests = tests.size();
  int numFailures = 0;
  for (int i = 0; i < tests.size(); i++){
    auto test = tests.at(i);
    try {
      std::cout << "running test: " << i << std::endl;
      test.test();
      std::string value = std::to_string(i) + std::string(" : ") + std::string(test.name) + std::string(" : pass\n"); 
      printColor(value, CONSOLE_COLOR_GREEN);
    }catch(std::logic_error ex){
      std::string value = std::to_string(i) + std::string(" : ") + std::string(test.name) + std::string(" : fail : ") + ex.what() + std::string("\n"); 
      printColor(value, CONSOLE_COLOR_RED);
      numFailures++;
    }
  }
  std::cout << "{ \"total\" : " << totalTests << ", \"passed\" : " << totalTests - numFailures << " } " << std::endl;
  return numFailures == 0 ? 0 : 1;
}


///Integration tests ///////////////
std::vector<IntegrationTest> integrationTests {
  basicMakeObjectTest,
  checkUnloadSceneTest,
  parentSceneTest,
  prefabParentingTest,
  prefabParentingTest2,
};

TestRunInformation createIntegrationTest(){
  return TestRunInformation {
    .currentTestIndex = std::nullopt,
    .testStartTime = std::nullopt,
    .waitUntil = std::nullopt,
    .test = NULL,
    .testData = (void*)NULL,
    .sceneId = std::nullopt,
  };
}

void unloadAllTestScenes(){
  modlog("test integration", "unloadAllTestScenes");
  auto integrationTestingScenes = mainApi -> listScenes(sceneTags);
  for (auto sceneId : integrationTestingScenes){
    mainApi -> unloadScene(sceneId);
  }
}
void loadTest(TestRunInformation& runInformation, int testIndex){
  modlog("test integration loading", std::to_string(testIndex));
  runInformation.currentTestIndex = testIndex;
  runInformation.testStartTime = mainApi -> timeSeconds(true);
  runInformation.test = &integrationTests.at(runInformation.currentTestIndex.value());
  runInformation.testData = runInformation.test -> createTestData();
  runInformation.sceneId = std::nullopt;

  unloadAllTestScenes();
  runInformation.sceneId = mainApi -> loadScene("./res/scenes/empty.p.rawscene",{}, std::nullopt, sceneTags);

  auto allScenes = mainApi -> listScenes(sceneTags);
  modlog("test integration loaded scenes", print(allScenes));
}

bool runIntegrationTests(TestRunInformation& runInformation){
  if (runInformation.testResults.has_value()){
    return true;
  }
  if (!runInformation.currentTestIndex.has_value()){
    loadTest(runInformation, 0);
  }

  if (runInformation.waitUntil.has_value()){
    if (mainApi -> timeSeconds(true) > runInformation.waitUntil.value()){
      runInformation.waitUntil = std::nullopt;
    }else{
      return false;
    }
  }

  auto testResult = runInformation.test -> test(runInformation.testData, runInformation.sceneId.value());
  auto integrationWaitTime = (testResult.has_value() ? std::get_if<IntegTestWaitTime>(&testResult.value()) : NULL);
  if (integrationWaitTime){
    runInformation.waitUntil = integrationWaitTime -> time;
    return false;
  }

  auto currentTime = mainApi -> timeSeconds(true);
  bool timedOut = runInformation.test -> timeout.has_value() && (currentTime - runInformation.testStartTime.value() > runInformation.test -> timeout.value());

  bool testFinished = (testResult.has_value() && std::get_if<IntegTestResult>(&testResult.value())) || timedOut;
  bool moreTestsToLoad = (runInformation.currentTestIndex.value() + 1) < integrationTests.size();
  bool doneTesting = !moreTestsToLoad && testFinished;

  if (testFinished){
    runInformation.test = NULL;
    runInformation.testData = (void*)NULL;
    runInformation.sceneId = std::nullopt;
    runInformation.testStartTime = std::nullopt;
    if (testResult.has_value()){
      auto testResultValue = std::get_if<IntegTestResult>(&testResult.value());
      if (testResultValue){
        if (testResultValue -> passed){
          runInformation.totalPassed++;
        }else{
          std::cout << "test integration: " << print(testResultValue -> reason) << std::endl;
        }
      }
    }
  }

  if (moreTestsToLoad && testFinished){
    loadTest(runInformation, runInformation.currentTestIndex.value() + 1);
  }
  if (doneTesting){
    unloadAllTestScenes();
    runInformation.testResults = TestResults {
      .totalTests = static_cast<int>(integrationTests.size()),
      .testsPassed = runInformation.totalPassed,
    };
  }
  return doneTesting;
}

std::string testResultsStr(TestResults& testResults){
  std::string value;
  value += std::string("test integration tests = ") + std::to_string(testResults.totalTests) + "\n";
  value += std::string("test integration passed = ") + std::to_string(testResults.testsPassed) + "\n";
  value += std::string("test integration failed = ") + std::to_string(testResults.totalTests - testResults.testsPassed) + "\n";
  return value;
}


/////////////////////////////////////

std::unordered_map<std::string, FeatureScene> featureScenes = {
  { "rotation", FeatureScene {
    .sceneFile = "./res/scenes/features/rotation.p.rawscene",
    .createBinding = std::nullopt,
  }},

  // Lighting
  { "shader", FeatureScene {
    .sceneFile = "./res/scenes/features/lighting/customshader.p.rawscene",
    .createBinding = std::nullopt,
  }},

  { "tint", FeatureScene {
    .sceneFile = "./res/scenes/features/lighting/tint.p.rawscene",
    .createBinding = std::nullopt,
  }},
  { "emission", FeatureScene {
    .sceneFile = "./res/scenes/features/lighting/emission.p.rawscene",
    .createBinding = cscriptCreateEmissionBinding,
    .scriptAuto = true,
  }},
  { "lights", FeatureScene {
    .sceneFile = "./res/scenes/features/lighting/lights.p.rawscene",
    .createBinding = std::nullopt,
  }},
  { "lights-dir", FeatureScene {
    .sceneFile = "./res/scenes/features/lighting/light_directional.p.rawscene",
    .createBinding = std::nullopt,
  }},
  { "lights-spotlight", FeatureScene {
    .sceneFile = "./res/scenes/features/lighting/light_spotlight.p.rawscene",
    .createBinding = std::nullopt,
  }},
  { "transparency", FeatureScene {
    .sceneFile = "./res/scenes/features/lighting/transparency.p.rawscene",
    .createBinding = std::nullopt,
  }},
  { "normal", FeatureScene {
    .sceneFile = "./res/scenes/features/lighting/normal.p.rawscene",
    .createBinding = std::nullopt,
  }},

  // Objtypes
  { "camera", FeatureScene {
    .sceneFile = "./res/scenes/features/objtypes/camera.p.rawscene",
    .createBinding = std::nullopt,
  }},
  { "camera-dof", FeatureScene {
    .sceneFile = "./res/scenes/features/objtypes/camera-dof.p.rawscene",
    .createBinding = std::nullopt,
  }},
  { "emitter-subelement", FeatureScene {
    .sceneFile = "./res/scenes/features/objtypes/emitter-subelement.p.rawscene",
    .createBinding = std::nullopt,
  }},
  { "emitter", FeatureScene {
    .sceneFile = "./res/scenes/features/objtypes/emitter.p.rawscene",
    .createBinding = std::nullopt,
  }},
  { "octree", FeatureScene {
    .sceneFile = "./res/scenes/features/objtypes/octree.p.rawscene",
    .createBinding = std::nullopt,
  }},
  { "portal-fixed", FeatureScene {
    .sceneFile = "./res/scenes/features/objtypes/portal-fixed.p.rawscene",
    .createBinding = std::nullopt,
  }},
  { "portal", FeatureScene {
    .sceneFile = "./res/scenes/features/objtypes/portal.p.rawscene",
    .createBinding = std::nullopt,
  }},
  { "sound", FeatureScene {
    .sceneFile = "./res/scenes/features/objtypes/sounds.p.rawscene",
    .createBinding = cscriptSoundBinding,
  }},
  { "sound-loop", FeatureScene {
    .sceneFile = "./res/scenes/features/objtypes/soundslooping.p.rawscene",
    .createBinding = cscriptSoundBinding,
  }},
  // Physics 
  { "physics.gravity", FeatureScene {
    .sceneFile = "./res/scenes/features/physics/gravity.rawscene",
    .createBinding = std::nullopt,
  }},  
  { "physics.collisiontypes", FeatureScene {
    .sceneFile = "./res/scenes/features/physics/collisiontypes.p.rawscene",
    .createBinding = std::nullopt,
  }},  
  { "physics.exactshape", FeatureScene {
    .sceneFile = "./res/scenes/features/physics/exactshape.p.rawscene",
    .createBinding = std::nullopt,
  }},  
  { "physics.layers", FeatureScene {
    .sceneFile = "./res/scenes/features/physics/physics-layers.p.rawscene",
    .createBinding = std::nullopt,
  }},  
  { "physics.velocity", FeatureScene {
    .sceneFile = "./res/scenes/features/physics/velocity.rawscene",
    .createBinding = std::nullopt,
  }},  


  // Textures
  { "subimage", FeatureScene {
    .sceneFile = "./res/scenes/features/textures/subimage.p.rawscene",
    .createBinding = std::nullopt,
  }},  

  { "lookat", FeatureScene {
    .sceneFile = "./res/scenes/features/lookat.p.rawscene",
    .createBinding = std::nullopt,
  }},  

  { "selection", FeatureScene {
    .sceneFile = "./res/scenes/features/scripting/selection.p.rawscene",
    .createBinding = cscriptCreateSelectionBinding,
  }},  

  // Scenegraph
  { "parent", FeatureScene {
    .sceneFile = "./res/scenes/features/scenegraph/parent.p.rawscene",
    .createBinding = std::nullopt,
  }},


  // Misc 
  { "screenshot", FeatureScene {
    .sceneFile = "./res/scenes/features/scripting/screenshot.rawscene",
    .createBinding = cscriptCreateScreenshotBinding,
  }},  
  { "text", FeatureScene {
    .sceneFile = std::nullopt,
    .createBinding = cscriptCreateTextBinding,
    .scriptAuto = true,
  }},

  { "time", FeatureScene {
    .sceneFile = std::nullopt,
    .createBinding = cscriptCreateTimeBinding,
    .scriptAuto = true,
  }},
  {
    "nobjects", FeatureScene {
      .sceneFile = std::nullopt,
      .createBinding = cscriptCreateNObjectsBinding,
      .scriptAuto = true,
  }},
  {
    "animation", FeatureScene {
      .sceneFile = std::nullopt,
      .createBinding = cscriptCreateAnimationBinding,
      .scriptAuto = true,   
  }},
};


std::string printFeatures(){
  std::string value = "";
  for (auto &[name, scene] : featureScenes){
    value += name + " - " + print(scene.sceneFile) + "\n";
  }
  return value;
}

void printFeatureSceneHelp(){
  std::cout << printFeatures() << std::endl;
}

FeatureScene& getFeatureScene(std::string name){
  if (featureScenes.find(name) == featureScenes.end()){
    modassert(false, "invalid feature scene name");
  }
  FeatureScene& featureScene = featureScenes.at(name);
  return featureScene;
}
void runFeatureScene(std::string name){
  if (featureScenes.find(name) == featureScenes.end()){
    modassert(false, "invalid feature scene name");
  }
  FeatureScene& featureScene = featureScenes.at(name);
  auto sceneId = featureScene.sceneFile.has_value() ?
    mainApi -> loadScene(featureScene.sceneFile.value(), {}, std::nullopt, sceneTags) :
    mainApi -> loadScene("./res/scenes/example.p.rawscene", {}, std::nullopt, sceneTags);
  

  std::unordered_map<std::string, GameobjAttributes> submodelAttributes = {};
  if (featureScene.scriptAuto && featureScene.createBinding.has_value()){
    auto bindingName = featureScene.createBinding.value()(*mainApi).bindingMatcher;
    GameobjAttributes attr = {
      .attr = {
        { "script", bindingName },
      },
    };
    mainApi -> makeObjectAttr(sceneId, std::string("testscript"), attr, submodelAttributes).value();
  }
}