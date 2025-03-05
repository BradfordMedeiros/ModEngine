#include "./tools.h"

extern CustomApiBindings* mainApi;


CScriptBinding cscriptCreateToolsBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/tools", api);
  binding.onFrame = [](int32_t id, void* data) -> void {
    auto selectedIds = mainApi -> selected();
  	for (auto idInScene : selectedIds){
  		auto groupId = mainApi -> groupId(idInScene);


      if (true ){
      	auto name = mainApi -> getGameObjNameForId(idInScene).value();
        auto position = mainApi -> getGameObjectPos(idInScene, true);
        auto rotation = mainApi -> getGameObjectRotation(idInScene, true);
        auto toPosition = position + (rotation * glm::vec3(0.f, 0.f, -1.f));
        auto leftArrow = position + (rotation * glm::vec3(-0.2f, 0.f, -0.8f));
        auto rightArrow = position + (rotation * glm::vec3(0.2f, 0.f, -0.8f));

				mainApi -> drawLine(position, toPosition, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
				mainApi -> drawLine(leftArrow, toPosition, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
				mainApi -> drawLine(rightArrow, toPosition, false, -1, glm::vec4(1.f, 0.f, 0.f, 1.f), std::nullopt, std::nullopt);
      	modlog("tools", print(position) + " " + print(toPosition));
      	modlog("tools name", name);
      }
  	}
  };

  return binding;
}
