#include "./explorer_loader.h"

extern CustomApiBindings* mainApi;

struct EditorExploreScenegraph {
	std::string name;
	std::string title;
	std::string topic;
	std::string texturePath;
	int basenumber;
	std::vector<std::string> values;
};

void createScenegraph(objid sceneId, EditorExploreScenegraph scenegraph){
  GameobjAttributes attr {
    .stringAttributes = {
    	{ "script", "native/scenegraph" },
      { "depgraph", "raw" },
      { "gentexture", scenegraph.texturePath },
      { "title", scenegraph.title },
      { "topic", scenegraph.topic },
      { "values", join(scenegraph.values, '|') },
    },
    .numAttributes = {
    	{ "basenumber", scenegraph.basenumber },
    },
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  mainApi -> makeObjectAttr(sceneId, scenegraph.name, attr, submodelAttributes);
}

struct EditorExplorerLoader {
  std::optional<objid> explorerInstance;
	std::optional<std::string> explorerSoundValue;
	std::optional<std::string> explorerHeightmapValue;
	std::optional<std::string> explorerHeightmapBrushValue;
};

struct ExplorerData {
  std::string title;
  std::string texture;
};
std::map<std::string, ExplorerData> messageToExplorer = {
  { "load-test", ExplorerData { .title = "Test Menu", .texture = "./res/textures/wood.jpg" }},
  { "load-sound", ExplorerData { .title = "Sound Values", .texture = "explorer-gentexture-sound" }},
  { "load-heightmap", ExplorerData { .title = "Heightmap Values", .texture = "explorer-gentexture-heightmap" }},
  { "load-heightmap-brush", ExplorerData { .title = "Heightmap Brushes", .texture = "explorer-gentexture-heightmap-brush" }},
};

void loadExplorer(EditorExplorerLoader& explorerLoader, std::string key){
  modlog("editor", "scenegraph load instance: " + key);
  auto value = messageToExplorer.at(key);
  auto sceneId = mainApi -> loadScene(
    "./res/scenes/editor/explore.rawscene", { 
      { "(dialog", "tint", "0 1 1 0.2" },
      { ")text_main", "value", value.title },
      { "*basicbutton1", "ontexture", value.texture },
    }, 
    std::nullopt,  
    std::optional<std::vector<std::string>>({ "editor" })
  );
  modassert(!explorerLoader.explorerInstance.has_value(),  "explorer loader should not have an instance");
  explorerLoader.explorerInstance = sceneId;
}
void explorerUnload(EditorExplorerLoader& explorerLoader){
  if (explorerLoader.explorerInstance.has_value()){
    mainApi -> unloadScene(explorerLoader.explorerInstance.value());
    explorerLoader.explorerInstance = std::nullopt;
  }
}
void explorerHandleLoad(EditorExplorerLoader& explorerLoader, std::string& value){
  explorerUnload(explorerLoader);
  loadExplorer(explorerLoader, value);
}

CScriptBinding cscriptExplorerLoaderBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/explorer-loader", api);

  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
  	EditorExplorerLoader* explorerLoader = new EditorExplorerLoader;
    explorerLoader -> explorerInstance = std::nullopt;
  	explorerLoader -> explorerSoundValue = std::nullopt;
  	explorerLoader -> explorerHeightmapValue = std::nullopt;
  	explorerLoader -> explorerHeightmapBrushValue = std::nullopt;

    createScenegraph(sceneId, EditorExploreScenegraph {
      .name = "|fileexplorer-sound",
      .title = "Sound List",
      .topic = "explorer-sound",
      .texturePath = "explorer-gentexture-sound",
      .basenumber = 30000,
      .values = mainApi -> listResources("sounds"),
    });
    createScenegraph(sceneId, EditorExploreScenegraph {
      .name = "|fileexplorer-heightmap",
      .title = "Heightmap List",
      .topic = "explorer-heightmap",
      .texturePath = "explorer-gentexture-heightmap",
      .basenumber = 40000,
      .values = mainApi -> listResources("heightmaps"),
    });
    createScenegraph(sceneId, EditorExploreScenegraph {
      .name = "|fileexplorer-heightmap-brush",
      .title = "Heightmap Brushes",
      .topic = "explorer-heightmap-brush",
      .texturePath = "explorer-gentexture-heightmap-brush",
      .basenumber = 50000,
      .values = mainApi -> listResources("heightmap-brushes"),
    });

    return explorerLoader;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
  	EditorExplorerLoader* explorerLoader = static_cast<EditorExplorerLoader*>(data);
  	delete explorerLoader;
  };

  binding.onMessage = [](objid scriptId, void* data, std::string& topic, AttributeValue& value) -> void {
  	EditorExplorerLoader* explorerLoader = static_cast<EditorExplorerLoader*>(data);
  	if (topic == "explorer-sound"){
  		auto val = std::get_if<std::string>(&value);
  		modassert(val, "explorer-sound value must be string");
  		explorerLoader -> explorerSoundValue = *val;
  	}else if (topic == "explorer-heightmap"){
  		auto val = std::get_if<std::string>(&value);
  		modassert(val, "explorer-heightmap value must be string");
  		explorerLoader -> explorerHeightmapValue = *val;
  	}else if (topic == "explorer-heightmap-brush"){
  		auto val = std::get_if<std::string>(&value);
  		modassert(val, "explorer-heightmap-brush value must be string");
  		explorerLoader -> explorerHeightmapBrushValue = *val;
  	}else if (topic == "explorer"){
  		auto val = std::get_if<std::string>(&value);
  		modassert(val, "explorer key value must be a string");
  		if (*val == "explorer-ok"){
        if (explorerLoader -> explorerSoundValue.has_value()){
          mainApi -> sendNotifyMessage("explorer-sound-final", explorerLoader -> explorerSoundValue.value());
          explorerLoader -> explorerSoundValue = std::nullopt;
        }
        if (explorerLoader -> explorerHeightmapValue.has_value()){
          mainApi -> sendNotifyMessage("explorer-heightmap-final", explorerLoader -> explorerHeightmapValue.value());
          explorerLoader -> explorerHeightmapValue = std::nullopt;
        }
        if (explorerLoader -> explorerHeightmapBrushValue.has_value()){
          mainApi -> sendNotifyMessage("explorer-heightmap-brush-final", explorerLoader -> explorerHeightmapBrushValue.value());

          explorerLoader -> explorerHeightmapBrushValue = std::nullopt;
        }
		    explorerUnload(*explorerLoader);
  		}else if (*val == "explorer-cancel"){
  			explorerUnload(*explorerLoader);
  		}else if (*val == "load-sound" || *val == "load-test" || *val == "load-heightmap" || *val == "load-heightmap-brush"){
  			explorerHandleLoad(*explorerLoader, *val);
  		}
  	}

  };

  return binding;
}



