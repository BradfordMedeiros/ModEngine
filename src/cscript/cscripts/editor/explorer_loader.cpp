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

void createScenegraph(objid sceneId, EditorExploreScenegraph& scenegraph){
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
	std::optional<std::string> explorerSoundValue;
	std::optional<std::string> explorerHeightmapValue;
	std::optional<std::string> explorerHeightmapBrushValue;
};

void loadExplorer(){
	/*
	(define (loadExplorer key)
  (define value (assoc key messageToExplorer))
  (define title (cadr value))
  (define texture (caddr value))
  (define sceneId 
    (load-scene 
      "./res/scenes/editor/explore.rawscene"
      (list
        (list "(dialog" "tint" "0 1 1 0.2")
        (list ")text_main" "value" title)
        (list "*basicbutton1" "ontexture" texture)
      )
    )
  )
  (format #t "load explorer: ~a\n" value)
  (set! explorerInstance sceneId)
)
*/
}
void explorerUnload(){
	/* (define explorerInstance #f)

(define (unloadExplorer)
  (if explorerInstance
    (unload-scene explorerInstance)
  )
  (set! explorerInstance #f)
)
*/
}
void explorerHandleLoad(std::string& value){
/*
(define (handleLoad value)
  (unloadExplorer)
  (loadExplorer value)
)
*/
}

CScriptBinding cscriptExplorerLoaderBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/explorer-loader", api);

  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
  	EditorExplorerLoader* explorerLoader = new EditorExplorerLoader;
  	explorerLoader -> explorerSoundValue = std::nullopt;
  	explorerLoader -> explorerHeightmapValue = std::nullopt;
  	explorerLoader -> explorerHeightmapBrushValue = std::nullopt;
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
				/*
        (begin
          (if explorerSoundValue
            (send "explorer-sound-final" explorerSoundValue)
          )
          (set! explorerSoundValue #f)
 
          (if explorerHeightmapBrushValue
            (send "explorer-heightmap-brush-final" explorerHeightmapBrushValue)
          )
          (set! explorerHeightmapBrushValue #f)        

          (if explorerHeightmapValue
            (send "explorer-heightmap-final" explorerHeightmapValue)
          )
          (set! explorerHeightmapValue #f)
          (unloadExplorer)
        )*/
  		}else if (*val == "explorer-cancel"){
  			//  (unloadExplorer)
  			explorerUnload();
  		}else if (*val == "load-sound" || *val == "load-test" || *val == "load-heightmap" || *val == "load-heightmap-brush"){
  			explorerHandleLoad(*val);
  		}
  	}

  };

  return binding;
}

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


/*


(createScenegraph 
  "|fileexplorer-sound"
  "Sound List"
  "explorer-sound"
  "explorer-gentexture-sound"
  30000
  (ls-res "sounds")
)
;
;(createScenegraph
;  "|fileexplorer-heightmap"
;  "Heightmap List"
;  "explorer-heightmap"
;  "explorer-gentexture-heightmap"
;  40000
;  (append (ls-res "heightmaps"))
;)
;
;(createScenegraph
;  "|fileexplorer-heightmap-brush"
;  "Heightmap Brushes"
;  "explorer-heightmap-brush"
;  "explorer-gentexture-heightmap-brush"
;  50000
;  (append (ls-res "heightmap-brushes"))
;)
*/