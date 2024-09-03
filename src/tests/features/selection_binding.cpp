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
