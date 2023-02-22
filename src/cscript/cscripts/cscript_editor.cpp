#include "./cscript_editor.h"

CustomApiBindings* mainApi;

struct EditorData {
  std::vector<objid> panels;

  std::optional<std::string> currOption;
  std::optional<objid> popoverSceneId;
  std::map<int, objid> snapPosToSceneId;
};

std::string popItemPrefix = ")text_";
const int popItemPrefixLength = popItemPrefix.size();
bool isPopoverElement(std::string name){
  return name.substr(0, popItemPrefixLength) == name;
}


void maybeUnloadPopover(EditorData& editorData){
  if (editorData.popoverSceneId.has_value()){
    mainApi -> unloadScene(editorData.popoverSceneId.value());
    editorData.currOption = std::nullopt;
    editorData.popoverSceneId = std::nullopt;
  }
}


std::vector<std::vector<std::string>> popoverOptions(std::string elementName, std::vector<std::string> menuOptions){
  return {};
}
/*
(define (popover-options elementName menuOptions)
  (define nextName (genNextName popItemPrefix))
  (define (create-value text) (textvalue (nextName) text text))

  (define val (map create-value menuOptions))
  (define itemnames (map car val))
  (define elements (map cadr val))
  (define joinedNames (string-join itemnames ","))
  (define joinedElements (apply append elements))
  (define baselist (list 
    (list "(dialog" "elements" joinedNames)
    (list "(dialog" "anchor" elementName)
    (list "(dialog" "anchor-offset" "-0.04 -0.067 0")
    (list "(dialog" "anchor-dir-horizontal" "right")
    (list "(dialog" "anchor-dir-vertical" "down")

  ))
  (format #t "joinedNames: ~a\n" joinedNames)
  (format #t "joinedElements name: ~a\n" joinedElements)
  (format #t "element name is: ~a\n" elementName)
  (append baselist  joinedElements)
)
*/

void changePopover(EditorData& editorData, std::string elementName, std::string uiOption){
  modlog("editor", "load popover: " + uiOption);
  bool isAlreadyLoaded = editorData.currOption.has_value() && editorData.currOption.value() == uiOption;
  if (isAlreadyLoaded){
    modlog("editor", "ui is already loaded: " + uiOption);
    return;
  }
  
  maybeUnloadPopover(editorData);
  
  // (popover-options elementName (cadr (assoc uiOption uilist)))
  std::optional<std::vector<std::string>> tags = std::optional<std::vector<std::string>>({ "editor" });
  
  std::vector<std::string> menuOptions; // need to get this from something
  // (set! sceneId (load-scene "./res/scenes/editor/popover.rawscene" (popover-options elementName (cadr (assoc uiOption uilist))) (list "editor")))
  auto sceneId = mainApi -> loadScene("./res/scenes/editor/popover.rawscene", popoverOptions(elementName, menuOptions), std::nullopt, tags);
  editorData.popoverSceneId = sceneId;
  editorData.currOption = uiOption;

  auto dialogId = mainApi -> getGameObjectByName("(dialog", sceneId, true);
  mainApi -> enforceLayout(dialogId.value()); // wait....why need two passed?
  mainApi -> enforceLayout(dialogId.value());
}


std::string fullElementName(std::string localname, objid mainSceneId){
  auto fullElementName = "." + std::to_string(mainSceneId) + "/" + localname;
  return fullElementName;
} 

void handleDialogClick(std::string name, GameobjAttributes& attributes){
/*
(define (handle-dialog-click name attr)
  (define optionname (resolve-option-name name attr))
  (if (and activeDialogName (not (equal? optionname "")))
    ((get-fn-for-dialog-option activeDialogName optionname))
  )
)
*/
}

void maybeUnloadSidepanelByScene(int sceneId){
/*
(define (maybe-unload-sidepanel-by-scene sceneIndex)
  (define snapIndex (list-index snappingPositionToSceneId sceneIndex))
  (format #t "dock: remove: ~a ~a\n" snapIndex sceneIndex)
  (if snapIndex 
    (maybe-unload-sidepanel-snap snapIndex)
    (unload-scene sceneIndex)
  )
) 
*/
  mainApi -> unloadScene(sceneId);
}

void maybeUnloadSidepanelAll(EditorData& editorData){
  for (auto [_, sceneId] : editorData.snapPosToSceneId){
    maybeUnloadSidepanelByScene(sceneId);
  }
}

void maybeUnloadSidepanelSnap(EditorData& editorData, int panelIndex){
  if (editorData.snapPosToSceneId.find(panelIndex) != editorData.snapPosToSceneId.end()){
    auto sceneId = editorData.snapPosToSceneId.at(panelIndex);
    mainApi -> unloadScene(sceneId);
    editorData.snapPosToSceneId.erase(panelIndex);
  }
}


std::optional<int> getSnapId(EditorData& editorData, int index){
  if (editorData.snapPosToSceneId.find(index) != editorData.snapPosToSceneId.end()){
    return editorData.snapPosToSceneId.at(index);
  }
  return std::nullopt;
}



void popoverAction(std::string action){
  /*(define (popoverAction action)
  (define mappedAction (assoc action nameAction))
  (define actionName (car mappedAction))
  (if mappedAction 
    (let ((openedDialog ((cadr mappedAction))))
      (if (equal? openedDialog #t)
        (set! activeDialogName actionName)
      )
    )
  )
)
*/
}




void saveAllPanels(){
  std::cout << "should save all panels" << std::endl;

/*
/*(define (sceneFileForSceneId sceneId) 
  (define sceneIds (list-scenefiles sceneId))
  (if (> (length sceneIds) 0) (car sceneIds) #f)
)

(define (save-all-panels layoutname)
  (define layoutInfo 
    (map 
      (lambda(sceneId) 
        (let ((scenefilename (sceneFileForSceneId sceneId)))
          (if scenefilename
            (list 
              scenefilename
              (gameobj-pos (lsobj-name "(test_panel" sceneId))
            )
            #f
          )
        )
      ) 
      panels
    )
  )*/
}


bool hasLayoutTable(){
  auto query = mainApi -> compileSqlQuery("show tables", {});
  bool validSql = false;
  auto result = mainApi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");

  bool tableExists = false;
  for (auto row : result){
    if (row.at(0) == "layout"){
      return true;
    }
  }
  return false;
}

struct PanelAndPosition {
  std::string panel;
  glm::vec3 position;
};


objid loadSidePanel(std::string scene, std::optional<glm::vec3> pos, bool moveable, bool restrictX, bool snapX){
  std::vector<std::vector<std::string>> additionalTokens = {
    { ")window_x", "script", "./res/scenes/editor/dock/details.scm" },
    // ;(list "(test_panel" "anchor" anchorElementName)
    { "(test_panel", "position", serializeVec(pos.has_value() ? pos.value() : glm::vec3(-0.78f, -0.097f, -1.f)) }, 
  };
  if (moveable){
    additionalTokens.push_back({ "(test_panel", "script", "./res/scenes/editor/dialogmove.scm" });
  }
  if (restrictX){
    additionalTokens.push_back({ "(test_panel", "dialogmove-restrictx", "true" });
  }
  if (snapX){
    additionalTokens.push_back({ "(test_panel", "editor-shouldsnap", "true" });
  }
  std::optional<std::vector<std::string>> tags = std::optional<std::vector<std::string>>({ "editor" });
  return mainApi -> loadScene(scene, additionalTokens, std::nullopt, tags);
}

void updateSnapPos(EditorData& editorData, int snappingIndex, int sceneId){
  editorData.snapPosToSceneId[snappingIndex] = sceneId;
}

void changeSidepanel(EditorData& editorData, int snappingIndex, std::string scene, std::string anchorElementName){
  modlog("editor", "change sidepanel, load scene: " + scene + ", anchor: " + anchorElementName);
  maybeUnloadSidepanelSnap(editorData, snappingIndex);
  auto snapId = getSnapId(editorData, snappingIndex);
  if (!snapId.has_value()){
    auto sidePanelSceneId = loadSidePanel(scene, std::nullopt, true, true, true);
    auto testPanelId = mainApi -> getGameObjectByName("(test_panel", sidePanelSceneId, false);
    updateSnapPos(editorData, snappingIndex, sidePanelSceneId);
    mainApi -> enforceLayout(testPanelId.value());
  }
}

std::vector<objid> loadAllPanels(std::vector<PanelAndPosition> panels){
  std::cout << "wanting to load panels: " << panels.size() << std::endl;
  auto args =  mainApi -> getArgs();
  auto panelsMoveable = args.find("gridmove") != args.end();

  std::vector<objid> ids;
  for (auto panel : panels){
    ids.push_back(loadSidePanel(panel.panel, panel.position, panelsMoveable, false, false));
  }
  return ids;
}

std::vector<PanelAndPosition> tableLayout(std::string layoutname){
  auto query = mainApi -> compileSqlQuery(
    "select panel, position from layout where name = ?",  // maybe just hardcode this, since built in data dir which isn't really supported, for eg play pause button
    { layoutname }
  );
  bool validSql = false;
  auto result = mainApi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");

  std::vector<PanelAndPosition> panels;
  for (auto row : result){
    panels.push_back(PanelAndPosition {
      .panel = row.at(0),
      .position = parseVec(row.at(1)),
    });
  }
  return panels;
}




void loadPanelsFromDb(EditorData& editorData, std::string layoutname){
  std::cout << "should load all panels" << std::endl;
  auto layoutTableExists = hasLayoutTable();
  modlog("editor", "layout exists: " + print(layoutTableExists));
  if (layoutTableExists){
    //(for-each maybe-unload-sidepanel-by-scene panels) need to implement
    for (auto panelSceneId : editorData.panels){
      maybeUnloadSidepanelByScene(panelSceneId);
    }
    editorData.panels = loadAllPanels(tableLayout(layoutname));
  }
}

void onObjectSelected(int32_t id, void* data, int32_t index, glm::vec3 color){
  EditorData* editorData = static_cast<EditorData*>(data);

  auto objattrs =  mainApi -> getGameObjectAttr(index);
  auto popOptionPair = getStrAttr(objattrs, "popoption");
  auto popAction = getStrAttr(objattrs, "popaction");
  bool hasPopOption = popOptionPair.has_value();
  auto elementName = mainApi -> getGameObjNameForId(index).value();
  auto isPopover = isPopoverElement(elementName);
  auto dialogOption = getStrAttr(objattrs, "dialogoption");

  auto sceneId = mainApi -> listSceneId(id);
  if (hasPopOption){
    changePopover(*editorData, fullElementName(elementName, sceneId), popOptionPair.value());
  }else{
    maybeUnloadPopover(*editorData);
  }

  if (isPopover && popAction.has_value()){
    popoverAction(popAction.value());
  }

  if (! (hasPopOption || isPopover)){
    maybeUnloadPopover(*editorData);
  }

  handleDialogClick(elementName, objattrs);
  if (dialogOption.has_value() && dialogOption.value() != ""){
    if (dialogOption.value() == "HIDE"){
      maybeUnloadSidepanelAll(*editorData);
    }else{
      changeSidepanel(*editorData, 1, dialogOption.value(), fullElementName("(menubar", sceneId));
    }
  }
}


CScriptBinding cscriptEditorBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/editor", api);
  mainApi = &api;
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    EditorData* editorData = new EditorData;
    editorData -> panels = {};
    editorData -> currOption = std::nullopt;
    editorData -> popoverSceneId = std::nullopt;
    editorData -> snapPosToSceneId = {};

    auto args =  mainApi -> getArgs();
    auto layoutToUse = args.find("layout") == args.end() ? "none" : args.at("layout");
    loadPanelsFromDb(*editorData, layoutToUse);

    auto row2Id = mainApi -> getGameObjectByName("(row2", sceneId, false);
    auto row3Id = mainApi -> getGameObjectByName("(row3", sceneId, false);
    auto menubarId = mainApi -> getGameObjectByName("(menubar", sceneId, false);
    mainApi -> enforceLayout(row2Id.value());
    mainApi -> enforceLayout(row3Id.value());
    mainApi -> enforceLayout(menubarId.value());
    return editorData;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    EditorData* editorData = static_cast<EditorData*>(data);
    delete editorData;
  };


  binding.onKeyCharCallback = [](int32_t id, void* data, unsigned int codepoint) -> void {
    if (codepoint == 'o'){
      saveAllPanels();
    }else if (codepoint == 'p'){
      EditorData* editorData = static_cast<EditorData*>(data);
      loadPanelsFromDb(*editorData, "main");
    }
  };

  binding.onObjectSelected = onObjectSelected;

  return binding;
}
