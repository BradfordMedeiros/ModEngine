#include "./scenegraph.h"

extern CustomApiBindings* mainApi;

struct SceneDependency {
	std::string element;
	std::string depends;
	int elementScene;
	int dependsScene;
	objid parentId;
	objid childId;
};


struct EditorScenegraph {
	objid mainObjId;
	bool didScroll;
	float offset;
	float maxOffset;
	float minOffset;
	int selectedIndex;
	int maxIndex;

	std::string depgraphType;
	std::optional<std::vector<SceneDependency>> depGraph;
	std::optional<objid> textureId;

	int fontSize;
	int mappingOffset;

	std::map<int, bool> idToExpandState;
};

std::vector<SceneDependency> getNoData(EditorScenegraph& scenegraph){
	return { 
		SceneDependency { .element = "data", .depends = "none available", .elementScene = 0, .dependsScene = 0 } 
	};
}

struct IdCreator {
	int idIterator;
	std::map<std::pair<std::string, int>, int> pairToId;
};
IdCreator makeIdCreator(){
	return IdCreator { .idIterator = -1, .pairToId = { }};
}
int createId(IdCreator& idCreator, std::string name, int sceneId){
	auto idPair = std::make_pair(name, sceneId);
	if (idCreator.pairToId.find(idPair) == idCreator.pairToId.end()){
		idCreator.idIterator++;
		idCreator.pairToId[idPair] = idCreator.idIterator;
		return idCreator.idIterator;
	}
	return idCreator.pairToId.at(idPair);
}

std::vector<SceneDependency> getMockScenegraph(EditorScenegraph& scenegraph){
	return {
		SceneDependency { .element = "root", .depends = "mainfolder", .elementScene = 0, .dependsScene = 0, .parentId = 0, .childId = 1, },
		SceneDependency { .element = "root2", .depends = "mainfolder2", .elementScene = 0, .dependsScene = 1, .parentId = 2, .childId = 3 },
		SceneDependency { .element = "mainfolder", .depends = "folder1", .elementScene = 0, .dependsScene = 0, .parentId = 1, .childId = 4 },
		SceneDependency { .element = "mainfolder", .depends = "folder2", .elementScene = 0, .dependsScene = 0 },
		SceneDependency { .element = "mainfolder", .depends = "folder3", .elementScene = 0, .dependsScene = 0 },
		SceneDependency { .element = "folder3", .depends = "folder3-1", .elementScene = 0, .dependsScene = 0 },
		SceneDependency { .element = "folder3", .depends = "folder3-2", .elementScene = 0, .dependsScene = 0 },
	};
}

std::vector<SceneDependency> getFilteredScenegraph(EditorScenegraph& scenegraph){
	auto editorScenes = mainApi -> listScenes(std::optional<std::vector<std::string>>({ "editor" }));
	std::vector<SceneDependency> filteredScenes;
	for (auto &scenePair : mainApi -> scenegraph()){
		bool sceneInList = false;
		for (auto sceneId : editorScenes){
			if (scenePair.parentScene == sceneId || scenePair.childScene == sceneId){
				sceneInList = true;
				break;
			}
		}
		if (!sceneInList){
		  filteredScenes.push_back({
		  	.element = scenePair.parent,
		  	.depends = scenePair.child,
		  	.elementScene = scenePair.parentScene,
		  	.dependsScene = scenePair.childScene,
		  	.parentId = scenePair.parentId,
		  	.childId = scenePair.childId,
		  });
		}
	}
	return filteredScenes;
}
std::vector<SceneDependency> getMockModelList(EditorScenegraph& scenegraph){
	auto idCreator = makeIdCreator();
	return {
		SceneDependency { .element = "AllMeshes", .depends = "mesh1", .elementScene = 0, .dependsScene = 0, .parentId = createId(idCreator, "AllMeshes", 0), .childId = createId(idCreator, "mesh1", 0) },
		SceneDependency { .element = "AllMeshes", .depends = "mesh2", .elementScene = 0, .dependsScene = 0, .parentId = createId(idCreator, "AllMeshes", 0), .childId = createId(idCreator, "mesh2", 0) },
	};
}
std::vector<SceneDependency> getModelList(EditorScenegraph& scenegraph){
	std::vector<SceneDependency> deps;
	auto idCreator = makeIdCreator();
	for (auto &model : mainApi -> listResources("models")){
		deps.push_back(SceneDependency {
			.element = "models",
			.depends = model,
			.elementScene = 0,
			.dependsScene = 0,
			.parentId = createId(idCreator, "models", 0),
			.childId = createId(idCreator, model, 0),
		});
	}
	return deps;
}
std::vector<SceneDependency> getMockTextureList(EditorScenegraph& scenegraph){
	auto idCreator = makeIdCreator();
	return { 
		SceneDependency { 
			.element = "Textures", 
			.depends = "texture-mock", 
			.elementScene = 0, 
			.dependsScene = 0,
			.parentId = createId(idCreator, "Textures", 0),
			.childId = createId(idCreator, "texture-mock", 0),
		} 
	};
}
std::vector<SceneDependency> getTextureList(EditorScenegraph& scenegraph){
	std::vector<SceneDependency> deps;
	auto idCreator = makeIdCreator();
	for (auto &texture : mainApi -> listResources("textures")){
		deps.push_back(SceneDependency {
			.element = "textures",
			.depends = texture,
			.elementScene = 0,
			.dependsScene = 0,
			.parentId = createId(idCreator, "textures", 0),
			.childId = createId(idCreator, texture, 0),
		});
	}
	return deps;
}

std::vector<SceneDependency> getRawExplorerList(EditorScenegraph& scenegraph){
	auto attr = mainApi -> getGameObjectAttr(scenegraph.mainObjId);
	auto values = split(getStrAttr(attr, "values").value(), '|');
	auto title = getStrAttr(attr, "title").value();
	std::vector<SceneDependency> deps;

	auto idCreator = makeIdCreator();
	for (auto &value : values){
		deps.push_back(SceneDependency {
			.element = title,
			.depends = value,
			.elementScene = 0,
			.dependsScene = 0,
			.parentId = createId(idCreator, title, 0),
			.childId = createId(idCreator, value, 0),
		});
	}
	return deps;
}

struct SceneDrawElement {
	std::string elementName;
	objid sceneId;
	int depth;
	int height;
	bool expanded;
	bool isSelected;
	int mappingNumber;
	bool hasChildren;
	objid id;
};
struct SceneDrawList {
	std::vector<SceneDrawElement> elements;
};

void onGraphChange(EditorScenegraph& scenegraph);
SceneDrawList drawListFromDepGraph(EditorScenegraph& scenegraph);

objid getIdForSelectedIndex(EditorScenegraph& scenegraph){
	auto drawListElement = drawListFromDepGraph(scenegraph).elements.at(scenegraph.selectedIndex);
	return drawListElement.id;;
}
SceneDrawElement getElementForSelectedIndex(EditorScenegraph& scenegraph){
	auto drawListElement = drawListFromDepGraph(scenegraph).elements.at(scenegraph.selectedIndex);
	return drawListElement;
}

void doNothing(EditorScenegraph& scenegraph, bool isAlt){
	modlog("editor", "scenegraph doNothing");
}
void selectScenegraphItem(EditorScenegraph& scenegraph, bool isAlt){
	modlog("editor", "scenegraph selectScenegraphItem: " + std::to_string(scenegraph.selectedIndex));
	auto selectedId = getIdForSelectedIndex(scenegraph);
	if (isAlt){
	  // set camera position to target
	  //mainApi -> setGameObjectPos(cameraId, mainApi -> getGameObjectPos(selectedId, true));
	}else{
  	mainApi -> setWorldState({ 
  	  ObjectValue {
  	    .object = "editor",
  	    .attribute = "selected-index",
  	    .value = std::to_string(selectedId),
  	  }
  	});
	}
}
void selectModelItem(EditorScenegraph& scenegraph, bool isAlt){
	modlog("editor", "scenegraph selectModelItem: " + std::to_string(scenegraph.selectedIndex));
	auto element = getElementForSelectedIndex(scenegraph);
	auto objName = std::to_string(rand() % 1000000000) + "-generated";
  auto sceneId = mainApi -> listSceneId(scenegraph.mainObjId); 
  modlog("editor", "scenegraph - mkobj in editor scene, which is wrong");
  GameobjAttributes attr {
    .stringAttributes = {
     	{ "mesh", element.elementName }
    },
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  mainApi -> makeObjectAttr(sceneId, objName, attr, submodelAttributes);
}

void selectTextureItem(EditorScenegraph& scenegraph, bool isAlt){
	modlog("editor", "scenegraph selectTextureItem: " + std::to_string(scenegraph.selectedIndex));
	auto element = getElementForSelectedIndex(scenegraph);
  GameobjAttributes attr {
    .stringAttributes = { {"texture", element.elementName }},
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  for (auto targetObjId : mainApi -> selected()){
  	modlog("editor", "scenegraph set texture: " + std::to_string(targetObjId) + ", name = " + mainApi -> getGameObjNameForId(targetObjId).value() + ", with texture = " + element.elementName);
  	mainApi -> setGameObjectAttr(targetObjId, attr);
  }
}

void selectRawItem(EditorScenegraph& scenegraph, bool isAlt){
	modlog("editor", "scenegraph selectRawItem: " + std::to_string(scenegraph.selectedIndex));
	auto element = getElementForSelectedIndex(scenegraph);
	auto attr = mainApi -> getGameObjectAttr(scenegraph.mainObjId);
	auto topic = getStrAttr(attr, "topic").value();
	mainApi -> sendNotifyMessage(topic, element.elementName);
}

void onObjDoNothing(EditorScenegraph& scenegraph, objid gameobjid){
	modlog("editor", "scenegraph - onObjDoNothing");
}


void onObjectSelectedSelectItem(EditorScenegraph& scenegraph, objid gameobjid){
	auto drawList = drawListFromDepGraph(scenegraph);
	for (int i = 0; i <drawList.elements.size(); i++){
		if (drawList.elements.at(i).id == gameobjid){
			scenegraph.selectedIndex = i;
			break;
		}
	}
	onGraphChange(scenegraph);
}

struct ModeDepGraph {
	std::function<std::vector<SceneDependency>(EditorScenegraph& scenegraph)> getGraph;
	std::function<void(EditorScenegraph& scenegraph, bool)> handleItemSelected;
	bool showSceneIds;
	std::function<void(EditorScenegraph& scenegraph, objid gameobjid)> onObjectSelected;
};

std::map<std::string, ModeDepGraph> modeToGetDepGraph {
	{ "nodata", ModeDepGraph {
			.getGraph = getNoData,
			.handleItemSelected = doNothing,
			.showSceneIds = false,
			.onObjectSelected = onObjDoNothing,
	}},
	{ "mock-scenegraph", ModeDepGraph {
			.getGraph = getMockScenegraph,
			.handleItemSelected = doNothing,
			.showSceneIds = false,
			.onObjectSelected = onObjDoNothing,
	}},
	{ "scenegraph", ModeDepGraph {
			.getGraph = getFilteredScenegraph,
			.handleItemSelected = selectScenegraphItem,
			.showSceneIds = true,
			.onObjectSelected = onObjectSelectedSelectItem,
	}},
	{ "mock-models", ModeDepGraph {
			.getGraph = getMockModelList,
			.handleItemSelected = doNothing,
			.showSceneIds = false,
			.onObjectSelected = onObjDoNothing,
	}},
	{ "models", ModeDepGraph {
			.getGraph = getModelList,
			.handleItemSelected = selectModelItem,
			.showSceneIds = false,
			.onObjectSelected = onObjDoNothing,
	}},
	{ "mock-textures", ModeDepGraph {
			.getGraph = getMockTextureList,
			.handleItemSelected = doNothing,
			.showSceneIds = false,
			.onObjectSelected = onObjDoNothing,
	}},
	{ "textures", ModeDepGraph {
			.getGraph = getTextureList,
			.handleItemSelected = selectTextureItem,
			.showSceneIds = false,
			.onObjectSelected = onObjDoNothing,
	}},
	{ "raw", ModeDepGraph {
			.getGraph = getRawExplorerList,
			.handleItemSelected = selectRawItem,
			.showSceneIds = false,
			.onObjectSelected = onObjDoNothing,
	}},
};


void refreshDepGraph(EditorScenegraph& scenegraph){
	modlog("editor", "refresh graph type = " + scenegraph.depgraphType);
	ModeDepGraph& depGraph = modeToGetDepGraph.at(scenegraph.depgraphType);
	scenegraph.depGraph = depGraph.getGraph(scenegraph);
}


float calcSpacing(int fontsize){
	return 4.f * fontsize / 1000.f; 
}
float calcX(int depth, int fontsize){
	float spacingPerLetter = calcSpacing(fontsize);
	return -1.f + (0.5f * spacingPerLetter) + (depth * spacingPerLetter);
}

float rawCalcY(int depth, int fontsize){
	float spacingPerLetter = calcSpacing(fontsize);
	return 1.f - (0.5f * spacingPerLetter) - (depth * spacingPerLetter);

}

float calcY(int depth, int fontsize, float offset){
	return rawCalcY(depth, fontsize) - offset;
}
void doDrawList(EditorScenegraph& scenegraph, SceneDrawList& drawList, bool showSceneIds){
	modlog("editor", "scenegraph - offset: " + std::to_string(scenegraph.offset));
	for (auto &drawElement : drawList.elements){
		if (drawElement.hasChildren){
			mainApi -> drawText(
				drawElement.expanded ? "X" : "O",
				calcX(drawElement.depth , scenegraph.fontSize),
				calcY(drawElement.height, scenegraph.fontSize, scenegraph.offset), 
				scenegraph.fontSize,
				false,
				drawElement.isSelected ? glm::vec4(0.7f, 0.7f, 0.7f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f), 
				scenegraph.textureId, 
				true, 
				"./res/fonts/Walby-Regular.ttf", 
				drawElement.mappingNumber
			);
		}
		mainApi -> drawText(
			showSceneIds ? (drawElement.elementName + " (" + std::to_string(drawElement.sceneId) + ")") : drawElement.elementName, 
			calcX(drawElement.depth + 1, scenegraph.fontSize), 
			calcY(drawElement.height, scenegraph.fontSize, scenegraph.offset), 
			scenegraph.fontSize, 
			false, 
			drawElement.isSelected ? glm::vec4(0.7f, 0.7f, 0.7f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f), 
			scenegraph.textureId, 
			true, 
			"./res/fonts/Walby-Regular.ttf", 
			drawElement.mappingNumber  + scenegraph.mappingOffset
		);
	}
}

bool isAnyDependency(std::map<objid, std::set<objid>>& dependencyMap, objid value){
	for (auto &[_, dep] : dependencyMap){
		if (dep.count(value) > 0){
			return true;
		}
	}
	return false;
}

struct DepInfo {
	std::string name;
	objid sceneId;
	objid id;
	bool expanded;
};


void navigateDeps(std::vector<SceneDrawElement>& _elements, std::map<objid, std::set<objid>> _dependencyMap, objid currElement, int& index, int childIndex, std::function<DepInfo(objid)> infoForId){
	bool hasChildren = _dependencyMap.find(currElement) != _dependencyMap.end();
	auto idInfo = infoForId(currElement);
	_elements.push_back(
		SceneDrawElement {
			.elementName = idInfo.name,
			.sceneId = idInfo.sceneId,
			.depth = childIndex,
			.height = index,
			.expanded = idInfo.expanded,
			.isSelected = false,
			.mappingNumber = index,
			.hasChildren = hasChildren,
			.id = currElement,
		}
	);
	index++;
	if (!hasChildren || !idInfo.expanded){
		return;
	}
	auto dependencies = _dependencyMap.at(currElement);
	for (auto &dependency : dependencies){
		navigateDeps(_elements, _dependencyMap, dependency, index, childIndex + 1, infoForId);
	}
}

void addElementsToList(std::vector<SceneDrawElement>& elements, std::map<objid, std::set<objid>> _dependencyMap, int index, std::function<DepInfo(objid)> infoForId){
	std::vector<objid> rootElements;
	for (auto &[item, deps] : _dependencyMap){
		if (!isAnyDependency(_dependencyMap, item)){
			rootElements.push_back(item);
		}
	}
	for (auto &rootElement : rootElements){
		navigateDeps(elements, _dependencyMap, rootElement, index, 0, infoForId);
	}
}

bool scenegraphExpanded(EditorScenegraph& scenegraph, objid id){
	if (scenegraph.idToExpandState.find(id) == scenegraph.idToExpandState.end()){
		return true;
	}
	return scenegraph.idToExpandState.at(id);
}

SceneDrawList drawListFromDepGraph(EditorScenegraph& scenegraph){
	std::map<objid, std::set<objid>> dependencyMap = {};
	for (auto &sceneDep : scenegraph.depGraph.value()){
		if (dependencyMap.find(sceneDep.parentId) == dependencyMap.end()){
			dependencyMap[sceneDep.parentId] = {};
		}
		dependencyMap.at(sceneDep.parentId).insert(sceneDep.childId);
	}
	std::vector<SceneDrawElement> elements;
	addElementsToList(
		elements, 
		dependencyMap, 
		0, 
		[&scenegraph](objid id) -> DepInfo { 
			for (auto &scenedep : scenegraph.depGraph.value()){
				if (scenedep.parentId == id){
					return DepInfo { .name = scenedep.element, .sceneId = scenedep.elementScene, .id = id, .expanded = scenegraphExpanded(scenegraph, id) };
				}else if (scenedep.childId == id){
					return DepInfo { .name = scenedep.depends, .sceneId = scenedep.dependsScene, .id = id, .expanded = scenegraphExpanded(scenegraph, id)};
				}
			}
			modassert(false, "editor - scenegraph - could not resolve id = " + std::to_string(id));
			return DepInfo {}; 
		}
	);
	SceneDrawList drawList {
		.elements = elements,
	};
	return drawList;
}




bool drawTitle = false;
void onGraphChange(EditorScenegraph& scenegraph){
	refreshDepGraph(scenegraph);

	mainApi -> clearTexture(scenegraph.textureId.value(), std::nullopt, std::nullopt, std::nullopt);
	if (drawTitle){
		mainApi -> drawText("Scenegraph", 20, 30, 20, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), scenegraph.textureId, false, std::nullopt, std::nullopt);
  	mainApi -> drawLine(glm::vec3(-1.f, 0.9f, 0), glm::vec3(1.f, 0.9f, 0.f), false, scenegraph.mainObjId, glm::vec4(0, 0, 1.f, 1.f), scenegraph.textureId, std::nullopt);
  	mainApi -> drawLine(glm::vec3(-1.f, 0.9f, 0), glm::vec3(1.f, 0.9f, 0), false, scenegraph.mainObjId, glm::vec4(0, 0, 1.f, 1.f), scenegraph.textureId, std::nullopt);
  	mainApi -> drawLine(glm::vec3(-1.f, 1.f, 0), glm::vec3(1.f, 1.f, 0), false, scenegraph.mainObjId, glm::vec4(0, 0, 1.f, 1.f), scenegraph.textureId, std::nullopt);
	}

	auto drawList = drawListFromDepGraph(scenegraph);
	scenegraph.selectedIndex = std::min(static_cast<int>(drawList.elements.size() - 1),  std::max(0, scenegraph.selectedIndex));
	drawList.elements.at(scenegraph.selectedIndex).isSelected = true;
	modlog("editor", "draw list size: " + std::to_string(drawList.elements.size()) + ", selected index = " + std::to_string(scenegraph.selectedIndex));
	doDrawList(scenegraph, drawList, modeToGetDepGraph.at(scenegraph.depgraphType).showSceneIds);
}

void toggleExpanded(EditorScenegraph& scenegraph){
	modlog("editor", "scenegraph toggle index: " + std::to_string(scenegraph.selectedIndex));
	auto selectedIndexId = getIdForSelectedIndex(scenegraph);
	if (scenegraph.idToExpandState.find(selectedIndexId) == scenegraph.idToExpandState.end()){
		scenegraph.idToExpandState[selectedIndexId] = true;
	}
	scenegraph.idToExpandState.at(selectedIndexId) = !scenegraph.idToExpandState.at(selectedIndexId);
}


CScriptBinding cscriptScenegraphBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/scenegraph", api);

  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    EditorScenegraph* scenegraph = new EditorScenegraph;
    scenegraph -> mainObjId = id;
    scenegraph -> didScroll = false;
    scenegraph -> offset = 0.f;
    scenegraph -> maxOffset = 0.f;
    scenegraph -> minOffset = 0.f;
    scenegraph -> selectedIndex = 0;
    scenegraph -> depGraph = std::nullopt;
    scenegraph -> textureId = std::nullopt;
    scenegraph -> fontSize = 22;
    scenegraph -> mappingOffset = 5000;
    scenegraph -> idToExpandState = {};

		auto attr = mainApi -> getGameObjectAttr(scenegraph -> mainObjId);
		auto type = getStrAttr(attr, "depgraph").value();
		scenegraph -> depgraphType = type;
		auto textureName = getStrAttr(attr, "gentexture").value();
		scenegraph -> textureId = mainApi -> createTexture(textureName, 1000, 1000, id);
		onGraphChange(*scenegraph);
    return scenegraph;
	};

  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    EditorScenegraph* scenegraph = static_cast<EditorScenegraph*>(data);
    delete scenegraph;
  };


  binding.onFrame = [](int32_t id, void* data) -> void {
  	EditorScenegraph* scenegraph = static_cast<EditorScenegraph*>(data);
  	if (scenegraph -> didScroll){
  		scenegraph -> didScroll = false;
  		onGraphChange(*scenegraph);

  	}
  };

  binding.onObjectSelected = [](int32_t id, void* data, int32_t index, glm::vec3 color) -> void {
  	EditorScenegraph* scenegraph = static_cast<EditorScenegraph*>(data);
		modeToGetDepGraph.at(scenegraph -> depgraphType).onObjectSelected(*scenegraph, index);
	};

	binding.onMapping = [](int32_t id, void* data, int32_t index) -> void {
/*(define (onMapping index)
	(define selectedIndexForMapping (baseNumberToSelectedIndex index))
	(define isManagedIndex (isManagedByThisScript index))
	(format #t "mapping: ~a, baseNumber: ~a, mappingoffset = ~a, ismanagedIndex = ~a, name ~a\n" index baseNumber mappingOffset isManagedIndex (gameobj-name mainobj))
	(format #t "attr: ~a\n" (gameobj-attr mainobj))

	(if (and selectedIndexForMapping isManagedIndex)
		(let (
				(isToggle (not (cadr selectedIndexForMapping)))
				(mappedIndex (car selectedIndexForMapping))
			)
			(if isToggle
				(begin
					;(refreshDepGraph)
	  			(setSelectedIndex mappedIndex)
	  			(refreshGraphData)   
	   			(toggleExpanded selectedName)
	  			(onGraphChange)
				)
				(begin
					;(refreshDepGraph)
					(format #t "mapping: selected index for mapping!\n")
	  			(setSelectedIndex mappedIndex)
					(format #t "mapping: did set selected index for mapping!\n")
	  			(refreshGraphData)   
	  			(format #t "mapping: refreshed graph data!\n")
	  			(handleItemSelected selectedElement #f)
	  			(format #t "mapping: handled selected item!\n")
	  			(onGraphChange)
				)
			)
		)
	)
	;(format #t "selected_index = ~a, selected_name = ~a, selected_element = ~a\n" selectedIndex selectedName selectedElement)
)
*/
	};

	binding.onScrollCallback = [](objid scriptId, void* data, double amount) -> void{
		EditorScenegraph* scenegraph = static_cast<EditorScenegraph*>(data);
		scenegraph -> didScroll = true;
		scenegraph -> offset = std::min(scenegraph -> minOffset, static_cast<float>(scenegraph -> offset + (amount * 0.04f)));
	};

  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
  	EditorScenegraph* scenegraph = static_cast<EditorScenegraph*>(data);
  	if (action == 1){
  		if (key == 264){  // up
  			scenegraph -> selectedIndex = scenegraph -> selectedIndex + 1;
  			onGraphChange(*scenegraph);
  		}else if (key == 265){  // down
  			scenegraph -> selectedIndex = scenegraph -> selectedIndex - 1;
  			onGraphChange(*scenegraph);
  		}else if (key == 257){  // enter
  			toggleExpanded(*scenegraph);
  			onGraphChange(*scenegraph);
  		}
  		/*
     	(if (equal? key 344) (handleItemSelected selectedElement #f))  ; shift
     	(if (equal? key 345) (handleItemSelected selectedElement #t))  ; ctrl
		)*/
  	}else if (key == 344){
  		modeToGetDepGraph.at(scenegraph -> depgraphType).handleItemSelected(*scenegraph, false);
  	}else if (key == 345){
  		modeToGetDepGraph.at(scenegraph -> depgraphType).handleItemSelected(*scenegraph, true);
  	}else if (key == '='){  // =
  		modlog("editor", "scenegraph - increase font size");
			scenegraph -> fontSize += 1;
			onGraphChange(*scenegraph);
  	}else if (key == '-'){  // -
  		modlog("editor", "scenegraph - decrease font size");
			scenegraph -> fontSize -= 1;
			onGraphChange(*scenegraph);
  	}


  };


  return binding;
}
