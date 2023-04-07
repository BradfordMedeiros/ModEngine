#include "./details.h"

extern CustomApiBindings* mainApi;

struct DataValue {
  bool modified;
  AttributeValue value;
};

struct EditorDetails {
  objid editorSceneId;
  std::optional<objid> activeSceneId;
  std::optional<objid> hoveredObj;
  std::optional<objid> focusedElement;
  std::optional<objid> managedObj;
  bool playModeEnabled;
  bool pauseModeEnabled;
  std::optional<objid> managedSelectionMode;

  std::map<std::string, DataValue> dataValues;
  bool ctrlHeld;
};

std::string uniqueName(){
  auto objName = std::to_string(rand() % 1000000000);
  return objName;
}

struct UpdatedValue {
  std::string key;
  AttributeValue value;
};

std::optional<AttributeValue> getDataValue(EditorDetails& details, std::string attrField){
  if (details.dataValues.find(attrField) == details.dataValues.end()){
    return std::nullopt;
  }
  return details.dataValues.at(attrField).value;
} 


std::string print(std::map<std::string, DataValue> values){
  std::string strValue = "";
  for (auto &[key, value] : values){
    strValue += "( " + key + ", " + print(value.value) + ") ";
  }
  return strValue;
}
void prettyPrint(std::map<std::string, DataValue>& dataValues){
  std::cout << "data values: [\n";
  for (auto &[key, value] : dataValues){
    std::cout << "( " << key << ", value = " << print(value.value) << " )\n";
  }
  std::cout << "]" << std::endl;
}


std::optional<UpdatedValue> getUpdatedValue(EditorDetails& details, std::string& detailBindingName, std::optional<int> detailBindingIndex, AttributeValue newValue) {
  auto oldValue = getDataValue(details, detailBindingName);
  if (!oldValue.has_value()){
    prettyPrint(details.dataValues);
    modlog("editor", "get update value no old value for: " + detailBindingName);
    return std::nullopt;
  }

  modlog("editor", "details - update " + print(oldValue.value()) + ", " + print(newValue));
  auto oldValueStr = std::get_if<std::string>(&oldValue.value());
  auto oldValueVec3 = std::get_if<glm::vec3>(&oldValue.value());
  auto oldValueVec4 = std::get_if<glm::vec4>(&oldValue.value());
  auto oldValueFloat = std::get_if<float>(&oldValue.value());

  auto newValueStr = std::get_if<std::string>(&newValue);
  auto newValueVec3 = std::get_if<glm::vec3>(&newValue);
  auto newValueVec4 = std::get_if<glm::vec4>(&newValue);
  auto newValueFloat = std::get_if<float>(&newValue);

  if (detailBindingIndex.has_value()){
    if (oldValueStr){
      modassert(false, "detail binding index specified for string type, not allowed");
    }else if (oldValueFloat){
      modassert(false, "detail binding index specified for float type, not allowed");
    }else if (oldValueVec3){
      modassert(newValueFloat, "old value is vec3, new value must be float, got: " + print(newValue));
      modassert(detailBindingIndex.value() <= 2, "detail binding index must be <= 2 for vec3 target type");
      (*oldValueVec3)[detailBindingIndex.value()] = *newValueFloat;
      std::cout << "update value vec3 with value: " << *newValueFloat << std::endl;
    }else if (oldValueVec4){
      modassert(newValueFloat, "old value is vec4, new value must be float, got: " + print(newValue));
      modassert(detailBindingIndex.value() <= 3, "detail binding index must be <= 3 for vec4 target type");
      (*oldValueVec4)[detailBindingIndex.value()] = *newValueFloat;
    }else{
      modassert(false, "default case should not reach");
    }
 
  }else{
    if (oldValueStr){
      modassert(newValueStr, "old value is a string, new value must be a string, got: " + print(newValue));
      oldValue = *newValueStr;
    }else if (oldValueFloat){
      modassert(newValueFloat, "old value is float, new value must be float, got: " + print(newValue));
      oldValue = *newValueFloat;
    }else if (oldValueVec3){
      modassert(false, "not yet implemented vec3");
    }else if (oldValueVec4){
      modassert(false, "not yet implemented vec4");
    }else{
      modassert(false, "default case should not reach");
    }
     
  }
 
  return UpdatedValue{ .key = detailBindingName, .value = oldValue.value() };
}


void updateStoreValueModified(EditorDetails& details, std::string key, AttributeValue value, bool modified){
  modlog("editor", "details - updateStoreValueModified: " + key + ", " + print(value));
  details.dataValues[key] = DataValue { .modified = modified, .value = value };

}
void updateStoreValue(EditorDetails& details, std::string key, AttributeValue value){
  updateStoreValueModified(details, key, value, false);
}
AttributeValue getStoreValue(EditorDetails& details, std::string key){
  return details.dataValues.at(key).value;
}

template <typename T>
T getAsType(AttributeValue value){
  auto tValue = std::get_if<T>(&value);
  modassert(tValue, "attribute value must be different type");
  return *tValue;
}

void clearStore(EditorDetails& details){
  details.dataValues = {};
}

AttributeValue uiStringToAttrVal(std::string& text, AttributeValue& oldValue, std::optional<int> detailBindingIndex){
  auto strValue = std::get_if<std::string>(&oldValue);
  auto vec3Value = std::get_if<glm::vec3>(&oldValue);
  auto vec4Value = std::get_if<glm::vec4>(&oldValue);
  auto floatValue = std::get_if<float>(&oldValue);
  modassert(strValue || vec3Value || vec4Value || floatValue, "uiStringToAttrVal invalid type for attributeValue");
  if (strValue){
    modassert(!detailBindingIndex.has_value(), "old value string value, cannot have detail binding index");
    return text;
  }
  if (floatValue || vec3Value || vec4Value){
    if (vec3Value || vec4Value){
      modassert(detailBindingIndex.has_value(), "old value vec3, vec4 value, must have detail binding index");
    }
    if (floatValue){
      modassert(!detailBindingIndex.has_value(), "old value float must not have detail binding index");
    }
    if (text == "" || text == "-" || text == "."){
      return 0.f;
    }
    float number = 0.f;
    bool isFloat = maybeParseFloat(text, number);
    modassert(isFloat, "uiStringToAttrVal invalid float: " + text);
    return number;
  }
  modassert(false, "invalid uiStringToAttrVal, not yet supported");
  return text;
}

std::string attrValToUiString(AttributeValue& value){
  auto strValue = std::get_if<std::string>(&value);
  if (strValue){
    return *strValue;
  }
  auto floatValue = std::get_if<float>(&value);
  if (floatValue){
    return serializeFloat(*floatValue);
  }
  modassert(false, "attrValToUiString not yet implemented for this type: " + print(value));
  return "";
}

std::string bindQueryMain(std::string& query, std::map<std::string, DataValue>& dataValues){
  std::string newQuery = query;
  for (auto &[key, dataValue] : dataValues){
    auto keyToReplace = std::string("$") + key;
    auto replaceIndex = newQuery.find(keyToReplace.c_str());
    if (replaceIndex == std::string::npos){
      modlog("editor", "bind query: key = " + key + ", but couldn't find a matching key");
      continue;
    }
    auto dataValueStr = attrValToUiString(dataValue.value);
    modlog("editor", "bind query: key = " + key + ", value = " + dataValueStr);
    newQuery.replace(replaceIndex, keyToReplace.size(), dataValueStr.c_str());
  }
  return newQuery;
}

AttributeValue parseSqlVec(std::string& value){
  auto parts = split(value, '?');
  modassert(parts.size() == 3 || parts.size() == 4, "parseSqlVec size must be 3 or 4");
  if (parts.size() == 4){
    return glm::vec4(std::atoi(parts.at(0).c_str()), std::atoi(parts.at(1).c_str()), std::atoi(parts.at(2).c_str()), std::atoi(parts.at(3).c_str()));
  }
  return glm::vec3(std::atoi(parts.at(0).c_str()), std::atoi(parts.at(1).c_str()), std::atoi(parts.at(2).c_str()));
}
void populateSqlData(EditorDetails& details){
  auto allQueriesObj = mainApi -> getObjectsByAttr("sql-query", std::nullopt, std::nullopt);

  for (auto queryObjId : allQueriesObj){
    auto attr = mainApi -> getGameObjectAttr(queryObjId);
    auto sqlBinding = getStrAttr(attr, "sql-binding");
    auto sqlQuery = getStrAttr(attr, "sql-query");
    auto sqlCast = getStrAttr(attr, "sql-cast");
    auto query = bindQueryMain(sqlQuery.value(), details.dataValues);
    bool valid = false;
    modlog("editor", "sql query: " + sqlQuery.value());
    modlog("editor", "sql query bind: " + query);
    auto compiledQuery = mainApi -> compileSqlQuery(query, {});
    auto sqlResult = mainApi -> executeSqlQuery(compiledQuery, &valid);
    modassert(valid, "populate sql data, invalid query");

    auto firstColInFirstRow = sqlResult.at(0).at(0);
    modlog("editor", "sql query result: " + print(sqlResult.at(0)));
    if (!sqlCast.has_value() || sqlCast.value() == "string"){
      updateStoreValue(details, sqlBinding.value(), firstColInFirstRow);
    }else{
      auto type = sqlCast.value();
      if (type == "number"){
        updateStoreValue(details, sqlBinding.value(), std::atoi(firstColInFirstRow.c_str()));
      }else if (type == "vec"){
        updateStoreValue(details, sqlBinding.value(), parseSqlVec(firstColInFirstRow));
      }else{
        modassert(false, "invalid type populateSqlData");
      }
    }
  }

}

enum DetailObjType { SLIDER, TEXT, INVALID };
DetailObjType getGameobjType(std::string name){
  if (name.at(0) == '_'){
    return SLIDER;
  }
  if (name.at(0) == ')'){
    return TEXT;
  }
  modassert(false, "getGameobjType not found");
  return INVALID;
}


float calcSlideValue(GameobjAttributes& attr, float percentage){
  float min = getFloatAttr(attr, "min").value();
  float max = getFloatAttr(attr, "max").value();
  float range = max - min;
  return min + (range * percentage);
}

float uncalcSlideValue(objid obj, float value){
  auto objattr = mainApi -> getGameObjectAttr(obj);
  float min = getFloatAttr(objattr, "min").value();
  float max = getFloatAttr(objattr, "max").value();
  float ratio = (value - min) / (max - min);
  std::cout << "uncalc slide value = " << value<< ", ratio = " <<  ratio << ", min = " << min << ", max = " << max << std::endl;
  return ratio;
}


void updateBinding(EditorDetails& details, objid id, std::string& detailBinding, std::optional<int> detailBindingIndex){
  auto hasValue = details.dataValues.find(detailBinding) != details.dataValues.end();
  modlog("editor", "details update binding: " + detailBinding + ", hasvalue = " + print(hasValue));
  if (!hasValue){
    return;
  }
  auto dataValue = details.dataValues.at(detailBinding).value;
  auto objType = getGameobjType(mainApi -> getGameObjNameForId(id).value());
  // (format #t "binding index: ~a ~a\n" bindingIndex (number? bindingIndex))
  // (format #t "type for update dataValue is: ~a, value: ~a\n" (getType dataValue) dataValue)
  if (detailBindingIndex.has_value()){
    auto vec3 = std::get_if<glm::vec3>(&dataValue);
    auto vec4 = std::get_if<glm::vec4>(&dataValue);
    if (vec3 != NULL){
      modassert(detailBindingIndex.value() <= 2, "detail binding index must be [0, 2]");
      dataValue = (*vec3)[detailBindingIndex.value()];
    }else if (vec4 != NULL){
      modassert(detailBindingIndex.value() <= 3, "detail binding index must be [0, 3]");
      dataValue = (*vec4)[detailBindingIndex.value()];
    }else{
      modassert(false, "update binding detail binding index can only be applied to vec3 or vec4");
    }
  }

  if (objType == TEXT){
    GameobjAttributes newAttr {
      .stringAttributes = { {"value", attrValToUiString(dataValue) }},
      .numAttributes = { },
      .vecAttr = { .vec3 = {}, .vec4 = {} },
    };
    mainApi -> setGameObjectAttr(id, newAttr);
  }else if (objType == SLIDER){
    auto floatDataValue = std::get_if<float>(&dataValue);
    modassert(floatDataValue, "update binding slider, not float value");
    GameobjAttributes newAttr {
      .stringAttributes = {},
      .numAttributes = { { "slideamount", uncalcSlideValue(id, *floatDataValue) } },
      .vecAttr = { .vec3 = {}, .vec4 = {} },
    };
    mainApi -> setGameObjectAttr(id, newAttr);
  }else {
    modassert(false, "update binding, invalid obj type");
  }

  //(format #t "data value: ~a\n" dataValue)
  //(format #t "update binding for: ~a\n" (gameobj-name obj))
  //(format #t "datavalues: ~a\n" dataValues)
  //(format #t "data = ~a, index = ~a\n" dataValue bindingIndex)
}

void updateBindingWithValue(EditorDetails& details, float value, GameobjAttributes& objattr){
  auto detailBinding = getStrAttr(objattr, "details-binding");
  auto detailBindingIndex = optionalInt(getFloatAttr(objattr, "details-binding-index"));
  if (detailBinding.has_value()){
    auto updateValue = getUpdatedValue(details, detailBinding.value(), detailBindingIndex, value);
    if (updateValue.has_value()){
      updateStoreValueModified(details, updateValue.value().key, updateValue.value().value, true);
    }
  }
}


bool controlEnabled(EditorDetails& details, GameobjAttributes& gameobjAttr){
  auto isEnabledBinding = getStrAttr(gameobjAttr, "details-enable-binding");
  auto isEnabledBindingOff = getStrAttr(gameobjAttr, "details-enable-binding-off");
  if (isEnabledBinding.has_value() && isEnabledBindingOff.has_value()){
    auto bindingValue = getDataValue(details, isEnabledBinding.value());
    auto bindingValueStr = std::get_if<std::string>(&bindingValue.value());
    modassert(bindingValueStr, "control enabled - binding value must be string type");
    return *bindingValueStr != isEnabledBindingOff.value();
  }
  return true;
}

void updateToggleBinding(EditorDetails& details, objid id, std::string& detailBinding, std::optional<int> detailBindingIndex, std::optional<AttributeValue> bindingOn){
  auto hasValue = details.dataValues.find(detailBinding) != details.dataValues.end();
  modlog("editor", "details update binding: " + detailBinding + ", hasvalue = " + print(hasValue));
  if (!hasValue){
    return;
  }
  auto toggleEnableText = details.dataValues.at(detailBinding).value;

  auto enableValueStr = bindingOn.has_value() ? bindingOn.value() : "enabled";
  auto enableValue = aboutEqual(enableValueStr, toggleEnableText);
  std::cout << "setting detailbinding: " << detailBinding << " equal?: " << enableValue << " enableValueStr = " << print(enableValueStr) << ", toggleEnableText = " << print(toggleEnableText) << std::endl;
  auto gameobjAttr = mainApi -> getGameObjectAttr(id);
  auto isEnabled = controlEnabled(details, gameobjAttr);
  auto onOffColor = enableValue ? glm::vec4(0.3f, 0.3f, 0.6f, 1.f) : glm::vec4(1.f, 1.f, 1.f, 1.f);
  GameobjAttributes newAttr {
    .stringAttributes = {{ "state", enableValue ? "on" : "off" }},
    .numAttributes = {  },
    .vecAttr = { .vec3 = {}, .vec4 = { {"tint", isEnabled ? onOffColor : glm::vec4(0.4f, 0.4f, 0.4f, 1.f) }} },
  };
  mainApi -> setGameObjectAttr(id, newAttr);
}


AttributeValue specialTransform(std::string& key, AttributeValue& value){
  if (key == "gameobj:texturetiling" || key == "gameobj:texturesize" || key == "gameobj:textureoffset"){
    auto strValue = std::get_if<std::string>(&value);
    modassert(strValue, "texturetiling str must be a string");
    auto vec2 = parseVec2(*strValue);
    return glm::vec3(vec2.x, vec2.y, 0.f);
  }
  return value;
}

AttributeValue inverseSpecialTransform(std::string key, AttributeValue& value){
  if (key == "gameobj:texturetiling" || key == "gameobj:texturesize" || key == "gameobj:textureoffset"){
    auto vec3Value = std::get_if<glm::vec3>(&value);
    modassert(vec3Value, "texturetiling str must be a vec3");
    return serializeVec(glm::vec2(vec3Value -> x, vec3Value -> y));
  }
  return value;
}

bool getBoolValue(EditorDetails& details, std::string key, std::string on, std::string off, bool defaultValue){
  auto oldValueAttr = getDataValue(details, key);
  if (!oldValueAttr.has_value()){
    return defaultValue;
  }
  auto strValue = std::get_if<std::string>(&oldValueAttr.value());
  if (!strValue){
    return defaultValue;
  }
  if (*strValue == on){
    return true;
  }
  if (*strValue == off){
    return false;
  }
  return defaultValue;
}

void refillStore(EditorDetails& details, objid gameobj){
   auto oldAutoSaveValue = getBoolValue(details, "auto-save", "enabled", "disabled", false);
   auto oldPlayModeValue = getBoolValue(details, "play-mode-on", "on", "off", false);

   clearStore(details);
   updateStoreValue(details, "object_name", mainApi -> getGameObjNameForId(gameobj).value());
   auto gameobjAttr = mainApi -> getGameObjectAttr(gameobj);
   auto allKeysAndAttr = allKeysAndAttributes(gameobjAttr);
   for (auto &[attribute, value] : allKeysAndAttr){
      auto keyname = std::string("gameobj:" + attribute);
      updateStoreValue(details, keyname, specialTransform(keyname, value));
   }
   updateStoreValue(details, "meta-numscenes", mainApi -> listScenes({}).size());
   updateStoreValue(details, "runtime-id", std::to_string(gameobj));
   updateStoreValue(details, "runtime-sceneid", std::to_string(mainApi -> listSceneId(gameobj)));
   updateStoreValue(details, "play-mode-on", oldPlayModeValue ? "on" : "off");
   updateStoreValue(details, "pause-mode-on", details.pauseModeEnabled ? "on" : "off");
   updateStoreValue(details, "auto-save", oldAutoSaveValue ? "enabled" : "disabled");
   for (auto &state : mainApi -> getWorldState()){
      auto keyname =  std::string("world:") + state.object + ":" + state.attribute;
      updateStoreValue(details, keyname, specialTransform(keyname, state.value));
   }
   populateSqlData(details);
}

void enforceLayouts(EditorDetails& details){
  auto id = mainApi -> getGameObjectByName("(test_panel", details.editorSceneId, true).value();
  mainApi -> enforceLayout(id);
}
void populateData(EditorDetails& details){
  for (auto &bindingElementId : mainApi -> getObjectsByAttr("details-binding", std::nullopt, std::nullopt)){
    auto attr = mainApi -> getGameObjectAttr(bindingElementId);
    auto detailBindingIndex = getFloatAttr(attr, "details-binding-index");
    auto bindingTypeValue = getStrAttr(attr, "details-binding");
    updateBinding(details, bindingElementId, bindingTypeValue.value(), optionalInt(detailBindingIndex));
  }

  for (auto &bindingElementId : mainApi -> getObjectsByAttr("details-binding-toggle", std::nullopt, std::nullopt)){
    auto attr = mainApi -> getGameObjectAttr(bindingElementId);
    auto detailBindingIndex = getFloatAttr(attr, "details-binding-index");
    auto detailBinding = getStrAttr(attr, "details-binding-toggle");
    auto bindingTypeValue = getStrAttr(attr, "details-binding");
    auto bindingOn = getAttr(attr, "details-binding-on");
    updateToggleBinding(details, bindingElementId, detailBinding.value(), optionalInt(detailBindingIndex), bindingOn);
  }
  enforceLayouts(details);
}


struct ParsedDetailAttr {
  std::optional<std::string> prefixAttr;
  std::string attr;
};
ParsedDetailAttr parseDetailAttr(std::string value){
  auto parts = split(value, ':');
  auto restString = join(subvector(parts, 1, parts.size()), ':');
  return ParsedDetailAttr { .prefixAttr = parts.size() >= 2 ? std::optional<std::string>(parts.at(0)) : std::nullopt, .attr = restString };
}

void submitWorldAttr(std::vector<KeyAndAttribute>& worldAttrs){
  std::vector<ObjectValue> worldState;
  for (auto &keyAttr : worldAttrs){
    auto prefix = split(keyAttr.key, ':');
    modassert(prefix.size () == 2, "prefix size is incorrect");
    auto name = prefix.at(0);
    auto attribute = prefix.at(1);
    auto value = keyAttr.value;

    std::cout << "setting world state: name = " << name << ", attr = " << attribute << ", value = " << print(value) << std::endl;
    worldState.push_back(ObjectValue {
      .object = name,
      .attribute = attribute,
      .value = value,
    });
  }
  mainApi -> setWorldState(worldState);
}


void submitSqlUpdates(EditorDetails& details){
  auto allQueriesObj = mainApi -> getObjectsByAttr("sql-query", std::nullopt, std::nullopt);
  for (auto queryObjId : allQueriesObj){
    auto attr = mainApi -> getGameObjectAttr(queryObjId);
    auto sqlBinding = getStrAttr(attr, "sql-binding");
    auto sqlUpdate = getStrAttr(attr, "sql-update");
    auto sqlCast = getStrAttr(attr, "sql-cast");
    
    modlog("editor", "details update - " + sqlUpdate.value());
    auto query = bindQueryMain(sqlUpdate.value(), details.dataValues);
    modlog("editor", "details update binded - " + query);

    bool valid = false;
    auto compiledQuery = mainApi -> compileSqlQuery(query, {});
    auto sqlResult = mainApi -> executeSqlQuery(compiledQuery, &valid);
    modassert(valid, "populate sql data, invalid query");
  }
}

bool managedObjExists(EditorDetails& details){
  return details.managedObj.has_value() && mainApi -> getGameObjNameForId(details.managedObj.value());
}

void submitData(EditorDetails& details){
  std::vector<KeyAndAttribute> gameobjAttrs;
  std::vector<KeyAndAttribute> worldAttrs;
  if (managedObjExists(details)){
    for (auto &[key, value] : details.dataValues){
      if (!value.modified){
        continue;
      }
      auto parsedAttr = parseDetailAttr(key);
      if (parsedAttr.prefixAttr.has_value()){
        if(parsedAttr.prefixAttr.value() == "gameobj"){
          gameobjAttrs.push_back(KeyAndAttribute {
            .key = parsedAttr.attr,
            .value = inverseSpecialTransform(key, value.value),
          });
        }else if (parsedAttr.prefixAttr.value() == "world"){
          worldAttrs.push_back(KeyAndAttribute {
            .key = parsedAttr.attr,
            .value = inverseSpecialTransform(key, value.value),
          });
        }else{
          modassert(false, "submit data, unknown attr: " + parsedAttr.prefixAttr.value());
        }
      }
    }

    auto objAttrs = gameobjAttrFromAttributes(gameobjAttrs);
    std::cout << "setting attr: " << print(objAttrs) << std::endl;

    mainApi -> setGameObjectAttr(details.managedObj.value(), objAttrs);

    submitWorldAttr(worldAttrs);
    submitSqlUpdates(details);
  }
}



void submitAndPopulateData(EditorDetails& details){
  submitData(details);
  populateData(details);
}

void maybeSetBinding(EditorDetails& details, GameobjAttributes& objattr){
  auto shouldSet = getStrAttr(objattr, "details-binding-set").has_value();
  auto detailBinding = getStrAttr(objattr, "details-binding-toggle");
  auto detailBindingIndex = getFloatAttr(objattr, "details-binding-index");
  auto enableValue = getAttr(objattr, "details-binding-on");

  if (shouldSet && enableValue.has_value() && detailBinding.has_value()){
    auto updatedValue = getUpdatedValue(details, detailBinding.value(), detailBindingIndex, enableValue.value());
    if (updatedValue.has_value()){
      updateStoreValueModified(details, updatedValue.value().key, updatedValue.value().value, true);
    }
  }
  submitAndPopulateData(details);
}
void maybeSetTextCursor(objid gameobj){
  auto objattr = mainApi -> getGameObjectAttr(gameobj);
  auto text = getStrAttr(objattr, "value");
  auto wrapAmount = getFloatAttr(objattr, "wrapamount");
  auto offset = getFloatAttr(objattr, "offset");
  if (text.has_value() && wrapAmount.has_value() && offset.has_value()){
    auto cursorValue = std::min(std::max(1, static_cast<int>(text.value().size())) - 1,  static_cast<int>(wrapAmount.value()) - 1 + static_cast<int>(offset.value()));
    modassert(cursorValue >= 0, "cursor value is expected to be >= 0");
    GameobjAttributes newAttr {
      .stringAttributes = { {"cursor-dir", "right"} },
      .numAttributes = { { "cursor", cursorValue }, { "cursor-highlight", 0 }},
      .vecAttr = { .vec3 = {}, .vec4 = {} },
    };
    mainApi -> setGameObjectAttr(gameobj, newAttr);
  }
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
  auto name = getAsType<std::string>(getStoreValue(details, "gameobj:map"));
  modassert(false, std::string("save heightmap not implemented, tried to save: ") + name);
}

void saveHeightmapAs(EditorDetails& details){
  auto name = getAsType<std::string>(getStoreValue(details, "heightmap:filename"));
  bool isNamed = name.size() > 0;
  if (isNamed && managedObjExists(details)){
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

void createVolume(EditorDetails& details){
  modlog("editor", "details create volume placeholder");

  GameobjAttributes attr {
    .stringAttributes = { 
       { "mesh", "../gameresources/build/primitives/walls/1-0.2-1.gltf" },
       { "trigger-switch", "someswitch" },
       { "physics", "enabled" },
    },
    .numAttributes = {},
    .vecAttr = { 
      .vec3 = {
        { "scale", glm::vec3(5.f, 5.f, 5.f) },
      }, 
      .vec4 = {
        { "tint", glm::vec4(0.f, 1.f, 0.f, 0.2f) },
      } 
    },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  mainApi -> makeObjectAttr(details.activeSceneId.value(), "volume-" + uniqueName(), attr, submodelAttributes); 
}

void createWater(EditorDetails& details){
  modlog("editor", "details create volume placeholder");
  GameobjAttributes attr {
    .stringAttributes = { 
       { "mesh", "../gameresources/build/primitives/walls/1-0.2-1.gltf" },
       { "physics", "enabled" },
       { "water", "true" },
       { "layer", "transparency" },
    },
    .numAttributes = {
      { "water-density", 10.f },
      { "water-gravity", 0.0125f },
      { "water-viscosity", 0.1f },
    },
    .vecAttr = { 
      .vec3 = {
        { "scale", glm::vec3(5.f, 5.f, 5.f) },
      }, 
      .vec4 = {
        { "tint", glm::vec4(0.f, 0.f, 1.f, 0.2f) },
      } 
    },
  };
  std::map<std::string, GameobjAttributes> submodelAttributes;
  mainApi -> makeObjectAttr(details.activeSceneId.value(), "volume-" + uniqueName(), attr, submodelAttributes); 
}

void createScene(EditorDetails& details){
  modlog("editor", "create scene placeholder\n");
  modassert(false, "create scene not yet implemented");
}
void saveScene(EditorDetails& details){
  modlog("editor", "save scene placeholder:\n");
  if (!details.activeSceneId.has_value()){
    modlog("editor", "no active scene not saving");
    return;
  }

  //play-mode", details.playModeEnabled ? "true" : "false");
  auto playMode = getDataValue(details, "play-mode-on").value();
  auto playModeStr = std::get_if<std::string>(&playMode);
  bool playModeEnabled = playModeStr && *playModeStr == "on";
  if (playModeEnabled){
    mainApi -> sendNotifyMessage("alert", "cannot save in play mode");
    return;
  }
  auto oldSceneFile  = mainApi -> listSceneFiles(details.activeSceneId.value()).at(0);
  modlog("editor", "saving scene: " + std::to_string(details.activeSceneId.value()) + " (file = " + oldSceneFile + ")");
  bool didSave = mainApi -> saveScene(false, details.activeSceneId.value(), std::nullopt);
  if (didSave){
    mainApi -> sendNotifyMessage("alert", "saved scene: " + std::to_string(details.activeSceneId.value()) + " (file = " + oldSceneFile + ")");
  }else{
    mainApi -> sendNotifyMessage("alert", "could not save scene: " + std::to_string(details.activeSceneId.value()) + " (file = " + oldSceneFile + ")");
  }
}

void reloadHud(EditorDetails& details){
  auto playerHud = getDataValue(details, "player-hud");
  mainApi -> sendNotifyMessage("reload-config:hud",  playerHud.value());
  modlog("editor", "details - reload hud request: " + print(playerHud.value()));
}

ObjectValue getWorldState(std::string object, std::string attribute){
  auto worldState = mainApi -> getWorldState();
  for (auto &objectValue : worldState){
    if(objectValue.object == "editor" && objectValue.attribute == "debugmask"){
      return objectValue;
    }
  }
  modassert(false, "get world state incorrect object and attribute");
  return ObjectValue {};
}


int maskBasedOnFloatField(EditorDetails& details, int debugValue, std::string field, int mask){
  auto soundValue = getDataValue(details, field);
  auto strSoundValue = !soundValue.has_value() ? NULL : std::get_if<std::string>(&soundValue.value());
  bool showSound = strSoundValue && (*strSoundValue == "enabled");
  if (showSound){
    debugValue |= mask;
  }else{
    debugValue &= ~(mask);
  }
  return debugValue;
}

void submitDebugVisualization(EditorDetails& details){
  modlog("editor", "details - submit debug visualization");
  auto debugMask = getWorldState("editor", "debugmask");
  auto debugMaskFloat = std::get_if<float>(&debugMask.value);
  modassert(debugMaskFloat, "debug mask is not a float");
  auto debugValue = static_cast<int>(*debugMaskFloat);

  debugValue = maskBasedOnFloatField(details, debugValue, "debug-show-cameras", 0b10);
  debugValue = maskBasedOnFloatField(details, debugValue, "debug-show-sound", 0b100);
  debugValue = maskBasedOnFloatField(details, debugValue, "debug-show-lights", 0b1000);
  modlog("editor", "debug visualization: new debug mask is: " + std::to_string(debugValue));

  mainApi -> setWorldState({
     ObjectValue {
       .object = "editor",
       .attribute = "debugmask",
       .value = debugValue,
     },
  });
}


void setManipulatorMode(std::string mode){
  mainApi -> setWorldState({ 
    ObjectValue {
      .object = "tools",
      .attribute = "manipulator-mode",
      .value = mode,
    }
  });
}
void setAxis(std::string axis){
  mainApi -> setWorldState({ 
    ObjectValue {
      .object = "tools",
      .attribute = "manipulator-axis",
      .value = axis,
    }
  }); 
}


void setPauseMode(EditorDetails& details, bool enabled){
  details.pauseModeEnabled = enabled;
  mainApi -> setWorldState({ 
    ObjectValue {
      .object = "world",
      .attribute = "paused",
      .value = details.pauseModeEnabled ? "true" : "false",
    }
  });
  mainApi -> sendNotifyMessage("alert", std::string("pause mode ") + (details.pauseModeEnabled ? "enabled" : "disabled"));
  updateStoreValue(details, "pause-mode-on", details.pauseModeEnabled ? "on" : "off");
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
  updateStoreValue(details, "play-mode-on", details.playModeEnabled ? "on" : "off");
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
  return start + static_cast<char>(key) + end;
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
  auto editableText = getStrAttr(attr, "details-editable-type");
  return editableText.has_value() && editableText.value() == type;
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
  return true;
}


void updateText(EditorDetails& details, objid obj, std::string& text, CursorUpdate& cursor, int offset, GameobjAttributes& attr){
  auto cursorIndex = cursor.index;
  auto cursorDir = cursor.newCursorDir;
  auto cursorHighlightLength = cursor.highlightLength;

  auto detailBinding = getStrAttr(attr, "details-binding");
  auto detailBindingIndex = optionalInt(getFloatAttr(attr, "details-binding-index"));
  auto detailBindingType = getStrAttr(attr, "details-editable-type");

  GameobjAttributes newAttr {
    .stringAttributes = { {"cursor-dir", cursorDir}, { "value", text } },
    .numAttributes = { {"offset", offset }, { "cursor", cursorIndex}, { "cursor-highlight", cursorHighlightLength} },
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };

  modlog("editor", "details setting " + mainApi -> getGameObjNameForId(obj).value() + " text with: " + text);
  mainApi -> setGameObjectAttr(obj, newAttr);
  if (detailBinding.has_value()){
    auto oldValue = getDataValue(details, detailBinding.value()).value();
    auto updateValue = getUpdatedValue(details, detailBinding.value(), detailBindingIndex, uiStringToAttrVal(text, oldValue, detailBindingIndex));
    modlog("editor", "details warning: no value for binding: " + detailBinding.value());
    updateStoreValueModified(details, updateValue.value().key, updateValue.value().value, true);
  }
}

bool uiDisabled(EditorDetails& details){
  return !details.managedObj.has_value();
}

void processFocusedElement(EditorDetails& details, int key){
  if (uiDisabled(details)){
    return;
  }
  if (details.focusedElement.has_value()){
    auto attr = mainApi -> getGameObjectAttr(details.focusedElement.value());
    if (shouldUpdateText(attr)){
      modlog("editor", "should update: " + std::to_string(details.focusedElement.value()));
      auto cursorIndex = optionalInt(getFloatAttr(attr, "cursor")).value();
      auto oldCursorDir = getStrAttr(attr, "cursor-dir").value();
      auto oldHighlight = optionalInt(getFloatAttr(attr, "cursor-highlight")).value();
      auto offsetIndex = optionalInt(getFloatAttr(attr, "offset")).value();
      auto updateType = getUpdateType(key);
      auto wrapAmount = optionalInt(getFloatAttr(attr, "wrapamount")).value();
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

bool isManagedText(GameobjAttributes& attr, std::string objname){
  return shouldUpdateText(attr) && objname.at(0) == ')';
}


void setManagedSelectionMode(EditorDetails& details, objid obj){
  details.managedSelectionMode = obj;
  modlog("editor", std::string("details - set managed selection mode: ") + std::to_string(obj));  
}
void finishManagedSelectionMode(EditorDetails& details, objid selectedObj){
  modlog("editor", "details - finished managed selection mode");  
  auto name = mainApi -> getGameObjNameForId(selectedObj).value();
  CursorUpdate cursor { .index = -1, .highlightLength = 0, .newCursorDir = "left" };
  auto attr = mainApi -> getGameObjectAttr(details.managedSelectionMode.value());
  updateText(details, details.managedSelectionMode.value(), name, cursor, 0, attr);
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
      modlog("editor", "details reselection");
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
        details -> managedObj = index;
        refillStore(*details, index);
        populateData(*details);
      }

      if (!uiDisabled(*details)){
        maybeSetBinding(*details, attr);
        if (managedText){
          maybeSetTextCursor(index);
        }
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

void toggleButtonBinding(EditorDetails& details, objid buttonId, bool on){
  auto attr = mainApi -> getGameObjectAttr(buttonId);
  auto detailsBinding = getStrAttr(attr, "details-binding-toggle");
  auto onValue = getStrAttr(attr, "details-binding-on");
  auto offValue = getStrAttr(attr, "details-binding-off");
  modlog("editor", std::string("details: on = ") + (onValue.has_value() ? onValue.value() : "novalue") + ", off = " + (offValue.has_value() ? offValue.value() : "novalue"));
  if (detailsBinding.has_value() && on && onValue.has_value()){
    updateStoreValueModified(details, detailsBinding.value(), onValue.value(), true);
  }
  if (detailsBinding.has_value() && !on && offValue.has_value()){
    updateStoreValueModified(details, detailsBinding.value(), offValue.value(), true);
  }
}

void handleActiveScene(EditorDetails& details, objid sceneId, GameobjAttributes& attr){
  auto layerAttr = getStrAttr(attr, "layer");
  if (layerAttr.has_value() && layerAttr.value() != "basicui"){
    details.activeSceneId = sceneId;
    mainApi -> sendNotifyMessage("active-scene-id", std::to_string(sceneId));
  }
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
  { "create-volume", createVolume },
  { "create-water", createWater },
  { "set-transform-mode", [](EditorDetails&) -> void { setManipulatorMode("translate"); }},
  { "set-scale-mode", [](EditorDetails&) -> void { setManipulatorMode("scale"); }},
  { "set-rotate-mode", [](EditorDetails&) -> void { setManipulatorMode("rotate"); }},
  { "toggle-play-mode", togglePlayMode },
  { "toggle-pause-mode", togglePauseMode },
  { "set-axis-x", [](EditorDetails&) -> void { setAxis("x"); }},
  { "set-axis-y", [](EditorDetails&) -> void { setAxis("y"); }},
  { "set-axis-z", [](EditorDetails&) -> void { setAxis("z"); }},
  { "copy-object", [](EditorDetails&) -> void { mainApi -> sendNotifyMessage("copy-object", "true"); }},
  { "reload-hud", reloadHud },
  { "submit-debug-vis", submitDebugVisualization },
  { "create-scene", createScene },
  { "save-scene", saveScene },
};

void maybePerformAction(EditorDetails& details, GameobjAttributes& objattr){
  if (!details.activeSceneId.has_value()){
    return;
  }
  auto attrAction = getStrAttr(objattr, "details-action");
  modlog("editor", std::string("details attr actions: ") + (attrAction.has_value() ? attrAction.value() : "no actions"));
  if (attrAction.has_value() && controlEnabled(details, objattr)){
    modlog("editor", "maybe perform action called: " + attrAction.value());
    if (buttonToAction.find(attrAction.value()) != buttonToAction.end()){
      auto action = buttonToAction.at(attrAction.value());
      action(details);
    }
  }
}


void onSlide(EditorDetails& details, objid id, float slideAmount, GameobjAttributes& attr){
  auto value = calcSlideValue(attr, slideAmount);
  modlog("editor", "on slide: id = " + std::to_string(id) + ", slideamount = " + std::to_string(slideAmount) + ", new value = " + std::to_string(value));
  updateBindingWithValue(details, value, attr);
  submitAndPopulateData(details);
}

void setInterval(objid id, float delayTimeMs, void* data, std::function<void(void* data)> fn){
  mainApi -> schedule(id, delayTimeMs, data, [fn, data, delayTimeMs, id](void*) -> void {
    fn(data);
    setInterval(id, delayTimeMs, data, fn);
  });
} 


CScriptBinding cscriptDetailsBinding(CustomApiBindings& api){
  auto binding = createCScriptBinding("native/details", api);

  binding.create = [](std::string scriptname, objid id, objid sceneId, bool isServer, bool isFreeScript) -> void* {
  	EditorDetails* details = new EditorDetails;
    details -> editorSceneId = sceneId;
    details -> activeSceneId = std::nullopt;
    details -> hoveredObj = std::nullopt;
    details -> focusedElement = std::nullopt;
    details -> managedObj = std::nullopt;
    details -> playModeEnabled = false;
    details -> pauseModeEnabled = true;
    details -> managedSelectionMode = std::nullopt;
    details -> dataValues = {};
    details -> ctrlHeld = false;


    setInterval(id, 5000, NULL, [details](void*) -> void {
      modlog("editor", "save scene");
      auto autoSaveData = getDataValue(*details, "auto-save");
      if (!autoSaveData.has_value()){
        return;
      }
      auto autoSave = autoSaveData.value();
      auto autoSaveStr = std::get_if<std::string>(&autoSave);
      modassert(autoSaveStr, "autosave not a string");
      if (*autoSaveStr == "enabled"){
        saveScene(*details);
      }
    });

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
      if (uiDisabled(*details)){ return; }
      updateDialogValues("load-sound", value);
      submitAndPopulateData(*details);
    }else if (topic == "explorer-heightmap-brush-final"){
      if (uiDisabled(*details)){ return; }
      updateDialogValues("load-heightmap-brush", value);
      submitAndPopulateData(*details);
    }else if (topic == "explorer-heightmap-final"){
      if (uiDisabled(*details)){ return; }
      updateDialogValues("load-heightmap", value);
      submitAndPopulateData(*details);
    }else if (topic == "active-scene-id"){
      auto strValue = std::get_if<std::string>(&value);
      modassert(strValue, "active-scene-id: str value is null");
      details -> activeSceneId = std::atoi(strValue -> c_str());
    }else if (topic == "editor-button-on"){
      if (uiDisabled(*details)){ return; }
      auto strValue = std::get_if<std::string>(&value);
      modassert(strValue, "editor-button-on: str value is null");
      toggleButtonBinding(*details, std::atoi(strValue -> c_str()), true);
      submitAndPopulateData(*details);
    }else if (topic == "editor-button-off"){
      if (uiDisabled(*details)){ return; }
      auto strValue = std::get_if<std::string>(&value);
      modassert(strValue, "editor-button-off: str value is null");
      toggleButtonBinding(*details, std::atoi(strValue -> c_str()), false);
      submitAndPopulateData(*details);
    }else if (topic == "details-editable-slide"){
      if (uiDisabled(*details)){ return; }
      auto strValue = std::get_if<std::string>(&value);
      modassert(strValue, "details-editable-slide: str value is null");
      auto sliderId = std::atoi(strValue -> c_str());
      auto attr = mainApi -> getGameObjectAttr(sliderId);
      auto slideAmount = getFloatAttr(attr, "slideamount");
      onSlide(*details, sliderId, slideAmount.value(), attr);
      // (format #t "slide: ~a\n" (onSlide (getSlidePercentage (string->number value))))
    }
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

    if (key == 341){
      if (action == 1){
        details -> ctrlHeld = true;
      }else {
        details -> ctrlHeld = false;
      }
    }

    std::cout << "Key is: " << key << std::endl;
    if (details -> ctrlHeld && key == 'S' && action == 1){
      saveScene(*details);
    }
  };

  binding.onKeyCharCallback = [](int32_t id, void* data, unsigned int key) -> void {
    EditorDetails* details = static_cast<EditorDetails*>(data);
    if (key == ','){
      prettyPrint(details -> dataValues);
    }
    if (!isControlKey(key)){
      processFocusedElement(*details, key);
    }
    if (isSubmitKey(key)){
      submitAndPopulateData(*details);
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
    EditorDetails* details = static_cast<EditorDetails*>(data);
    if(!mainApi -> getGameObjNameForId(index).has_value()){
      return;
    }
    auto attr = mainApi -> getGameObjectAttr(index);
    handleActiveScene(*details, mainApi -> listSceneId(index), attr);  // pull this into a seperate script, don't like the idea of the editor managing this 
    onDetailsObjectSelected(id, data, index);
  };
  binding.onObjectUnselected = [](int32_t id, void* data) -> void {
    EditorDetails* details = static_cast<EditorDetails*>(data);
    unsetFocused(*details);
  };

  binding.onMouseCallback = [](objid id, void* data, int button, int action, int mods) -> void {
    EditorDetails* details = static_cast<EditorDetails*>(data);
    modlog("editor", "on mouse callback: button: " + std::to_string(button) + ", action: " + std::to_string(action));
    if (!details -> managedObj.has_value() || !mainApi -> getGameObjNameForId(details -> managedObj.value()).has_value()){
      return;
    }
    if (!details -> hoveredObj.has_value() || !mainApi -> getGameObjNameForId(details -> hoveredObj.value()).has_value()){
      return;
    }
    auto objSceneId =  mainApi -> listSceneId(details -> hoveredObj.value());
    if (objSceneId != mainApi -> listSceneId(id)){
      return;
    }
    if ((button == 0) && (action == 0)){
      auto attr = mainApi -> getGameObjectAttr(details -> hoveredObj.value());
      maybePerformAction(*details, attr);
      submitAndPopulateData(*details);
    }

    if ((button == 1) && (action == 0) && details -> managedSelectionMode.has_value()){
      finishManagedSelectionMode(*details, details -> hoveredObj.value());
      submitAndPopulateData(*details);
    }
  };

  return binding;
}
