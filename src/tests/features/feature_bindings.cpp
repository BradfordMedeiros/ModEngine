#include "./feature_bindings.h"

CScriptBinding cscriptCreateSelectionBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("test/test-selection", api);
  binding.onObjectHover = [](int32_t id, void* data, int32_t index, bool hoverOn) -> void {
  	if (hoverOn){
  		modlog("test/test-selection", "hover on");
  	}else{
  		modlog("test/test-selection", "hover off");
  	}
 		modlog("test/test-selection", std::to_string(index));
  };
  return binding;
}

CScriptBinding cscriptCreateScreenshotBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("test/test-screenshot", api);
 
  binding.onKeyCharCallback = [&api](int32_t id, void* data, unsigned int key) -> void {
    if (key == 'p'){
      api.saveScreenshot("./res/data/screenshots/screenshot.png");
      modlog("test/test-screenshot", "take screenshot");
    }
  };

  return binding;
}

CScriptBinding cscriptCreateTextBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("test/test-text", api);
  binding.onFrame = [&api](int32_t id, void* data) -> void {
    modlog("test/test-text", "onFrame");
    const char* str = "a b c d e\n\nf g h i j\n\nk l m n o\n\np q r s t\n\nu v w x y\n\nz\n! @ # $ %\n\n^ & * ( )\n\n_ + -\n1 2 3 4 5\n\n6 7 8 9 0\n\n";
    api.drawText(str, 0, 0.f, 10.f, false, glm::vec4(1.f, 1.f, 1.f, 1.f), std::nullopt, true, std::nullopt, std::nullopt, std::nullopt, std::nullopt);
  };
  return binding;
}


CScriptBinding cscriptCreateTimeBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("test/test-text", api);
  binding.onFrame = [&api](int32_t id, void* data) -> void {
    modlog("time - not realtime", std::to_string(api.timeSeconds(false)));
    modlog("time - realtime", std::to_string(api.timeSeconds(true)));
    modlog("time - elapsed", std::to_string(api.timeElapsed()));
  };
  return binding;
}

CScriptBinding cscriptSoundBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("test/test-sound", api);
  binding.onMouseCallback = [&api](objid id, void* data, int button, int action, int mods) -> void {
    if (action != 1){
      return;
    }
    auto sceneId = api.listSceneId(id);
    auto sampleId = api.getGameObjectByName("&sample", sceneId).value();
    api.playClipById(sampleId, std::nullopt, std::nullopt);
  };
  return binding;
}

CScriptBinding cscriptCreateEmissionBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("test/test-sound", api);
  binding.onFrame = [&api](int32_t id, void* data) -> void {
    auto sceneId = api.listSceneId(id);
    auto cubeId = api.getGameObjectByName("boxfront/Cube", sceneId).value();

    auto currentTime = api.timeSeconds(false);

    float duration = 5.f;
    float remainingTime = currentTime - (duration * static_cast<int>(currentTime / duration));

    modlog("emission remaining time", std::to_string(remainingTime));
    glm::vec3 emissionAmount(0.f, remainingTime / duration, 0.f);
    api.setSingleGameObjectAttr(cubeId, "emission", emissionAmount);
  };
  return binding; 
}


int addNCounter = 0;
std::set<objid> addNObjects(CustomApiBindings& gameapi, objid sceneId, int width, int height, int depth, std::string mesh){
  auto start = std::chrono::system_clock::now();
  std::set<objid> ids;
  for (int x = 0; x < width; x++){
    for (int z = 0; z < depth; z++){
      for (int y = 0; y < height; y++){
        float xoffset = 2.5f * x;
        float yoffset = 2.5f * y;
        float zoffset = 2.5f * z;
        GameobjAttributes attr {
          .attr = {
            { "mesh", mesh },
            //{ "texture", "./res/textures/hexglow.png" },
          },
        };
        std::unordered_map<std::string, GameobjAttributes> submodelAttributes;
        auto name = std::string("debug-obj-") + std::to_string(addNCounter);
        addNCounter++;
        auto id = gameapi.makeObjectAttr(sceneId, name, attr, submodelAttributes);
        std::cout << "addNObjects: " << addNCounter << std::endl;
        gameapi.setGameObjectPosition(gameapi.groupId(id.value()), glm::vec3(xoffset, yoffset, zoffset), true, Hint{});
        ids.insert(id.value()); 
      }
    }
  }

  auto end = std::chrono::system_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start);
  std::cout << "addNObjects seconds: " << duration.count() << std::endl;

  return ids;
}

CScriptBinding cscriptCreateNObjectsBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("test/nobjects", api);
  binding.onFrame = [&api](int32_t id, void* data) -> void {
    static bool didAddObjects = false;
    if (!didAddObjects){
      didAddObjects = true;

      auto args = api.getArgs();
      int x = 1;
      if (args.find("x") != args.end()){
        x = std::atoi(args.at("x").c_str());
      }
      int y = 1;
      if (args.find("y") != args.end()){
        y = std::atoi(args.at("y").c_str());
      }
      int z = 1;
      if (args.find("z") != args.end()){
        z = std::atoi(args.at("z").c_str());
      }

      addNObjects(api, 0, x, y, z,"../gameresources/build/primitives/walls/1-0.2-1.gltf");
    }

  };

  return binding; 
}

CScriptBinding cscriptCreateAnimationBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("test/animation", api);
  binding.onFrame = [&api](int32_t id, void* data) -> void {
    static bool didAddObjects = false;
    if (!didAddObjects){
      didAddObjects = true;

      auto args = api.getArgs();
      int x = 1;
      if (args.find("x") != args.end()){
        x = std::atoi(args.at("x").c_str());
      }
      int y = 1;
      if (args.find("y") != args.end()){
        y = std::atoi(args.at("y").c_str());
      }
      int z = 1;
      if (args.find("z") != args.end()){
        z = std::atoi(args.at("z").c_str());
      }

      auto ids = addNObjects(api, 0, x, y, z, "../gameresources/build/characters/plaguerobot.gltf");
      for (auto id : ids){
        api.playAnimation(id, "walk", LOOP, {}, 0);
      }
    }

  };

  return binding; 
}