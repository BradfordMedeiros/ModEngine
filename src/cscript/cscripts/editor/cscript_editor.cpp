#include "./cscript_editor.h"

CustomApiBindings* mainApi;

struct EditorData {
  std::vector<objid> panels;

  std::optional<std::string> currOption;
  std::optional<objid> popoverSceneId;
  std::map<int, objid> snapPosToSceneId;

  std::optional<std::string> activeDialogName;
  std::optional<objid> dialogSceneId;

  std::optional<std::string> layoutToUse;
};

std::map<std::string, std::vector<std::string>> uiList = {
  { "file", { "quit", "load" }},
  { "misc", { "fullscreen" }},
};

void maybeUnloadDialog(EditorData& editorData){
  modlog("editor", "maybe unload dialog");
  if (editorData.dialogSceneId.has_value()){
    mainApi -> unloadScene(editorData.dialogSceneId.value());
    editorData.dialogSceneId = std::nullopt;
    editorData.activeDialogName = std::nullopt;
  }
}

std::function<std::string(void)> genNextName(std::string prefix){
  int index = -1;
  return [&index, &prefix]() -> std::string {
    index++;
    return prefix + std::to_string(index);
  };
}

void changeDialog(EditorData& editorData, std::string title, std::string subtitle, std::vector<std::string> options){
  modlog("editor", "changedialog " + title);
  if (!editorData.dialogSceneId.has_value()){
    std::vector<std::vector<std::string>> additionalTokens = {
        { ")text_2", "value", title },
        { ")text_main", "value", subtitle },
    };
    std::vector<std::string> dialogOptions;
    auto nextName = genNextName(")option_");
    for (int index = 0; index < options.size(); index++){
      auto option = options.at(index);
      auto elementName = nextName();

      additionalTokens.push_back({ elementName, "layer", "basicui" });
      additionalTokens.push_back({ elementName, "scale", "0.004 0.01 0.004" });
      additionalTokens.push_back({ elementName, "value", option });
      additionalTokens.push_back({ elementName, "option_index", option });

      dialogOptions.push_back(elementName);
    }
    additionalTokens.push_back({ "(options", "elements", join(dialogOptions, ',') });

    auto sceneId = mainApi -> loadScene("./res/scenes/editor/dialog.rawscene", additionalTokens, std::nullopt,  std::optional<std::vector<std::string>>({ "editor" }));
    editorData.dialogSceneId = sceneId;
  }
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

bool isFullScreen(){
  auto worldStates = mainApi -> getWorldState();
  for (auto &worldState : worldStates){
    if (worldState.object == "rendering" && worldState.attribute == "fullscreen"){
      auto strValue = std::get_if<std::string>(&worldState.value);
      return *strValue == "true";
    }
  }
  modassert(false, "could not determine if fullscreen");
  return false;
}

std::function<void(EditorData& editorData)> createPopoverAction(std::string dialogOption){
  DialogOptions& option = dialogMap.at(dialogOption);
  std::vector<std::string> options = {};
  for (auto &[key, _] : option.optionToAction){
    options.push_back(key);
  }
  return [&option, options, dialogOption](EditorData& editorData) -> void { 
     editorData.activeDialogName = dialogOption;
     changeDialog(editorData, option.title, option.message, options);
  }; 
}

std::map<std::string, std::function<void(EditorData& editorData)>> nameAction = {
  { "load", createPopoverAction("load") },
  { "quit", createPopoverAction("quit")},
  { "fullscreen", [](EditorData& editorData) -> void { 
    mainApi -> setWorldState({ 
      ObjectValue {
        .object = "rendering",
        .attribute = "fullscreen",
        .value = isFullScreen() ? "false" : "true",
      }
    });
  }}

};


std::string popItemPrefix = ")text_";
const int popItemPrefixLength = popItemPrefix.size();
bool isPopoverElement(std::string name){
  return name.substr(0, popItemPrefixLength) == popItemPrefix;
}


void maybeUnloadPopover(EditorData& editorData){
  if (editorData.popoverSceneId.has_value()){
    mainApi -> unloadScene(editorData.popoverSceneId.value());
    editorData.currOption = std::nullopt;
    editorData.popoverSceneId = std::nullopt;
  }
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


void handleDialogClick(EditorData& editorData, std::string& name, GameobjAttributes& attributes){
  modlog("editor", "handle dialog click");
  auto isOptionName = name.size() >= 8 && name.substr(0, 8) == ")option_";

  if (isOptionName && editorData.activeDialogName.has_value()){
    auto value = getStrAttr(attributes, "option_index").value();
    modlog("editor", "isOption = " + print(isOptionName) + ", activeDialogName: "  + (editorData.activeDialogName.has_value() ? editorData.activeDialogName.value() : "no dialog") + ", name = " + value);
    auto dialogOptionFn =  dialogMap.at(editorData.activeDialogName.value()).optionToAction.at(value);
    dialogOptionFn(editorData);
  }

  if (isOptionName && !editorData.activeDialogName.has_value()){
    std::cout << "is option but no active dialog" << std::endl;
  }
}

void maybeUnloadSidepanelByScene(EditorData& editorData, int sceneId){
  std::optional<int> snapPosToDelete = std::nullopt;
  for (auto &[snapPos, scene] : editorData.snapPosToSceneId){
    if (sceneId == scene){
      snapPosToDelete = snapPos;
      break;
    }
  }
  if (snapPosToDelete.has_value()){
    editorData.snapPosToSceneId.erase(snapPosToDelete.value());
  }
  mainApi -> unloadScene(sceneId);
}

void maybeUnloadSidepanelAll(EditorData& editorData){
  for (auto [_, sceneId] : editorData.snapPosToSceneId){
    maybeUnloadSidepanelByScene(editorData, sceneId);
  }
}

void maybeUnloadSidepanelSnap(EditorData& editorData, int snappingIndex){
  if (editorData.snapPosToSceneId.find(snappingIndex) != editorData.snapPosToSceneId.end()){
    auto sceneId = editorData.snapPosToSceneId.at(snappingIndex);
    mainApi -> unloadScene(sceneId);
    editorData.snapPosToSceneId.erase(snappingIndex);
  }
}


std::optional<int> getSceneIdForSnapIndex(EditorData& editorData, int index){
  if (editorData.snapPosToSceneId.find(index) != editorData.snapPosToSceneId.end()){
    return editorData.snapPosToSceneId.at(index);
  }
  return std::nullopt;
}

std::optional<int> getCurrSnapForScene(EditorData& editorData, int sceneId){
  for (auto &[snapindex, sceneIdAtSnapPos] : editorData.snapPosToSceneId){
    if (sceneId == sceneIdAtSnapPos){
      return snapindex;
    }
  }
  return std::nullopt;
}


void popoverAction(EditorData& editorData, std::string action){
  modlog("editor", "popover action: " + action);
  auto actionFn = nameAction.at(action);
  actionFn(editorData);
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
    { ")window_x", "script", "native/details" },
    //{ ")window_x", "script", "./res/scenes/editor/dock/details.scm" },
    // ;(list "(test_panel" "anchor" anchorElementName)
    { "(test_panel", "position", serializeVec(pos.has_value() ? pos.value() : glm::vec3(-0.78f, -0.097f, -1.f)) }, 
  };
  if (moveable){
    additionalTokens.push_back({ "(test_panel", "script", "native/dialogmove" });
  }
  if (restrictX){
    additionalTokens.push_back({ "(test_panel", "dialogmove-restrictx", "true" });
  }
  if (snapX){
    additionalTokens.push_back({ "(test_panel", "editor-shouldsnap", "true" });
  }
  std::optional<std::vector<std::string>> tags = std::optional<std::vector<std::string>>({ "editor" });

  auto sceneId = mainApi -> loadScene(scene, additionalTokens, std::nullopt, tags);
  auto testPanelId = mainApi -> getGameObjectByName("(test_panel", sceneId, false);
  if (testPanelId.has_value()){
    mainApi -> enforceLayout(testPanelId.value());
  }
  return sceneId;
}

void doPrintSnapPos(EditorData& editorData){
  std::string snapPosStr = "";
  for (auto &[snapPos, sceneId] : editorData.snapPosToSceneId){
    snapPosStr += std::to_string(snapPos) + "(" + std::to_string(sceneId) + ") ";
  }
  modlog("editor", "snap pos: [ " + snapPosStr + " ]\n");
}

// returns if did update
bool updateSnapPos(EditorData& editorData, int snappingIndex, int sceneId){
  if (editorData.snapPosToSceneId.find(snappingIndex) != editorData.snapPosToSceneId.end()){
    return false;
  }
  auto oldSnappingIndex = getCurrSnapForScene(editorData, sceneId);
  if (oldSnappingIndex.has_value()){
    editorData.snapPosToSceneId.erase(oldSnappingIndex.value());
  }
  editorData.snapPosToSceneId[snappingIndex] = sceneId;
  doPrintSnapPos(editorData);

  return true;
}


void changeSidepanel(EditorData& editorData, int snappingIndex, std::string scene, std::string anchorElementName){
  modlog("editor", "change sidepanel, load scene: " + scene + ", anchor: " + anchorElementName);
  maybeUnloadSidepanelSnap(editorData, snappingIndex);

  auto sceneId = getSceneIdForSnapIndex(editorData, snappingIndex);
  modassert(!sceneId.has_value(), "snap index already occupied");
  auto sidePanelSceneId = loadSidePanel(scene, std::nullopt, true, true, true);
  auto testPanelId = mainApi -> getGameObjectByName("(test_panel", sidePanelSceneId, false);
  bool didUpdate = updateSnapPos(editorData, snappingIndex, sidePanelSceneId);
  modassert(didUpdate, "could not load sidepanel, was occupied");
  mainApi -> enforceLayout(testPanelId.value());
  doPrintSnapPos(editorData);
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
    for (auto panelSceneId : editorData.panels){
      maybeUnloadSidepanelByScene(editorData, panelSceneId);
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

  std::cout << "element = " << elementName << ", is popover: " << isPopover << ", hasPopAction " << popAction.has_value() << std::endl;
  if (isPopover && popAction.has_value()){
    popoverAction(*editorData, popAction.value());
  }

  if (! (popOption.has_value() || isPopover)){
    maybeUnloadPopover(*editorData);
  }

  handleDialogClick(*editorData, elementName, objattrs);
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

struct SnappingPosition { 
  float xPos;
  glm::vec3 snappingPosition;
};

std::vector<SnappingPosition> snappingPositions = {
  //SnappingPosition { .xPos = 0.f, .snappingPosition = glm::vec3(0.78, -0.097f, 0.f) },
  SnappingPosition { .xPos = -1.2f, .snappingPosition = glm::vec3(-1.2f, -0.097f, 0.f) },
  SnappingPosition { .xPos = -1.f, .snappingPosition = glm::vec3(-0.78, -0.097f, 0.f) },
  SnappingPosition { .xPos = 1.f, .snappingPosition = glm::vec3(0.78 , -0.097f, 0.f) },
  SnappingPosition { .xPos = 1.2f, .snappingPosition = glm::vec3(1.2f, -0.097f, 0.f) },
};

int getSnappingPosition(glm::vec3 position){
  float minDistance = glm::abs(position.x - snappingPositions.at(0).xPos);
  int minIndex = 0;

  std::cout << "snapping, curr pos: " << print(position) << std::endl;
  for (int i = 0; i < snappingPositions.size(); i++){
    auto distance = glm::abs(position.x - snappingPositions.at(i).xPos);
    std::cout << "snapping: distance is: " << distance << ", index = " << i << ", min = " << minDistance << std::endl;
    if (distance < minDistance){
      minIndex = i;
      minDistance = distance;
    }
  }
  return minIndex;
}

void maybeHandleSidePanelDrop(EditorData& editorData, objid id){
  auto sceneId = mainApi -> listSceneId(id);
  auto attr = mainApi -> getGameObjectAttr(id);
  auto snapValue = getStrAttr(attr, "editor-shouldsnap");
  bool shouldSnap = snapValue.has_value() && snapValue.value() == "true";
  if (shouldSnap){
    auto pos = mainApi -> getGameObjectPos(id, true);
    int snappingIndex = getSnappingPosition(pos);
    bool didUpdate = updateSnapPos(editorData, snappingIndex, sceneId);
    if (didUpdate){
      auto snappingPos = snappingPositions.at(snappingIndex).snappingPosition;
      mainApi -> setGameObjectPos(id, glm::vec3(snappingPos.x, pos.y, pos.z));
      mainApi -> enforceLayout(id);
    }else{
      auto currSnapIndex = getCurrSnapForScene(editorData, sceneId);
      if (currSnapIndex.has_value()){
        auto snappingPos = snappingPositions.at(currSnapIndex.value()).snappingPosition;
        mainApi -> setGameObjectPos(id, glm::vec3(snappingPos.x, pos.y, pos.z));
        mainApi -> enforceLayout(id);
      }
    }
  }
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
    editorData -> activeDialogName = std::nullopt;
    editorData -> dialogSceneId = std::nullopt;

    auto args =  mainApi -> getArgs();
    editorData -> layoutToUse = args.find("layout") == args.end() ? std::optional<std::string>("main"): std::optional<std::string>(args.at("layout"));

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
    }else if (codepoint == 'i'){
      EditorData* editorData = static_cast<EditorData*>(data);
      doPrintSnapPos(*editorData);
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
      maybeHandleSidePanelDrop(*editorData, std::atoi(idStr -> c_str()));
    }else if (topic == "dock-self-remove"){
      modlog("editor", "should unload the dock because x was clicked");
      auto idStr = std::get_if<std::string>(&value);
      maybeUnloadSidepanelByScene(*editorData, mainApi -> listSceneId(std::atoi(idStr -> c_str())));
    }else if (topic == "play-mode"){
      auto valueStr = std::get_if<std::string>(&value);
      tintThemeColors(*valueStr == "true", mainApi -> listSceneId(scriptId));
    }
  };

  return binding;
}
