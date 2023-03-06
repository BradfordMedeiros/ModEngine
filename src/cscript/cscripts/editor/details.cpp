#include "./details.h"

extern CustomApiBindings* mainApi;

struct EditorDetails {
  std::optional<objid> activeSceneId;
  std::optional<objid> hoveredObj;
  std::optional<objid> focusedElement;
};

std::string uniqueName(){
  auto objName = std::to_string(rand() % 1000000000);
  return objName;
}

void createObject(EditorDetails& details, std::string prefix){
  GameobjAttributes attr {
    .stringAttributes = {},
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  mainApi -> makeObjectAttr(details.activeSceneId.value(), std::string(prefix) + uniqueName(), attr, submodelAttributes); 
}

void createCamera(EditorDetails& details){
  createObject(details, ">camera-");
}

void createLight(EditorDetails& details){
  createObject(details, "!light-");
}

void createSound(EditorDetails& details){
  GameobjAttributes attr {
    .stringAttributes = { {"clip", "./res/sounds/sample.wav" }},
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  mainApi -> makeObjectAttr(details.activeSceneId.value(), "&sound-" + uniqueName(), attr, submodelAttributes); 
}

void createText(EditorDetails& details){
  GameobjAttributes attr {
    .stringAttributes = { {"value", "sample text" }},
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  mainApi -> makeObjectAttr(details.activeSceneId.value(), ")text-" + uniqueName(), attr, submodelAttributes); 
}

void createGeo(EditorDetails& details){
  createObject(details, "<geo-");
}

void createPortal(EditorDetails& details){
  createObject(details, "@portal-");
}

void createHeightmap(EditorDetails& details){
  GameobjAttributes attr {
    .stringAttributes = { {"map", "./res/heightmaps/default.jpg" }},
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  mainApi -> makeObjectAttr(details.activeSceneId.value(), "-heightmap-" + uniqueName(), attr, submodelAttributes); 
}

void saveHeightmap(EditorDetails& details){
/*
  (define (saveHeightmap)
    (define name (getStoreValue "gameobj:map"))
    (format #t "save heightmap ~a\n" name)
    (format #t "not implemented")
    (exit 1)
  )
*/
}

void saveHeightmapAs(EditorDetails& details){
	/*
(define (saveHeightmapAs)
  (define name (getStoreValue "heightmap:filename"))
  (define isNamed (and name (> (string-length name) 0)))
  (if (and isNamed managedObj)
    (save-heightmap (gameobj-id managedObj) (string-append "./res/heightmaps/" name ".png"))
    (format #t "no name provided so cannot save heightmap\n")
  )
)
	*/
}

void createVoxels(EditorDetails& details){
  GameobjAttributes attr {
    .stringAttributes = { 
        { "from", "2|2|2|11001111" },
        { "fromtextures", "./res/brush/border_5x5.png,./res/heightmaps/dunes.jpg|10201020" },
    },
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  mainApi -> makeObjectAttr(details.activeSceneId.value(), "]voxel-" + uniqueName(), attr, submodelAttributes); 
}

void setManipulatorMode(std::string mode){
  mainApi -> setWorldState({ 
    ObjectValue {
      .object = "tools ",
      .attribute = "manipulator-mode",
      .value = mode,
    }
  });
}
void setAxis(std::string axis){
  mainApi -> setWorldState({ 
    ObjectValue {
      .object = "tools ",
      .attribute = "manipulator-axis",
      .value = axis,
    }
  }); 
}

/*
(define (togglePauseMode)
 (setPauseMode (not pauseModeEnabled))
)

(define playModeEnabled #f)

*/

void togglePlayMode(EditorDetails& details){
/*
(define (togglePlayMode)
  (if playModeEnabled
    (begin
      (setPauseMode #t)
      (for-each reset-scene (list-scenes (list "editable")))
    )
    (begin
      (setPauseMode #f)
    )
  )
  (set! playModeEnabled (not playModeEnabled))
  (updateStoreValue (list "play-mode-on" (if playModeEnabled "on" "off")))
  (format #t "play mode: ~a" (if playModeEnabled "true" "false"))
  (send "alert" (format #f "play mode: ~a" (if playModeEnabled "true" "false"))) 
  (send "play-mode" (if playModeEnabled "true" "false")) 

)*/
}

void togglePauseMode(EditorDetails& details){
  /*(define (setPauseMode enabled)
  (set! pauseModeEnabled enabled)
  (set-wstate (list
    (list "world" "paused" (if pauseModeEnabled "true" "false"))
  ))
  (send "alert" (string-append "pause mode " (if pauseModeEnabled "enabled" "disabled"))) 
  (updateStoreValue (list "pause-mode-on" (if pauseModeEnabled "on" "off")))
)
(define (togglePauseMode)
 (setPauseMode (not pauseModeEnabled))
)
*/
}

bool isSubmitKey(int key){
  return key == '/';
}
bool isControlKey(int key){
  return isSubmitKey(key) || key == 44 || key == 257 /* enter*/;
}

void submitData(EditorDetails& details){

}

bool shouldUpdateText(GameobjAttributes& attr){
  auto editableText = getStrAttr(attr, "details-editabletext");
  return editableText.has_value() && editableText.value() == "true";
}


enum DetailsUpdateType { UPDATE_BACKSPACE, UPDATE_DELETE, UPDATE_LEFT, UPDATE_RIGHT, UPDATE_UP, UPDATE_DOWN, UPDATE_SELECT_ALL, UPDATE_INSERT };
DetailsUpdateType getUpdateType(int key){
  if (key == 259 /* backspace */){
    return UPDATE_BACKSPACE;
  }else if (key == 96 /* delete */){
    return UPDATE_DELETE;
  }else if (key == 263 /* left */){
    return UPDATE_LEFT;
  }else if (key == 262 /* right */){
    return UPDATE_RIGHT;
  }else if (key == 265 /* up */){
    return UPDATE_UP;
  }else if (key == 264 /* down */){
    return UPDATE_DOWN;
  }else if (key == 344 /* shift */){
    return UPDATE_SELECT_ALL;
  }
  return UPDATE_INSERT;
}

std::string getUpdatedText(GameobjAttributes& attr, objid focusedElement, int key, int cursorIndex, std::string& oldCursorDir, int oldHighlight, DetailsUpdateType updateType){
/*
(define (getUpdatedText attr obj key cursorIndex cursorDir highlightLength eventType)
  (define effectiveIndex (if (equal? cursorDir "left") cursorIndex (+ cursorIndex 1)))
  (define currentText (cadr (assoc "value" attr)))
  (format #t "highlight length: ~a\n" highlightLength)
  (cond 
    ((equal? eventType 'backspace) (set! currentText (deleteChar currentText (if (<= highlightLength 0) effectiveIndex (+ effectiveIndex 1)) highlightLength)))
    ((equal? eventType 'delete) (set! currentText (deleteChar currentText (+ effectiveIndex 1) highlightLength)))
    ((or (equal? eventType 'selectAll) (equal? eventType 'left) (equal? eventType 'right) (equal? eventType 'up) (equal? eventType 'down)) #t)
    (#t 
      (begin
       ;(format #t "key is ~a ~a\n" key (string (integer->char key)))
        (set! currentText (appendString currentText key effectiveIndex highlightLength))
      )
    )
  )
  currentText
)
*/
  return "";
}

int newCursorIndex(DetailsUpdateType& eventType, int oldIndex, int newTextLength, int oldOffset, int wrapAmount, std::string& oldCursorDir, int oldHighlightLength){
  auto distanceOnLine = oldIndex - oldOffset;
  auto lastEndingOnRight = distanceOnLine == (wrapAmount - 1);
  auto lastEndingOnLeft = distanceOnLine == 0;
  auto oldCursorDirLeft = oldCursorDir == "left";
  auto newCursorDir = oldCursorDir;
  auto highlightLength = oldHighlightLength;

  auto index = oldIndex;
  if (eventType == UPDATE_UP){
    highlightLength = std::min(newTextLength - (oldCursorDirLeft ? oldIndex : (oldIndex + 1)), highlightLength + 1);
  }else if (eventType == UPDATE_DOWN){
    highlightLength = std::max(0, highlightLength -1);
  }else if (eventType == UPDATE_SELECT_ALL){
    highlightLength = newTextLength;
    newCursorDir = "left";
    index = 0;
  }else if ((eventType == UPDATE_LEFT) || ((eventType == UPDATE_BACKSPACE) && (oldHighlightLength <= 0))){
    if (!oldCursorDirLeft){
      newCursorDir = "left";
      index = oldIndex;
    }else{
      index = std::max(0, oldIndex - 1);
    }
  }else if (eventType == UPDATE_INSERT || eventType == UPDATE_RIGHT){
    if (lastEndingOnRight && oldCursorDirLeft){
      newCursorDir = "right";
      index = std::min(newTextLength, oldIndex);
    }else{
      index = std::min(oldIndex + 1, newCursorDir == "right" ? (newTextLength - 1) : newTextLength);
    }
  }

  if (highlightLength > 0){
    if (eventType == UPDATE_LEFT){
      index = oldIndex;
    }else if (eventType == UPDATE_RIGHT){
      index = oldIndex + oldHighlightLength;
    }
  }

  

/*

  (if (or (equal? eventType 'right) (equal? eventType 'left) (equal? eventType 'insert) (equal? eventType 'backspace) (equal? eventType 'delete))
    (set! highlightLength 0)
  )

  (format #t "last ending: (~a, ~a)\n" lastEndingOnLeft lastEndingOnRight)
  (format #t "distance on line: ~a\n" distanceOnLine)
  (list index newCursorDir highlightLength)
)
*/
  return 0;
}

int newOffsetIndex(DetailsUpdateType& type, int oldOffset, int cursor, int wrapAmount, int strLen){
/*
; todo -> take into account when the text is deleted 
(define (newOffsetIndex type oldoffset cursor wrapAmount strlen)
  (define rawCursorIndex (car cursor))
  (define newCursorIndex (if (equal? (cadr cursor) "left") rawCursorIndex (+ rawCursorIndex 1)))
  (define cursorHighlight (caddr cursor))
  (define cursorFromOffset (- (+ newCursorIndex cursorHighlight) oldoffset))
  (define wrapRemaining (- wrapAmount cursorFromOffset))
  (define cursorOverLeftSide (> wrapRemaining wrapAmount))
  (define cursorOverRightSide (< wrapRemaining 0))
  (define amountOverLeftSide (- wrapRemaining wrapAmount))
  (define amountOverRightSide (* -1 wrapRemaining))
  (define newOffset   
    (cond 
      (cursorOverRightSide (+ oldoffset amountOverRightSide))
      (cursorOverLeftSide  (- oldoffset amountOverLeftSide))
      (#t oldoffset)
    )
  )
  (define numCharsLeft (- strlen newOffset))
  (define diffFromWrap (- numCharsLeft wrapAmount))
  (define finalOffset (if (<= diffFromWrap 0)
    (+ newOffset diffFromWrap)
    newOffset
  ))

  (format #t "offset index, highlight: ~a\n" cursorHighlight)
  finalOffset
)

*/
  return 0;
}

bool shouldUpdateType(std::string& newText, GameobjAttributes& attr){
/*(define (shouldUpdateType newText attr)
  (define update (cond
    ((isEditableType "number" attr) 
      (format #t "evaluating number type!\n")
      (isTypeNumber newText)
    )
    ((isEditableType "positive-number" attr) 
      (format #t "evaluating positive number type!\n")
      (isTypePositiveNumber newText)
    )
    ((isEditableType "integer" attr)
      (isTypeInteger newText)
    )
    ((isEditableType "positive-integer" attr)
      (isTypePositiveInteger newText)
    )
    (#t #t)
  ))
  (format #t "should update: (~a) (~a)\n" newText update)
  update
)
*/
  return false;
}

void updateText(objid focusedElement, std::string& newText, int cursor, int offset){
/*
(define (updateText obj text cursor offset)
  (define cursorIndex (car cursor))
  (define cursorDir (cadr cursor))
  (define cursorHighlightLength (caddr cursor))
  (define objattr (gameobj-attr obj))
  (define detailBindingPair (assoc "details-binding" objattr))
  (define detailBindingIndexPair (assoc "details-binding-index" objattr))
  (define detailBinding (if detailBindingPair (cadr detailBindingPair) #f))
  (define detailBindingIndex (if detailBindingIndexPair (inexact->exact (cadr detailBindingIndexPair)) #f))
  (define editableTypePair (assoc "details-editable-type" objattr))
  (define editableType (if editableTypePair (cadr editableTypePair) #f))

  (define newValues 
    (list
      (list "offset" offset)
      (list "cursor" cursorIndex)
      (list "cursor-dir" cursorDir)
      (list "cursor-highlight" cursorHighlightLength)
      (list "value" text)
    )
  )

  (format #t "cursor highlight: ~a\n" cursorHighlightLength)
  (format #t "updating text: ~a\n" text)

  (gameobj-setattr! obj newValues)
  (if detailBinding
    (updateStoreValueModified (getUpdatedValue detailBinding detailBindingIndex editableType text) #t)
  )
  (format #t "cursor is: ~a\n" cursor)
)
*/
}

void processFocusedElement(EditorDetails& details, int key){
  if (details.focusedElement.has_value()){
    auto attr = mainApi -> getGameObjectAttr(details.focusedElement.value());
    if (shouldUpdateText(attr)){
      modlog("editor", "should update: " + std::to_string(details.focusedElement.value()));
      auto cursorIndex = static_cast<int>(getFloatAttr(attr, "cursor").value());
      auto oldCursorDir = getStrAttr(attr, "cursor-dir").value();
      auto oldHighlight = static_cast<int>(getFloatAttr(attr, "cursor-highlight").value());
      auto offsetIndex = static_cast<int>(getFloatAttr(attr, "offset").value());
      auto updateType = getUpdateType(key);
      auto wrapAmount = static_cast<int>(getFloatAttr(attr, "wrapamount").value());
      auto newText = getUpdatedText(attr, details.focusedElement.value(), key, cursorIndex, oldCursorDir, oldHighlight, updateType);
      auto cursor = newCursorIndex(updateType, cursorIndex, newText.size(), offsetIndex, wrapAmount, oldCursorDir, oldHighlight);
      auto offset = newOffsetIndex(updateType, offsetIndex, cursor, wrapAmount, newText.size());
      if (shouldUpdateType(newText, attr)){
        updateText(details.focusedElement.value(), newText, cursor, offset);
      }
    }else{
      modlog("editor", "details should not update: " + std::to_string(details.focusedElement.value()));
    }        

  }
  submitData(details);
}

std::map<std::string, std::function<void(EditorDetails&)>> buttonToAction = {
	{ "create-camera", createCamera },
	{ "create-light", createLight },
	{ "create-sound", createSound },
	{ "create-text", createText },
	{ "create-geo", createGeo },
	{ "create-portal", createPortal },
	{ "create-heightmap", createHeightmap },
	{ "save-heightmap", saveHeightmap },
	{ "save-heightmap-as", saveHeightmapAs },
	{ "create-voxels", createVoxels },
  { "set-transform-mode", [](EditorDetails&) -> void { setManipulatorMode("translate"); }},
  { "set-scale-mode", [](EditorDetails&) -> void { setManipulatorMode("scale"); }},
  { "set-rotate-mode", [](EditorDetails&) -> void { setManipulatorMode("rotate"); }},
  { "toggle-play-mode", togglePlayMode },
  { "toggle-pause-mode", togglePauseMode },
  { "set-axis-x", [](EditorDetails&) -> void { setAxis("x"); }},
  { "set-axis-y", [](EditorDetails&) -> void { setAxis("y"); }},
  { "set-axis-z", [](EditorDetails&) -> void { setAxis("z"); }},
  { "copy-object", [](EditorDetails&) -> void { mainApi -> sendNotifyMessage("copy-object", "true"); }},
};

CScriptBinding cscriptDetailsBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/details", api);

  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
  	EditorDetails* details = new EditorDetails;
    details -> activeSceneId = std::nullopt;
    details -> hoveredObj = std::nullopt;
    details -> focusedElement = std::nullopt;
    return details;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    EditorDetails* details = static_cast<EditorDetails*>(data);
    delete details;
  };

  binding.onMessage = [](objid scriptId, void* data, std::string& topic, AttributeValue& value) -> void {
/*
  (if (equal? key "explorer-sound-final")
    (begin
      (updateDialogValues "load-sound" value)
      (submitAndPopulateData)
    )
  )
  (if (equal? key "explorer-heightmap-brush-final")
    (begin
      (updateDialogValues "load-heightmap-brush" value)
      (submitAndPopulateData)
    )
  )
  (if (equal? key "explorer-heightmap-final")
    (begin
      (updateDialogValues "load-heightmap" value)
      (submitAndPopulateData)
    )
  )
  (if (equal? key "active-scene-id")
    (set! activeSceneId (string->number value))
  )
  (if (equal? key "editor-button-on")
    (begin
      (toggleButtonBinding (string->number value) #t)
      (submitAndPopulateData) ; remove
    )

  )
  (if (equal? key "editor-button-off")
    (begin
      (toggleButtonBinding (string->number value) #f)
      (submitAndPopulateData) ; remove
    )
  )
  (if (equal? key "details-editable-slide")
    (format #t "slide: ~a\n" (onSlide (getSlidePercentage (string->number value))))
  )
  */
  };


  binding.onKeyCallback = [](int32_t id, void* data, int key, int scancode, int action, int mods) -> void {
    EditorDetails* details = static_cast<EditorDetails*>(data);
    if (action == 0){
      if (key == ';'){
        togglePauseMode(*details);
      }else if (key == '='){
        togglePlayMode(*details);
      }
    }
    if (
      (action == 1 || action == 2) && 
      !isControlKey(key) && 
      (key == 262 || key == 263 || key == 264 || key == 265 || key == 259 || key == 261) // /* arrow, backspace, delete
    ){
      processFocusedElement(*details, key);
    }
  };

  binding.onObjectHover = [](objid scriptId, void* data, int32_t index, bool hoverOn) -> void {
    EditorDetails* details = static_cast<EditorDetails*>(data);
    if (hoverOn){
      details -> hoveredObj = index;
    }else{
      details -> hoveredObj = std::nullopt;
    }
  };

  binding.onObjectSelected = [](int32_t id, void* data, int32_t index, glm::vec3 color) -> void {
/*
  (define objattr (gameobj-attr gameobj))
  (define reselectAttr (assoc "details-reselect" objattr))
  (define sceneId (list-sceneid (gameobj-id gameobj)))
  (define objInScene (equal? sceneId (list-sceneid (gameobj-id mainobj))))
  (define managedText (and objInScene (isManagedText gameobj)))
  (define valueIsSelectionType (assoc "details-value-selection" objattr))
  (define valueDialogType (assoc "details-value-dialog" objattr))

  (if (and objInScene reselectAttr)
    (onObjSelected (lsobj-name (cadr reselectAttr)) #f)
    (begin
      (if (equal? (gameobj-id gameobj) (gameobj-id mainobj)) ; assumes script it attached to window x
        (send "dock-self-remove" (number->string (gameobj-id mainobj)))
      )

      (if valueDialogType
        (send "explorer" (cadr valueDialogType))
      )
      (if valueIsSelectionType
        (if managedText 
          (setManagedSelectionMode gameobj)
          (format #t "selection type but not text\n")
        )
      )

      (if (not managedTextSelectionMode)
        (begin
          (if managedText
            (begin
              ;(format #t "is is a managed element: ~a\n" (gameobj-name gameobj))
              (unsetFocused)
              (set! focusedElement gameobj)
              (gameobj-setattr! gameobj 
                (list (list "tint" (list 0.3 0.3 0.6 1)))
              )
            )
            (unsetFocused)
          )
          
          (if (isSelectableItem (assoc "layer" objattr))
            (begin
              (refillStore gameobj)
              (set! managedObj gameobj)
              (populateData)
            )
          )
          (maybe-set-binding objattr)
          (if managedText (maybe-set-text-cursor gameobj))
        )
      )
    )
  )
  */
	};

  return binding;
}
