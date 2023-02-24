#include "./cscript_editor.h"

CustomApiBindings* mainApi;

struct EditorData {
  std::vector<objid> panels;

  std::optional<std::string> currOption;
  std::optional<objid> popoverSceneId;
  std::map<int, objid> snapPosToSceneId;

  std::optional<std::string> layoutToUse;
};

std::map<std::string, std::vector<std::string>> uiList = {
  { "file", { "load", "quit" }},
  { "misc", { "fullscreen" }},
};



void maybeUnloadDialog(EditorData& editorData){
  /*
  (define (maybe-unload-dialog)
  (if dialogSceneId (unload-scene dialogSceneId))
  (set! dialogSceneId #f)
  (set! activeDialogName #f)
  )
  */
}

struct DialogOptions {
  std::string title;
  std::string message;
  std::map<std::string, std::function<void(EditorData&)>> optionToAction;
};

std::map<std::string, DialogOptions> dialogMap = {
  { "load", DialogOptions {
      .title = "Load Layout",
      .message = "layouts enable different ui workflows",
      .optionToAction = {
        { "CANCEL", [](EditorData& editorData) -> void { maybeUnloadDialog(editorData); }},
        { "LOAD", [](EditorData& editorData) -> void {}},
      },
    }
  },
  { "quit", DialogOptions {
      .title = "Confirm QUIT",
      .message = "are you sure you want to quit?",
      .optionToAction = {
        { "CANCEL", [](EditorData& editorData) -> void { maybeUnloadDialog(editorData); }},
        { "QUIT", [](EditorData& editorData) -> void { exit(0); }},
      },

    }
  },
};

std::map<std::string, std::function<void(EditorData&)>> nameAction = {
  { "load", [](EditorData& editorData) -> void { 
    /* (get-change-dialog "load")*/ } 
  },
  { "fullscreen", [](EditorData& editorData) -> void { /* 

    (lambda () 
//      (format #t "toggling fullscreen\n")
//      (set! fullscreen (not fullscreen))
//      (set-wstate (list
//        (list "rendering" "fullscreen" (if fullscreen "true" "false")) ; doesn't actually toggle since fullscreen state never updates to match internal with set-wstate
//      ))
//      #f
//    )
//  )

  */ } },

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


std::function<std::string(void)> genNextName(std::string prefix){
  int index = -1;
  return [&index, &prefix]() -> std::string {
    index++;
    return prefix + std::to_string(index);
  };
}

std::vector<std::vector<std::string>> textValue(std::string& name, std::string& content, std::string& action){
  return {
    { name, "layer", "basicui" },
    { name, "scale", "0.004 0.01 0.004" },
    { name, "value", content },
    { name, "popaction", action },
  };
    
}

std::vector<std::vector<std::string>> popoverOptions(std::string elementName, std::vector<std::string> menuOptions){
  auto nextName = genNextName(popItemPrefix);
  std::vector<std::vector<std::string>> items = {};
  std::vector<std::string> allItemNames;
  for (auto &menuOption : menuOptions){
    auto menuItemName = nextName();
    allItemNames.push_back(menuItemName);

    items.push_back({ menuItemName, "layer", "basicui" });
    items.push_back({ menuItemName, "scale", "0.004 0.01 0.004" });
    items.push_back({ menuItemName, "value", menuOption });
    items.push_back({ menuItemName, "popaction", menuOption });
  }

  items.push_back({ "(dialog", "anchor", elementName });
  items.push_back({ "(dialog", "anchor-offset", "-0.04 -0.067 0" });
  items.push_back({ "(dialog", "anchor-dir-horizontal", "right" });
  items.push_back({ "(dialog", "anchor-dir-vertical", "down" });
  items.push_back({ "(dialog", "elements", join(allItemNames, ',') });

  return items;
}

void changePopover(EditorData& editorData, std::string elementName, std::string uiOption){
  modlog("editor", "load popover: " + uiOption);
  bool isAlreadyLoaded = editorData.currOption.has_value() && editorData.currOption.value() == uiOption;
  if (isAlreadyLoaded){
    modlog("editor", "ui is already loaded: " + uiOption);
    return;
  }
  
  maybeUnloadPopover(editorData);
  
  auto sceneId = mainApi -> loadScene(
    "./res/scenes/editor/popover.rawscene", 
    popoverOptions(elementName, uiList.at(uiOption)), 
    std::nullopt, 
    std::optional<std::vector<std::string>>({ "editor" })
  );
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



void popoverAction(EditorData& editorData, std::string action){
  auto actionFn = nameAction.at(action);
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


void removeOldLayout(std::string& layoutname){
  auto query = mainApi -> compileSqlQuery("delete from layout where name = ?", { layoutname });
  bool validSql = false;
  auto result = mainApi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
}

void saveNewLayout(std::string& layoutname, std::string& panel, glm::vec3 position){
  auto query = mainApi -> compileSqlQuery( "insert into layout (name, panel, position) values (?, ?, ?)", { layoutname, panel, serializeVec(position) });
  bool validSql = false;
  auto result = mainApi -> executeSqlQuery(query, &validSql);
  modassert(validSql, "error executing sql query");
}

void saveAllPanels(EditorData& editorData, std::string layoutname){
  modlog("editor save panels", "saving layout: " + layoutname);
  removeOldLayout(layoutname);
  for (auto sceneIdForPanel : editorData.panels){
    auto sceneFile = mainApi -> listSceneFiles(sceneIdForPanel).at(0);
    auto testPanelId = mainApi -> getGameObjectByName("(test_panel", sceneIdForPanel, false).value();
    auto position = mainApi -> getGameObjectPos(testPanelId, false);
    saveNewLayout(layoutname, sceneFile, position);
  }
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
  auto popOption = getStrAttr(objattrs, "popoption");
  auto popAction = getStrAttr(objattrs, "popaction");
  auto elementName = mainApi -> getGameObjNameForId(index).value();
  auto isPopover = isPopoverElement(elementName);
  auto dialogOption = getStrAttr(objattrs, "dialogoption");

  auto sceneId = mainApi -> listSceneId(id);
  if (popOption.has_value()){
    changePopover(*editorData, fullElementName(elementName, sceneId), popOption.value());
  }else{
    maybeUnloadPopover(*editorData);
  }

  if (isPopover && popAction.has_value()){
    popoverAction(*editorData, popAction.value());
  }

  if (! (popOption.has_value() || isPopover)){
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

void changeCursor(bool hoverOn){
  std::vector<ObjectValue> values = {};
  if (hoverOn){
    values.push_back(
      ObjectValue {
        .object = "mouse",
        .attribute = "crosshair",
        .value = "./res/textures/crosshairs/crosshair029.png",
      }
    );
  }else{
    values.push_back(
      ObjectValue {
        .object = "mouse",
        .attribute = "crosshair",
        .value = "",
      }
    );   
  }
  mainApi -> setWorldState(values);
}
bool isButton(int32_t id){
  auto name = mainApi -> getGameObjNameForId(id).value();
  return name.size() >= 1 && name.at(0) == '*';
}

void maybeHandleSidePanelDrop(objid id){
/*
  (define gameobj (gameobj-by-id id))
  (define pos (gameobj-pos gameobj))
  (define snapValue (assoc "editor-shouldsnap" (gameobj-attr gameobj)))
  (define shouldSnap (if snapValue (equal? "true" (cadr snapValue)) #f))
  (define sceneId (list-sceneid id))
  (if shouldSnap 
    (let ((snappingPair (get-snapping-pair-by-loc gameobj)))
      ;(format #t "snapping pair is: ~a\n" snappingPair)
      (if (and snappingPair (not (snap-slot-occupied (car snappingPair))))
        (update-snap-pos (applySnapping gameobj snappingPair) sceneId)
        (update-snap-pos (applySnapping gameobj (get-current-snapping-pair sceneId)) sceneId)
      )
    )
  )
*/
}

void tintThemeColors(bool isPlay, objid sceneId){
  auto themeObjs = mainApi -> getObjectsByAttr("theme-play-tint", std::nullopt, sceneId);
  for (auto themeObjId : themeObjs){
    auto currAttr = mainApi -> getGameObjectAttr(themeObjId);
    auto themeTint = getVec4Attr(currAttr, "theme-play-tint");
    auto restoreTint = getVec4Attr(currAttr, "theme-restore-tint");
    GameobjAttributes attr {
      .stringAttributes = {},
      .numAttributes = {},
      .vecAttr = { .vec3 = {}, .vec4 = { {"tint", isPlay ? themeTint.value() : restoreTint.value() }} },
    };
    mainApi -> setGameObjectAttr(themeObjId, attr);
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
    editorData -> layoutToUse = args.find("layout") == args.end() ? std::nullopt: std::optional<std::string>(args.at("layout"));

    if (editorData -> layoutToUse.has_value()){
      loadPanelsFromDb(*editorData, editorData -> layoutToUse.value());
    }

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
      EditorData* editorData = static_cast<EditorData*>(data);
      if (editorData -> layoutToUse.has_value()){
        saveAllPanels(*editorData, editorData -> layoutToUse.value());
      }
    }else if (codepoint == 'p'){
      EditorData* editorData = static_cast<EditorData*>(data);
      loadPanelsFromDb(*editorData, "testload");
    }
  };

  binding.onObjectSelected = onObjectSelected;
  binding.onObjectUnselected = [](objid scriptId, void* data) -> void {
    EditorData* editorData = static_cast<EditorData*>(data);
    maybeUnloadPopover(*editorData);
  };


  binding.onObjectHover = [](objid scriptId, void* data, int32_t index, bool hoverOn) -> void {
    if (hoverOn){
      if (isButton(index)){
        changeCursor(true);
      }
    }else{
      changeCursor(false);
    }
  };

  binding.onMessage = [](objid scriptId, void* data, std::string& topic, AttributeValue& value) -> void {
    EditorData* editorData = static_cast<EditorData*>(data);
    if (topic == "dialogmove-drag-stop"){
      auto idStr = std::get_if<std::string>(&value);
      maybeHandleSidePanelDrop(std::atoi(idStr -> c_str()));
    }else if (topic == "dock-self-remove"){
      modlog("editor", "should unload the dock because x was clicked");
      auto idStr = std::get_if<std::string>(&value);
      maybeUnloadSidepanelByScene(mainApi -> listSceneId(std::atoi(idStr -> c_str())));
    }else if (topic == "play-mode"){
      auto valueStr = std::get_if<std::string>(&value);
      tintThemeColors(*valueStr == "true", mainApi -> listSceneId(scriptId));
    }
  };


  return binding;
}
