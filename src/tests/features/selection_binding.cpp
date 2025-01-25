#include "./selection_binding.h"

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
    auto sampleId = api.getGameObjectByName("&sample", sceneId, true).value();
    api.playClipById(sampleId, std::nullopt, std::nullopt);
  };
  return binding;
}

CScriptBinding cscriptCreateEmissionBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("test/test-sound", api);
  binding.onFrame = [&api](int32_t id, void* data) -> void {
    auto sceneId = api.listSceneId(id);
    auto cubeId = api.getGameObjectByName("boxfront/Cube", sceneId, true).value();

    auto currentTime = api.timeSeconds(false);

    float duration = 5.f;
    float remainingTime = currentTime - (duration * static_cast<int>(currentTime / duration));

    modlog("emission remaining time", std::to_string(remainingTime));
    glm::vec3 emissionAmount(0.f, remainingTime / duration, 0.f);
    api.setSingleGameObjectAttr(cubeId, "emission", emissionAmount);
  };
  return binding; 
}