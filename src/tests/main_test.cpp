#include "./main_test.h"

extern CustomApiBindings* mainApi;

///Unit tests ///////////////
struct TestCase {
  const char* name;
  std::function<void()> test;
};

void sampleTest(){ 
  //throw std::logic_error("error loading buffer");
}

std::vector<TestCase> tests = { 
  TestCase{
    .name = "sample_test",
    .test = sampleTest,
  },
  TestCase {
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
  },
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

enum CONSOLE_COLOR { CONSOLE_COLOR_GREEN, CONSOLE_COLOR_RED };
void printColor(std::string str, std::optional<CONSOLE_COLOR> color){
  if (!color.has_value()){
    std::cout << str;
    return;
  }
  if (color.value() == CONSOLE_COLOR_GREEN){
    std::cout << "\033[1;32m" << str << "\033[0m\n";
  }else if (color.value() == CONSOLE_COLOR_RED){
    std::cout << "\033[1;31m" << str << "\033[0m\n";
  }else{
    modassert(false, "invalid color");
  }
}

int runTests(){
  int totalTests = tests.size();
  int numFailures = 0;
  for (int i = 0; i < tests.size(); i++){
    auto test = tests.at(i);
    try {
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

std::map<std::string, FeatureScene> featureScenes = {
  // Lighting
  { "tint", FeatureScene {
    .sceneFile = "./res/scenes/features/lighting/tint.p.rawscene",
    .createBinding = std::nullopt,
  }},
  { "emission", FeatureScene {
    .sceneFile = "./res/scenes/features/lighting/emission.p.rawscene",
    .createBinding = std::nullopt,
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
  { "camera-interp", FeatureScene {
    .sceneFile = "./res/scenes/features/objtypes/camera-interp.p.rawscene",
    .createBinding = cscriptCreateInterpBinding,
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
  

  std::map<std::string, GameobjAttributes> submodelAttributes = {};
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