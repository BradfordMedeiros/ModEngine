#include "./scenegraph.h"

extern CustomApiBindings* mainApi;

struct SceneDependency {
	std::string element;
	std::string depends;
	int elementScene;
	int dependsScene;
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
};

std::vector<SceneDependency> getNoData(EditorScenegraph& scenegraph){
	return { 
		SceneDependency { .element = "data", .depends = "none available", .elementScene = 0, .dependsScene = 0 } 
	};
}
std::vector<SceneDependency> getMockScenegraph(EditorScenegraph& scenegraph){
	return {
		SceneDependency { .element = "root", .depends = "mainfolder", .elementScene = 0, .dependsScene = 0 },
		SceneDependency { .element = "root2", .depends = "mainfolder2", .elementScene = 0, .dependsScene = 1 },
		SceneDependency { .element = "mainfolder", .depends = "folder1", .elementScene = 0, .dependsScene = 0 },
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
			if (scenePair.vec.x == sceneId || scenePair.vec.y == sceneId){
				sceneInList = true;
				break;
			}
		}
		if (!sceneInList){
		  filteredScenes.push_back({
		  	.element = scenePair.key,
		  	.depends = scenePair.value,
		  	.elementScene = scenePair.vec.x,
		  	.dependsScene = scenePair.vec.y,
		  });
		}
	}
	return filteredScenes;
}
std::vector<SceneDependency> getMockModelList(EditorScenegraph& scenegraph){
	return {
		SceneDependency { .element = "AllMeshes", .depends = "mesh1", .elementScene = 0, .dependsScene = 0 },
		SceneDependency { .element = "AllMeshes", .depends = "mesh2", .elementScene = 0, .dependsScene = 0 },
	};
}
std::vector<SceneDependency> getModelList(EditorScenegraph& scenegraph){
	return {};
	//(define (getModelList) (map (lambda(model) (makeIntoGraph "models" model)) (ls-res "models")))
}
std::vector<SceneDependency> getMockTextureList(EditorScenegraph& scenegraph){
	return { 
		SceneDependency { .element = "Textures", .depends = "texture-mock", .elementScene = 0, .dependsScene = 0 } 
	};
}
std::vector<SceneDependency> getTextureList(EditorScenegraph& scenegraph){
	return {};
	// (define (getTextureList) (map (lambda(model) (makeIntoGraph "textures" model)) (ls-res "textures")))
}

std::vector<SceneDependency> getRawExplorerList(EditorScenegraph& scenegraph){
	auto attr = mainApi -> getGameObjectAttr(scenegraph.mainObjId);
	auto values = split(getStrAttr(attr, "values").value(), '|');
	auto title = getStrAttr(attr, "title").value();
	std::vector<SceneDependency> deps;
	for (auto &value : values){
		deps.push_back(SceneDependency {
			.element = title,
			.depends = value,
			.elementScene = 0,
			.dependsScene = 0,
		});
	}
	return deps;
}

// handleItemSelected
void doNothing(EditorScenegraph& scenegraph){
}
void selectScenegraphItem(EditorScenegraph& scenegraph){
/*(define (selectScenegraphItem x isAlt) 
  (define objname (car x))
  (define sceneId (cadr x))
  (define selectedObj (lsobj-name objname sceneId))
  (define objIndex (gameobj-id selectedObj))
  (define objpos (gameobj-pos-world selectedObj))
  (if isAlt
  	(mov-cam (car objpos) (cadr objpos) (caddr objpos) #f)
  	(set-wstate (list
  		(list "editor" "selected-index" (number->string objIndex))
  	))
  )
)*/
}
void selectModelItem(EditorScenegraph& scenegraph){
	/*
	(define (selectModelItem element isAlt) 
	(define modelpath (car element))
	(define objname (string-append (number->string (random 10000000)) "-fix-generated"))
	(mk-obj-attr
		objname
		(list
			(list "mesh" modelpath)
			(list (string-append objname "/Plane") "texture" "gentexture-scenegraph")
		)
	)
)
*/
}

void selectTextureItem(EditorScenegraph& scenegraph){
	/*
	(define (selectTextureItem element isAlt)
	(define texturepath (car element))
	(define gameobjs (map gameobj-by-id (selected)))
	(for-each 
		(lambda(gameobj)
			(gameobj-setattr! gameobj (list
				(list "texture" texturepath)
			))
		)
		gameobjs
	)
	#f
)
*/
}
void selectRawItem(EditorScenegraph& scenegraph){
	/*
		(define attr (gameobj-attr mainobj))
	(define topic  (cadr (assoc "topic" attr)))
	(send topic (car element))
	*/
}

// onObjectSelected
void onObjDoNothing(EditorScenegraph& scenegraph, objid gameobjid){
}


void onObjectSelectedSelectItem(EditorScenegraph& scenegraph, objid gameobjid){
	/*
	(define (onObjectSelectedSelectItem gameobj color)
	(refreshDepGraph)
	(let (
			(index (selectedIndexForGameobj gameobj))
		)
		(if index
			(begin
		    (format #t "on object selected: ~a, ~a\n" gameobj color)
		    (setSelectedIndex index)
		    (refreshGraphData)   
		    (onGraphChange)
			)
		)
	)
)
*/
}

struct ModeDepGraph {
	std::function<std::vector<SceneDependency>(EditorScenegraph& scenegraph)> getGraph;
	std::function<void(EditorScenegraph& scenegraph)> handleItemSelected;
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
	// (list "raw" getRawExplorerList selectRawItem #f onObjDoNothing)
	{ "raw", ModeDepGraph {
			.getGraph = getRawExplorerList,
			.handleItemSelected = selectRawItem,
			.showSceneIds = false,
			.onObjectSelected = onObjDoNothing,
	}},
};


void refreshDepGraph(EditorScenegraph& scenegraph){
	ModeDepGraph& depGraph = modeToGetDepGraph.at(scenegraph.depgraphType);
	scenegraph.depGraph = depGraph.getGraph(scenegraph);
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
};
struct SceneDrawList {
	std::vector<SceneDrawElement> elements;
};


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

bool isAnyDependency(std::map<std::string, std::set<std::string>>& dependencyMap, std::string value){
	for (auto &[_, dep] : dependencyMap){
		if (dep.count(value) > 0){
			return true;
		}
	}
	return false;
}

void navigateDeps(std::vector<SceneDrawElement>& _elements, std::map<std::string, std::set<std::string>> _dependencyMap, std::string currElement, int& index, int childIndex){
	bool hasChildren = _dependencyMap.find(currElement) != _dependencyMap.end();
	_elements.push_back(
		SceneDrawElement {
			.elementName = currElement,
			.sceneId = 0,
			.depth = childIndex,
			.height = index,
			.expanded = false,
			.isSelected = false,
			.mappingNumber = 0,
			.hasChildren = hasChildren,
		}
	);
	index++;

	if (!hasChildren){
		return;
	}
	auto dependencies = _dependencyMap.at(currElement);
	for (auto &dependency : dependencies){
		navigateDeps(_elements, _dependencyMap, dependency, index, childIndex + 1);
	}
}

void topologicalSort(std::vector<SceneDrawElement>& elements, std::map<std::string, std::set<std::string>> _dependencyMap, int index){
	std::vector<std::string> rootElements;
	for (auto &[item, deps] : _dependencyMap){
		if (!isAnyDependency(_dependencyMap, item)){
			rootElements.push_back(item);
		}
	}
	for (auto &rootElement : rootElements){
		navigateDeps(elements, _dependencyMap, rootElement, index, 0);
	}
}


bool drawTitle = true;
void onGraphChange(EditorScenegraph& scenegraph){
	refreshDepGraph(scenegraph);

	mainApi -> clearTexture(scenegraph.textureId.value(), std::nullopt, std::nullopt, std::nullopt);
	if (drawTitle){
		mainApi -> drawText("Scenegraph", 20, 30, 20, false, glm::vec4(1.f, 1.f, 1.f, 0.8f), scenegraph.textureId, false, std::nullopt, std::nullopt);
  	mainApi -> drawLine(glm::vec3(-1.f, 0.9f, 0), glm::vec3(1.f, 0.9f, 0.f), false, scenegraph.mainObjId, glm::vec4(0, 0, 1.f, 1.f), scenegraph.textureId, std::nullopt);
  	mainApi -> drawLine(glm::vec3(-1.f, 0.9f, 0), glm::vec3(1.f, 0.9f, 0), false, scenegraph.mainObjId, glm::vec4(0, 0, 1.f, 1.f), scenegraph.textureId, std::nullopt);
  	mainApi -> drawLine(glm::vec3(-1.f, 1.f, 0), glm::vec3(1.f, 1.f, 0), false, scenegraph.mainObjId, glm::vec4(0, 0, 1.f, 1.f), scenegraph.textureId, std::nullopt);
	}

	std::vector<SceneDrawElement> elements;
	topologicalSort(
		elements, 
		{
			{ "one", { "two", "three" }}, 
			{ "four", { "five" }},
			{ "five", { "six", "seven"}}
		},
		0
	);

	SceneDrawList drawList {
		.elements = elements,
	};

	scenegraph.selectedIndex = std::min(static_cast<int>(drawList.elements.size() - 1),  std::max(0, scenegraph.selectedIndex));
	drawList.elements.at(scenegraph.selectedIndex).isSelected = true;
	doDrawList(scenegraph, drawList, true);
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
  			/*  			
  				(toggleExpanded selectedName)
     			(refreshDepGraph)   
   				(onGraphChange)*/
  		}
  		/*
     	(if (equal? key 257) 
     		(begin
   
     		)  ; enter
     	)
     	(if (equal? key 344) (handleItemSelected selectedElement #f))  ; shift
     	(if (equal? key 345) (handleItemSelected selectedElement #t))  ; ctrl
		)*/
  	}else if (key == '='){  // =
			scenegraph -> fontSize += 1;
			onGraphChange(*scenegraph);
  	}else if (key == '-'){  // -
			scenegraph -> fontSize -= 1;
			onGraphChange(*scenegraph);
  	}


  };


  return binding;
}

/*



(define (getDepGraph) #f)
(define showSceneIds #f)
(define handleItemSelected donothing)
(define onObjectSelected onObjDoNothing)

(define (setDepGraphType type)
	(define depGraphPair (assoc type modeToGetDepGraph))
	(if depGraphPair
		(begin
			(set! getDepGraph (cadr depGraphPair))
			(set! handleItemSelected (caddr depGraphPair))
			(set! onObjectSelected (list-ref depGraphPair 4))
			(set! showSceneIds (cadddr depGraphPair))
		)
	)
)

(define (setTypeFromAttr)
	(define depgraphAttr (assoc "depgraph" (gameobj-attr mainobj)))
	(if depgraphAttr 
		(setDepGraphType (cadr depgraphAttr))
		(setDepGraphType "nodata")
	)
)
(setTypeFromAttr)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


(define expandState (list))
(define (expandPath name sceneId) (string-append name ":" (number->string sceneId)))

(define (rmFromList listVals keyToRemove) 
	(filter 
		(lambda(val) 
			(format #t "key = ~a, val = ~a\n" keyToRemove val)
			(not (equal? (car val) keyToRemove))
		) 
		listVals
	)
)
(define (toggleExpanded name)
	(define element (assoc name expandState))
	(define isExpanded (if element (cadr element) #t))
	(define filteredList (rmFromList expandState name))
	(define newExpandState (cons  (list name (not isExpanded)) filteredList))
	(format #t "toggle expanded, selectname = ~a, element = ~a\n" name element)
	(set! expandState newExpandState)
)



(define (setMinOffset depth) 
	(define newMinOffset (rawCalcY depth))
	(if (< newMinOffset minOffset)
		(set! minOffset newMinOffset)
	)
)
(define (resetMinOffset) (set! minOffset 0))
(define minOffset (* -1 (rawCalcY 1)))

(define selectedIndex 1)
(define maxIndex #f)
(define selectedElement #f)
(define selectedName #f)

; should set selectedElement and selectedName here

(define (checkExpanded elementName sceneId)
	(define elementPath (expandPath elementName sceneId))
	(define value (assoc elementPath expandState))
	(if value (equal? #t (cadr value)) #t)
)



(define baseNumber (inexact->exact (cadr (assoc "basenumber" (gameobj-attr mainobj)))))  ; arbitrary number, only uses for mapping selection for now, which numbering is basically a manually process

(define baseNumberMapping (list))
(define (baseNumberToSelectedIndex mappingIndex)
	(define isPayloadMapping (>= mappingIndex (+ baseNumber mappingOffset)))
	(define index (if isPayloadMapping (- mappingIndex mappingOffset) mappingIndex))
	(define baseIndexPair (assoc index baseNumberMapping))
	(format #t "base mapping: ~a\n" baseNumberMapping)
	(format #t "base index pair: ~a\n" baseIndexPair)
	(if baseIndexPair
		(list (cadr baseIndexPair) isPayloadMapping)
		(begin
			(format #t "warning: no basemapping for: ~a\n" index)
			#f
		)
	)
)

(define (clearBaseNumberMapping) (set! baseNumberMapping (list)))
(define (setBaseNumberMapping basenumber index)
	(set! baseNumberMapping (cons (list basenumber index) baseNumberMapping))
)

;	(list "mainfolder" "folder1" (list 0 0))
(define (fullParentName element) (list (car element) (car (caddr element))))
(define (fullChildName element) (list (cadr element)  (cadr (caddr element))))



(define (updateDrawListData drawList index)
	(resetMinOffset)
	(clearBaseNumberMapping)
	(set! maxIndex index)
	(setMinOffset index)
	(for-each
		(lambda (drawElement)
			(let (
					(isSelected (list-ref drawElement 5))
					(target (list-ref drawElement 0))
					(sceneId (list-ref drawElement 1))
					(mappingNumber (list-ref drawElement 6))
					(height (list-ref drawElement 3))
				)
				(if isSelected (set! selectedName (expandPath target sceneId)))
				(if isSelected (set! selectedElement (list target sceneId)))
				(setBaseNumberMapping mappingNumber height)
			)
		)
		drawList
	)
)
// (define (makeIntoGraph header modelpath) (list header modelpath (list 0 0)))



(define (refreshGraphData)
	(let* (
			(drawListWithIndex (getDrawList))
			(drawList (car drawListWithIndex))
			(index (cadr drawListWithIndex))
			(numElements (length drawList))
		)
		(if (>= numElements 5000)  ; because of mapping offsets, can adjust those to adjust this
			(begin
				(format #t "only 5k elements supported\n")
				(exit 1)
			)
		)
		(updateDrawListData drawList index)
  	drawList
	)
)


(define (isMatchingDrawElement gameobj)
	(define objname (gameobj-name gameobj))
	(define sceneId (list-sceneid (gameobj-id gameobj)))
	(format #t "objname = ~a, sceneid = ~a\n" objname sceneId)
	(lambda(drawElement)
		(let (
				(drawElementName (car drawElement))
				(drawElementSceneId (cadr drawElement))
			)
			(and (equal? drawElementName objname) (equal? sceneId drawElementSceneId))
		)
	)
)
(define (selectedIndexForGameobj gameobj)
	(define drawElements (car  (getDrawList)))
	(define matchGameObj (isMatchingDrawElement gameobj))
	(define newSelectIndex (list-index matchGameObj drawElements))
	;(define value (list-ref drawElements newSelectIndex))
	;(format #t "draw list: ~a\n" drawElements)
	;(format #t "newSelectIndex: ~a\n" newSelectIndex)
	;(format #t "value: ~a\n" value)
	newSelectIndex
)


(define (isManagedByThisScript index)
	(and 
		(< index (+ baseNumber (* 2 mappingOffset)))
		(>= index baseNumber)
	)
)


*/