#include "./details.h"

extern CustomApiBindings* mainApi;

struct EditorDetails {
  std::optional<objid> activeSceneId;
  std::optional<objid> hoveredObj;
  std::optional<objid> focusedElement;
  std::optional<objid> managedObj;
  bool playModeEnabled;
  bool pauseModeEnabled;
  std::optional<objid> managedSelectionMode;

  std::map<std::string, AttributeValue> dataValues;
};

std::string uniqueName(){
  auto objName = std::to_string(rand() % 1000000000);
  return objName;
}

struct UpdatedValue {
  std::string key;
  std::string value;
};

std::optional<std::string> getDataValue(EditorDetails& details, std::string attrField){
/*(define (getDataValue attrField) 
  (define value (assoc attrField dataValues))
  (if value (cadr value) #f)
)
*/
  return std::nullopt;
} 

UpdatedValue getUpdatedValue(EditorDetails& details, std::string& detailBindingName, int detailBindingIndex, std::string& detailBindingType, std::string& newvalueOld) {
  //auto oldvalue = getDataValue()
//
//  //(define oldvalue (getDataValue detailBindingName))
//  //(define newvalue (if (equal? detailBindingType "list") (fixList newvalueOld) newvalueOld))
//  //(if detailBindingIndex
//  //    (begin
//  //      (if (equal? oldvalue #f)
//  //        (set! oldvalue (list 0 0 0 0))  ; should come from some type hint
//  //      )
//  //      (list-set! oldvalue detailBindingIndex (makeTypeCorrect (list-ref oldvalue detailBindingIndex) newvalue))
//  //      (format #t "old value: ~a ~a\n"  oldvalue (map number? oldvalue))
//  //      (list detailBindingName oldvalue)
//  //    )
//  //    (list detailBindingName (makeTypeCorrect oldvalue newvalue))
  //)    
  return UpdatedValue{ };
}


void updateStoreValueModified(std::string key, std::string value, bool modified){
  /*
  (define newDataValues (delete (assoc key dataValues) dataValues))
  (set! dataValues (cons (list key value modified) newDataValues))*/

}
void updateStoreValue(std::string key, std::string value){
  updateStoreValueModified(key, value, false);
}
std::string getStoreValue(std::string key){
/*(define (getStoreValue key)
  (define value (assoc key dataValues))
  (if value (cadr value) #f)
)
*/
  return "";
}

void refillStore(EditorDetails& details, objid id){
/*(define (refillStore gameobj)
  (clearStore)
  (updateStoreValue (list "object_name" (gameobj-name gameobj)))
  (format #t "store: all attrs are: ~a\n" (gameobj-attr gameobj))
  (map updateStoreValue (map refillCorrectType (map getRefillGameobjAttr (gameobj-attr gameobj))))

  (updateStoreValue (list "meta-numscenes" (number->string (length (list-scenes)))))
  (updateStoreValue (list "runtime-id" (number->string (gameobj-id gameobj))))
  (updateStoreValue (list "runtime-sceneid" (number->string (list-sceneid (gameobj-id gameobj)))))

  (updateStoreValue (list "play-mode-on" (if playModeEnabled "on" "off")))
  (updateStoreValue (list "pause-mode-on" (if pauseModeEnabled "on" "off")))

  (for-each updateStoreValue (map refillCorrectType (map getRefillStoreWorldValue (get-wstate))))

  (populateSqlData)
)
*/
}
void populateData(EditorDetails& details){

}

void submitAndPopulateData(EditorDetails& details){
  /*(define (submitAndPopulateData)
  (submitData)
  (populateData)
)*/
}

void maybeSetBinding(GameobjAttributes& attr){
/*(define (maybe-set-binding objattr)
  (define shouldSet (if (assoc "details-binding-set" objattr) #t #f))
  (define detailBindingPair (assoc "details-binding-toggle" objattr))
  (define detailBindingIndexPair (assoc "details-binding-index" objattr))
  (define detailBinding (if detailBindingPair (cadr detailBindingPair) #f))
  (define detailBindingIndex (if detailBindingIndexPair (inexact->exact (cadr detailBindingIndexPair)) #f))
  (define bindingOn (assoc "details-binding-on" objattr))
  (define enableValue (if bindingOn (cadr bindingOn) #f))
  (define editableTypePair (assoc "details-editable-type" objattr))
  (define editableType (if editableTypePair (cadr editableTypePair) #f))

  ;;(format #t "shouldset = ~a, enableValue = ~a, detailBinding = ~a\n" shouldSet enableValue detailBinding)
  (if (and shouldSet enableValue detailBinding) 
    (updateStoreValueModified (getUpdatedValue detailBinding detailBindingIndex editableType enableValue) #t)
  )
  (submitAndPopulateData)
)
*/
}
void maybeSetTextCursor(objid obj){
/*(define (maybe-set-text-cursor gameobj)
  (define objattr (gameobj-attr gameobj))
  (define value (assoc "value" objattr))
  (define wrap (assoc "wrapamount" objattr))
  (define offset (assoc "offset" objattr))

  (define wrapAmount (if wrap (cadr wrap) #f))
  (define text (if value (cadr value) #f))
  (define offsetValue (if offset (cadr offset) #f))
  (if (and text wrapAmount offsetValue)
    (let ((cursorValue (min (- (max 1 (string-length text)) 1) (+ (- wrapAmount 1) offsetValue))))
      (assert (>= cursorValue 0) (format #f "name: ~a => cursor value is expected to be >= 0 (got = ~a), wrapAmount = ~a, text = ~a, offset = ~a\n" (gameobj-name gameobj) cursorValue wrapAmount text offsetValue))
      (gameobj-setattr! gameobj 
        (list
          (list "cursor" cursorValue)
          (list "cursor-dir" "right")
          (list "cursor-highlight" 0)
        )
      )
    )
  )
)
*/
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
  auto name = getStoreValue("gameobj:map");
  modassert(false, std::string("save heightmap not implemented, tried to save: ") + name);
}

void saveHeightmapAs(EditorDetails& details){
  auto name = getStoreValue("heightmap:filename");
  bool isNamed = name.size() > 0;
  if (isNamed && details.managedObj.has_value()){
    mainApi -> saveHeightmap(details.managedObj.value(), std::string("./res/heightmaps/") + name + ".png");
  }
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



void setPauseMode(EditorDetails& details, bool enabled){
  details.pauseModeEnabled = enabled;
  mainApi -> setWorldState({ 
    ObjectValue {
      .object = "world ",
      .attribute = "paused",
      .value = details.pauseModeEnabled ? "true" : "false",
    }
  });
  mainApi -> sendNotifyMessage("alert", std::string("pause mode ") + (details.pauseModeEnabled ? "enabled" : "disabled"));
  updateStoreValue("pause-mode-on", details.pauseModeEnabled ? "on" : "off");
}
void togglePlayMode(EditorDetails& details){
  if (details.playModeEnabled){
    setPauseMode(details, true);
    std::vector<std::string> tags = { "editable" };
    for (auto &scene : mainApi -> listScenes(tags)){
      mainApi -> resetScene(scene);
    }
  }else{
    setPauseMode(details, false);
  }
  details.playModeEnabled = !details.playModeEnabled;
  details.pauseModeEnabled = true;
  updateStoreValue("play-mode-on", details.playModeEnabled ? "on" : "off");
  modlog("editor", "play mode: " + details.playModeEnabled ? "true" : "false");
  mainApi -> sendNotifyMessage("alert", details.playModeEnabled ? "true" : "false");
  mainApi -> sendNotifyMessage("play-mode", details.playModeEnabled ? "true" : "false");
}

void togglePauseMode(EditorDetails& details){
  setPauseMode(details, !details.pauseModeEnabled);
}

bool isSubmitKey(int key){
  return key == '/';
}
bool isControlKey(int key){
  return isSubmitKey(key) || key == 44 || key == 257 /* enter*/;
}

void submitData(EditorDetails& details){
/*(define (submitData)
  (if managedObj
    (begin
      (let* (
        (updatedValues (map maybe-serialize-vec2 (filterUpdatedObjectValues))) 
        (objattr (getPrefixAttr updatedValues "gameobj"))
        (worldattr (getPrefixAttr updatedValues "world"))
      )
        (if (gameobj-name managedObj)
          (begin
            (gameobj-setattr! managedObj objattr)
            (format #t "set attr: ~a ~a\n" (gameobj-name managedObj) objattr)
            (format #t "world attr: ~a\n" worldattr)
          )
        )
        (submitWorldAttr worldattr)
        (submitSqlUpdates updatedValues)
      )
    )
  )
)
*/
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

std::string appendString(std::string& currentText, int key, int cursorIndex, int highlightLength){
  auto length = currentText.size();
  auto splitIndex = std::min(static_cast<int>(length), cursorIndex);
  auto start = currentText.substr(0, splitIndex);
  auto end = currentText.substr(splitIndex + highlightLength, length);
  return start + static_cast<char>(key) +end;
}

std::string deleteChar(std::string& currentText, int cursorIndex, int highlightLength){
  auto length = currentText.size();
  if ((cursorIndex > 0) && (cursorIndex < (length + 1))){
    auto splitIndex = std::min(static_cast<int>(length), cursorIndex - 1);
    auto start = currentText.substr(0, splitIndex);
    auto end = currentText.substr(highlightLength ==  0 ? (splitIndex + 1) : (splitIndex + highlightLength), length);
    currentText = start + end;
  }
  return currentText;
}

std::string getUpdatedText(GameobjAttributes& attr, objid focusedElement, int key, int cursorIndex, std::string& cursorDir, int highlightLength, DetailsUpdateType updateType){
  auto effectiveIndex = cursorDir == "left" ? cursorIndex : (cursorIndex + 1);
  auto currentText = getStrAttr(attr, "value").value();

  if (updateType == UPDATE_BACKSPACE){
    currentText = deleteChar(currentText, highlightLength <= 0 ? effectiveIndex : (effectiveIndex + 1) , highlightLength);
  }else if (updateType == UPDATE_DELETE){
    currentText = deleteChar(currentText, effectiveIndex + 1, highlightLength);
  }else if ((updateType == UPDATE_SELECT_ALL) || (updateType == UPDATE_LEFT) || (updateType == UPDATE_RIGHT) || (updateType == UPDATE_UP) || (updateType == UPDATE_DOWN)){
    // do nothing
  }else {
    currentText = appendString(currentText, key, cursorIndex, highlightLength);
  }
  return currentText;
}

struct CursorUpdate {
  int index;
  int highlightLength;
  std::string newCursorDir;
};
CursorUpdate newCursorIndex(DetailsUpdateType& eventType, int oldIndex, int newTextLength, int oldOffset, int wrapAmount, std::string& oldCursorDir, int oldHighlightLength){
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

  if ((eventType == UPDATE_RIGHT) || (eventType == UPDATE_LEFT) || (eventType == UPDATE_INSERT) || (eventType == UPDATE_BACKSPACE) || (eventType == UPDATE_DELETE)){
    highlightLength = 0;
  }

  return CursorUpdate {
    .index = index,
    .highlightLength = highlightLength,
    .newCursorDir = newCursorDir,
  };
}

int newOffsetIndex(DetailsUpdateType& type, int oldOffset, CursorUpdate& cursor, int wrapAmount, int strLen){
  auto rawCursorIndex = cursor.index;
  auto newCursorIndex = cursor.newCursorDir == "left" ? rawCursorIndex : (rawCursorIndex + 1);
  auto cursorHighlight = cursor.highlightLength;
  auto cursorFromOffset = newCursorIndex + cursorHighlight - oldOffset;
  auto wrapRemaining = wrapAmount - cursorFromOffset;
  auto cursorOverLeftSide = wrapRemaining > wrapAmount;
  auto cursorOverRightSide = wrapRemaining < 0;
  auto amountOverLeftSide = wrapRemaining - wrapAmount;
  auto amountOverRightSide = -1 * wrapRemaining;
  auto newOffset = oldOffset;
  if (cursorOverRightSide){
    newOffset = oldOffset + amountOverRightSide;
  }
  if (cursorOverLeftSide){
    newOffset = oldOffset - amountOverLeftSide;
  }
  auto numCharsLeft = strLen - newOffset;
  auto diffFromWrap = numCharsLeft - wrapAmount;
  auto finalOffset = (diffFromWrap <= 0) ? (newOffset + diffFromWrap) : newOffset;
  return finalOffset;
}

bool isEditableType(std::string type, GameobjAttributes& attr){
  /*
  (define (isEditableType type attr) 
  (define editableText (assoc "details-editable-type" attr))
  (define isEditableType (if editableText (equal? (cadr editableText) type) #f))
  (format #t "is editable type: ~a ~a\n" type isEditableType)
  isEditableType
)*/
  return false;
}

bool isZeroLength(std::string& text){ return text.size() == 0; }
bool isNumber(std::string& text){ 
  float number;
  return maybeParseFloat(text, number);
}
bool isDecimal(std::string& text){
  return text.size() == 1 && text.substr(0, 1) == ".";
} 
bool isNegativePrefix(std::string& text){
  return text.size() == 1 && text.substr(0, 1) == "-";
}
bool isPositiveNumber(std::string& text){
  float number;
  bool isFloat = maybeParseFloat(text, number);
  return isFloat && number >= 0.f;
}

bool isPositiveInteger(std::string& text){
  auto asInt = std::atoi(text.c_str());
  return asInt >= 0 && std::to_string(asInt) == text;
}
bool isInteger(std::string& text){
  auto asInt = std::atoi(text.c_str());
  return std::to_string(asInt) == text;
}
bool isTypeNumber(std::string& text){
  return isZeroLength(text) || isNumber(text) || isDecimal(text) || isNegativePrefix(text);
}
bool isTypePositiveNumber(std::string& text){
  return isZeroLength(text) || isDecimal(text) || isPositiveNumber(text);
} 
bool isTypeInteger(std::string& text){
  return isZeroLength(text) || isInteger(text) || isNegativePrefix(text);
}
bool isTypePositiveInteger(std::string& text){
  return isZeroLength(text) || isPositiveInteger(text);
}

bool shouldUpdateType(std::string& newText, GameobjAttributes& attr){
  if (isEditableType("number", attr)){
    return isTypeNumber(newText);
  }else if (isEditableType("positive-number", attr)){
    return isTypePositiveNumber(newText);
  }else if (isEditableType("integer", attr)){
    return isTypeInteger(newText);
  }else if (isEditableType("positive-integer", attr)){
    return isTypePositiveInteger(newText);
  }
  return false;
}


void updateText(EditorDetails& details, objid obj, std::string& text, CursorUpdate& cursor, int offset, GameobjAttributes& attr){
  auto cursorIndex = cursor.index;
  auto cursorDir = cursor.newCursorDir;
  auto cursorHighlightLength = cursor.highlightLength;

  auto detailBinding = getStrAttr(attr, "details-binding");
  auto detailBindingIndex = static_cast<int>(getFloatAttr(attr, "details-binding-index").value()); // should cast to int
  auto editableType = getStrAttr(attr, "details-editable-type");

  GameobjAttributes newAttr {
    .stringAttributes = { {"cursor-dir", cursorDir}, { "value", text } },
    .numAttributes = { {"offset", offset }, { "cursor", cursorIndex}, { "cursor-highlight", cursorHighlightLength} },
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  mainApi -> setGameObjectAttr(obj, newAttr);
  if (detailBinding.has_value()){
    auto updateValue = getUpdatedValue(details, detailBinding.value(), detailBindingIndex, editableType.value(), text);
    updateStoreValueModified(updateValue.key, updateValue.value, true);

  }
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
        updateText(details, details.focusedElement.value(), newText, cursor, offset, attr);
      }
    }else{
      modlog("editor", "details should not update: " + std::to_string(details.focusedElement.value()));
    }        

  }
  submitData(details);
}

std::string print(std::map<std::string, AttributeValue> values){
  std::string value = "";
  for (auto &[key, value] : values){
    value = "(" + key + ", " + print(value) + ") ";
  }
  return value;
}

bool isManagedText(GameobjAttributes& attr, std::string objname){
  return shouldUpdateText(attr) && objname.at(0) == ')';
}


void setManagedSelectionMode(EditorDetails& details, objid obj){
  details.managedSelectionMode = obj;
  modlog("editor", std::string("details - set managed selection mode") + std::to_string(obj));  
}
void finishManagedSelectionMode(EditorDetails& details, objid obj){
  modlog("editor", "details - finished managed selection mode");  
  auto name = mainApi -> getGameObjNameForId(obj).value();
  CursorUpdate cursor { .index = 0, .highlightLength = 0, .newCursorDir = "left" };
  auto attr = mainApi -> getGameObjectAttr(obj);
  updateText(details, obj, name, cursor, 0, attr);
  details.managedSelectionMode = std::nullopt;
}

bool isSelectableItem(GameobjAttributes& attr){
  auto layer = getStrAttr(attr, "layer");
  return !layer.has_value() || layer.value() != "basicui";
}


void unsetFocused(EditorDetails& details){
  if (details.focusedElement.has_value()){
    GameobjAttributes newAttr {
      .stringAttributes = { {"value", ""}, { "cursor-dir", "left" }},
      .numAttributes = { { "cursor", -1.f }, { "cursor-highlight", 0 } },
      .vecAttr = { .vec3 = {}, .vec4 = { { "tint", glm::vec4(1.f, 1.f, 1.f, 1.f) } }},
    };
    mainApi -> setGameObjectAttr(details.focusedElement.value(), newAttr);
  }
  details.focusedElement = std::nullopt;
}

void onDetailsObjectSelected(int32_t id, void* data, int32_t index) {
    EditorDetails* details = static_cast<EditorDetails*>(data);
    auto attr = mainApi -> getGameObjectAttr(index);
    auto reselectAttr = getStrAttr(attr, "details-reselect");
    auto sceneId = mainApi -> listSceneId(index);
    auto objInScene = sceneId == mainApi -> listSceneId(id);
    auto managedText = objInScene && isManagedText(attr, mainApi -> getGameObjNameForId(index).value());
    auto valueIsSelectionType = getStrAttr(attr, "details-value-selection");
    auto valueDialogType = getStrAttr(attr, "details-value-dialog");
    if (objInScene && reselectAttr.has_value()){
      onDetailsObjectSelected(id, data, mainApi -> getGameObjectByName(reselectAttr.value(), sceneId, true).value());
      return;
    }  

    if (index == id){ // ; assumes script it attached to window x
      mainApi -> sendNotifyMessage("dock-self-remove", std::to_string(id));
    }
    if (valueDialogType.has_value()){
      mainApi -> sendNotifyMessage("explorer", valueDialogType.value());
    }
    if (valueIsSelectionType.has_value() && managedText){
      setManagedSelectionMode(*details, index);
    }

    if (!details -> managedSelectionMode.has_value()){
      if (managedText){
        unsetFocused(*details);
        details -> focusedElement = index;
        GameobjAttributes newAttr {
          .stringAttributes = {},
          .numAttributes = {},
          .vecAttr = { .vec3 = {}, .vec4 = { { "tint", glm::vec4(0.3f, 0.3f, 0.6f, 1.f) } }},
        };
        mainApi -> setGameObjectAttr(index, newAttr);
      }else{
        unsetFocused(*details);
      }

      if (isSelectableItem(attr)){
        refillStore(*details, index);
        details -> managedObj = index;
        populateData(*details);
      }
      maybeSetBinding(attr);
      if (managedText){
        maybeSetTextCursor(index);
      }
    }
}

void updateDialogValues(std::string dialogType, AttributeValue value){
  /*(define (updateDialogValues dialogType value)
  (define allQueriesObj (lsobj-attr "details-value-dialog"))
  (define matchingDialogObjs (filter (lambda(obj) (objAttrEqual obj "details-value-dialog" dialogType)) allQueriesObj))
  (for-each 
    (lambda(obj)  
      (updateBindingWithValue value (gameobj-attr obj))
    ) 
    matchingDialogObjs
  )
)
*/
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
    details -> managedObj = std::nullopt;
    details -> playModeEnabled = false;
    details -> pauseModeEnabled = true;
    details -> managedSelectionMode = std::nullopt;
    details -> dataValues = {};
    return details;
  };
  binding.remove = [&api] (std::string scriptname, objid id, void* data) -> void {
    EditorDetails* details = static_cast<EditorDetails*>(data);
    delete details;
  };

  binding.onMessage = [](objid scriptId, void* data, std::string& topic, AttributeValue& value) -> void {
    EditorDetails* details = static_cast<EditorDetails*>(data);
    if (topic == "debug-details"){
      modlog("editor", "details - data values: [" + print(details -> dataValues) + "]");
    }else if (topic == "explorer-sound-final"){
      updateDialogValues("load-sound", value);
      submitAndPopulateData(*details);
    }else if (topic == "explorer-heightmap-brush-final"){
      updateDialogValues("load-heightmap-brush", value);
      submitAndPopulateData(*details);
    }else if (topic == "explorer-heightmap-final"){
      updateDialogValues("load-heightmap", value);
      submitAndPopulateData(*details);
    }else if (topic == "active-scene-id"){
      auto strValue = std::get_if<std::string>(&value);
      modassert(strValue, "active-scene-id: str value is null");
      details -> activeSceneId = std::atoi(strValue -> c_str());
    }

/*

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
    onDetailsObjectSelected(id, data, index);

  };
  binding.onObjectUnselected = [](int32_t id, void* data) -> void {
    EditorDetails* details = static_cast<EditorDetails*>(data);
    unsetFocused(*details);
  };


  return binding;
}
