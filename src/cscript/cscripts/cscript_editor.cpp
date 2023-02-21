#include "./cscript_editor.h"

CustomApiBindings* mainApi;

struct EditorData {
  std::vector<objid> panels;
};

std::string popItemPrefix = ")text_";
const int popItemPrefixLength = popItemPrefix.size();
bool isPopoverElement(std::string name){
  return name.substr(0, popItemPrefixLength) == name;
}



void changePopover(std::string elementName, std::string uiOption){
  // 
}
void maybeUnloadPopover(){

}

std::string fullElementName(std::string localname){
  // (string-append "." (number->string mainSceneId) "/" localname))
  modassert(false, "not yet implemented");
  return localname;
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

void maybeUnloadSidepanelAll(){
  /*
  (define (maybe-unload-sidepanel-all)
  (for-each (lambda (sceneId) (maybe-unload-sidepanel-by-scene sceneId)) snappingPositionToSceneId)
)
*/
}

void changeSidepanel(int snappingIndex, std::string scene, std::string fullElementName){
  //(format #t "change sidepanel: elementname: ~a\n" anchorElementName)
  //(maybe-unload-sidepanel-snap snappingIndex)
  //(if (not (get-snap-id snappingIndex))
  //  (begin
  //    (update-snap-pos snappingIndex (loadSidePanel scene #f #t #t #t))
  //    (format #t "editor: load scene: ~a\n" scene)
  //    (format #t "sidepanel id is: ~a\n" (get-snap-id snappingIndex))
  //    (enforce-layout (gameobj-id (lsobj-name "(test_panel" (get-snap-id snappingIndex))))
  //  )
  //)
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

void onObjectSelected(int32_t id, int32_t index, glm::vec3 color){
  auto objattrs =  mainApi -> getGameObjectAttr(index);
  auto popOptionPair = getStrAttr(objattrs, "popoption");
  auto popAction = getStrAttr(objattrs, "popaction");
  bool hasPopOption = popOptionPair.has_value();
  auto elementName = mainApi -> getGameObjNameForId(index).value();
  auto isPopover = isPopoverElement(elementName);
  auto dialogOption = getStrAttr(objattrs, "dialogoption");

  if (hasPopOption){
    changePopover(fullElementName(elementName), popOptionPair.value());
  }else{
    maybeUnloadPopover();
  }

  if (isPopover && popAction.has_value()){
    popoverAction(popAction.value());
  }

  if (! (hasPopOption || isPopover)){
    maybeUnloadPopover();
  }

  handleDialogClick(elementName, objattrs);
  if (dialogOption.has_value() && dialogOption.value() != ""){
    if (dialogOption.value() == "HIDE"){
      maybeUnloadSidepanelAll();
    }else{
      changeSidepanel(1, dialogOption.value(), fullElementName("(menubar"));
    }
  }
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
  //;;;;;;;;;;;;;;;;;;;;;;;
  //(define (hasLayoutTable) 
  //  (define tables (map car (sql (sql-compile "show tables"))))
  //  (define tableExists (> (length (filter (lambda(x) (equal? x "layout")) tables)) 0))
  //  tableExists
  //)
  return false;
}

std::vector<objid> loadAllPanels(){
  // (define panelsMoveable (args "gridmove"))
  //(define (load-all-panels panelIdAndPos)
  //(map 
  //  (lambda (panelIdAndPos)
  //    (loadSidePanel (car panelIdAndPos) (cadr panelIdAndPos) panelsMoveable #f #f)
  //  )
  //  panelIdAndPos
  //)
  //)
  return {};
}

void loadPanelsFromDb(EditorData& editorData){
  std::cout << "should load all panels" << std::endl;
  if (hasLayoutTable()){
    //(for-each maybe-unload-sidepanel-by-scene panels) need to implement

  }
/*  (if (hasLayoutTable) 
    (begin
      
      (set! panels  (load-all-panels (tableLayout layout)))
    )
  )*/
}


CScriptBinding cscriptEditorBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/editor", api);
  mainApi = &api;
  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
    EditorData* editorData = new EditorData;
    editorData -> panels = {};

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
      //loadPanelsFromDb();
    }
    /* (define (onKeyChar key)
  (if (equal? key 46)
    (save-all-panels layoutToUse)
    ;(loadPanelsFromDb "main")
  ) */
  };


  binding.onObjectSelected = onObjectSelected;
  return binding;
}
