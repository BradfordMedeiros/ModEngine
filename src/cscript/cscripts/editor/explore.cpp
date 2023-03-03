#include "./explore.h"

extern CustomApiBindings* mainApi;

struct EditorExplore {
   std::optional<objid> hoveredObjId;
};


CScriptBinding cscriptExploreBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/explore", api);

  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    EditorExplore* explore = new EditorExplore;
    explore -> hoveredObjId = std::nullopt;
    return explore;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    EditorExplore* explore = static_cast<EditorExplore*>(data);
    delete explore;
  };

  binding.onObjectHover = [](objid scriptId, void* data, int32_t index, bool hoverOn) -> void {
    EditorExplore* explore = static_cast<EditorExplore*>(data);
    if (hoverOn){
      explore -> hoveredObjId = index;
    }else{
      explore -> hoveredObjId = std::nullopt;
    }
  };

  binding.onMouseCallback = [](objid id, void* data, int button, int action, int mods) -> void {
    EditorExplore* explore = static_cast<EditorExplore*>(data);
    if (button == 0 && action == 0 && explore -> hoveredObjId.has_value()){
      auto sceneIdHovered = mainApi -> listSceneId(explore -> hoveredObjId.value());
      auto mainIdScene = mainApi -> listSceneId(id);
      if (sceneIdHovered == mainIdScene){
        auto attr = mainApi -> getGameObjectAttr(explore -> hoveredObjId.value());
        auto onClick = getStrAttr(attr, "onclick");
        if (onClick.has_value()){
          mainApi -> sendNotifyMessage("explorer", onClick.value());
        }
      }
    }
  };

  return binding;
}

