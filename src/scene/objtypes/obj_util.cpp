#include "./obj_util.h"

void addSerializeCommon(std::vector<std::pair<std::string, std::string>>& pairs, GameObjectUICommon& common){
  if (common.onFocus != ""){
    pairs.push_back(std::pair<std::string, std::string>("focus", common.onFocus));
  }
  if (common.onBlur != ""){
    pairs.push_back(std::pair<std::string, std::string>("blur", common.onBlur));
  }
}

void setTextureInfo(GameobjAttributes& attr, std::function<Texture(std::string)> ensureTextureLoaded, TextureInformation& info){
  attrSet(attr, &info.textureoffset, glm::vec2(0.f, 0.f), "textureoffset");
  attrSet(attr, &info.texturetiling, glm::vec2(1.f, 1.f), "texturetiling");
  attrSet(attr, &info.texturesize, glm::vec2(1.f, 1.f), "texturesize");
  attrSet(attr, &info.textureOverloadName, "", "texture");
  int textureOverloadId = info.textureOverloadName == "" ? -1 : ensureTextureLoaded(info.textureOverloadName).textureId;
  info.textureOverloadId = textureOverloadId;
}

void addSerializedTextureInformation(std::vector<std::pair<std::string, std::string>>& pairs, TextureInformation& texture){
  if (texture.textureoffset.x != 0.f && texture.textureoffset.y != 0.f){
    pairs.push_back(std::pair<std::string, std::string>("textureoffset", serializeVec(texture.textureoffset)));
  }
  if (texture.textureOverloadName != ""){
    pairs.push_back(std::pair<std::string, std::string>("texture", texture.textureOverloadName));
  }
  if (texture.texturesize.x != 1.f && texture.texturesize.y != 1.f){
    pairs.push_back(std::pair<std::string, std::string>("texturesize", serializeVec(texture.texturesize)));
  }
}

void setTextureAttributes(TextureInformation& info, GameobjAttributes& attr, ObjectSetAttribUtil& util){
  if (attr.stringAttributes.find("textureoffset") != attr.stringAttributes.end()){
    //std::cout << "setting texture offset" << std::endl;
    info.textureoffset = parseVec2(attr.stringAttributes.at("textureoffset"));
  }
  if (attr.stringAttributes.find("texture") != attr.stringAttributes.end()){
    util.releaseTexture(info.textureOverloadId);
    auto textureOverloadName = attr.stringAttributes.at("texture");
    int textureOverloadId = util.ensureTextureLoaded(textureOverloadName).textureId;
    info.textureOverloadName = textureOverloadName;
    info.textureOverloadId = textureOverloadId;
  }
}

void attrSet(GameobjAttributes& attr, bool* value, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    *value = attr.stringAttributes.at(field) == "enabled";
  }
}
void attrSet(GameobjAttributes& attr, bool* value, bool defaultValue, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    *value = attr.stringAttributes.at(field) == "enabled";
  }else{
    *value = defaultValue;
  }
}

void attrSet(GameobjAttributes& attr, bool* _value, const char* onString, const char* offString, const char* field, bool strict){
if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
     auto value = attr.stringAttributes.at(field);
     if (value == onString){
        *_value = true;
     }else if (value == offString){
        *_value = false;
     }else if (strict){
        modassert(false, "invalid on/off string");
     }
  }
}

void attrSet(GameobjAttributes& attr, std::string* value, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    *value = attr.stringAttributes.at(field);
  }
}
void attrSet(GameobjAttributes& attr, std::string* value, std::string defaultValue, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    *value = attr.stringAttributes.at(field);
  }else{
    *value = defaultValue;
  } 
}
void attrSetRequired(GameobjAttributes& attr, std::string* _value, const char* field){
  *_value = attr.stringAttributes.at(field);
}
void attrSet(GameobjAttributes& attr, float* value, const char* field){
  if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *value = attr.numAttributes.at(field);
  }
}
void attrSet(GameobjAttributes& attr, float* _value, float defaultValue, const char* field){
  if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *_value = attr.numAttributes.at(field);
  }else{
    *_value = defaultValue;
  }
}

void attrSet(GameobjAttributes& attr, float* _value, bool* _hasValue, float defaultValue, const char* field){
  if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *_value = attr.numAttributes.at(field);
    if (_hasValue != NULL){
      *_hasValue = true;  
    }
  }else{
    *_value = defaultValue;
    if (_hasValue != NULL){
      *_hasValue = false;
    }
  }
}

void attrSet(GameobjAttributes& attr, unsigned int* value, const char* field){
  if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *value = static_cast<unsigned int>(attr.numAttributes.at(field));
  }
}
void attrSet(GameobjAttributes& attr, unsigned int* value, unsigned int defaultValue, const char* field){
  if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *value = static_cast<unsigned int>(attr.numAttributes.at(field));
  }else{
    *value = defaultValue;
  }
}



void attrSet(GameobjAttributes& attr, int* _value, const char* field){
  if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *_value = static_cast<int>(attr.numAttributes.at(field));
  }
}

void attrSet(GameobjAttributes& attr, int* _value, int defaultValue, const char* field){
  if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *_value = static_cast<int>(attr.numAttributes.at(field));
  }else{
    *_value = defaultValue;
  }
}

void attrSet(GameobjAttributes& attr, glm::vec3* _value, bool* _hasValue, const char* field){
  if (attr.vecAttr.vec3.find(field) != attr.vecAttr.vec3.end()){
    *_value = attr.vecAttr.vec3.at(field);
    if (_hasValue != NULL){
      *_hasValue = true;
    }
  }
}

void attrSet(GameobjAttributes& attr, glm::vec3* _value, glm::vec3 defaultValue, const char* field){
  if (attr.vecAttr.vec3.find(field) != attr.vecAttr.vec3.end()){
    *_value = attr.vecAttr.vec3.at(field);
  }else{
    *_value = defaultValue;
  }
}
void attrSet(GameobjAttributes& attr, glm::vec3* _value, bool* _hasValue, glm::vec3 defaultValue, const char* field){
  if (attr.vecAttr.vec3.find(field) != attr.vecAttr.vec3.end()){
    *_value = attr.vecAttr.vec3.at(field);
    if (_hasValue != NULL){
      *_hasValue = true;
    }
  }else{
    *_value = defaultValue;
    if (_hasValue != NULL){
      *_hasValue = false;
    }
  }  
}


void attrSet(GameobjAttributes& attr, glm::vec4* _value, const char* field){
  if (attr.vecAttr.vec4.find(field) != attr.vecAttr.vec4.end()){
    *_value = attr.vecAttr.vec4.at(field);
  }
}

void attrSet(GameobjAttributes& attr, glm::vec4* _value, bool* _hasValue, const char* field){
  if (attr.vecAttr.vec4.find(field) != attr.vecAttr.vec4.end()){
    *_value = attr.vecAttr.vec4.at(field);
    if (_hasValue != NULL){
      *_hasValue = true;
    }
  }
}

void attrSet(GameobjAttributes& attr, glm::vec4* _value, glm::vec4 defaultValue, const char* field){
  if (attr.vecAttr.vec4.find(field) != attr.vecAttr.vec4.end()){
    *_value = attr.vecAttr.vec4.at(field);
  }else{
    *_value = defaultValue;
  }
}

void attrSet(GameobjAttributes& attr, glm::vec4* _value, bool* _hasValue, glm::vec4 defaultValue, const char* field){
  if (attr.vecAttr.vec4.find(field) != attr.vecAttr.vec4.end()){
    *_value = attr.vecAttr.vec4.at(field);
    if (_hasValue != NULL){
      *_hasValue = true;
    }
  }else{
    *_value = defaultValue;
    if (_hasValue != NULL){
      *_hasValue = false;
    }
  }  
}

void attrSet(GameobjAttributes& attr, glm::vec2* _value, glm::vec2 defaultValue, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    *_value = parseVec2(attr.stringAttributes.at(field));

  }else{
    *_value = defaultValue;
  }
}

void attrSet(GameobjAttributes& attr, bool* _value, const char* onString, const char* offString, bool defaultValue, const char* field, bool strict){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    auto stringValue = attr.stringAttributes.at(field);
    if (stringValue == onString){
      *_value = true;
    }else if (stringValue == offString){
      *_value = false;
    }else{
      modassert(!strict, "Invalid string value in attrSet - " + stringValue);
      *_value = defaultValue;
    }

  }else{
    *_value = defaultValue;
  }
}

void attrSetLoadTexture(GameobjAttributes& attr, std::function<Texture(std::string)> ensureTextureLoaded, int* _textureId, std::string defaultTexture, const char* field){
  std::string textureToLoad = defaultTexture;
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    textureToLoad = attr.stringAttributes.at(field);
  }
  *_textureId = ensureTextureLoaded(textureToLoad).textureId;
}

void attrSetLoadTexture(GameobjAttributes& attr, std::function<Texture(std::string)> ensureTextureLoaded, int* _textureId, std::string* _textureName, std::string defaultTexture, const char* field){
  std::string textureToLoad = defaultTexture;
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    textureToLoad = attr.stringAttributes.at(field);
  }
  *_textureId = ensureTextureLoaded(textureToLoad).textureId;
  if (_textureName != NULL){
    *_textureName = textureToLoad;  
  }
}

void attrSet(GameobjAttributes& attr, int* _value, std::vector<int> enums, std::vector<std::string> enumStrings, const char* field, bool strict){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    auto value = attr.stringAttributes.at(field);
    bool foundEnum = false;
    for (int i = 0; i < enumStrings.size(); i++){
      if (enumStrings.at(i) == value){
        *_value = enums.at(i);
        foundEnum = true;
        break;
      }
    }
    modassert(foundEnum || !strict, std::string("invalid enum type for ") + field + " - " + value);
  }
}

void attrSet(GameobjAttributes& attr, int* _value, std::vector<int> enums, std::vector<std::string> enumStrings, int defaultValue, const char* field, bool strict){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    auto value = attr.stringAttributes.at(field);
    bool foundEnum = false;
    for (int i = 0; i < enumStrings.size(); i++){
      if (enumStrings.at(i) == value){
        *_value = enums.at(i);
        foundEnum = true;
        break;
      }
    }
    modassert(foundEnum || !strict, std::string("invalid enum type for ") + field + " - " + value);
    if (!foundEnum){
      *_value = defaultValue;
    }
  }else{
    *_value = defaultValue;
  }
}

std::string enumStringFromEnumValue(int value, std::vector<int>& enums, std::vector<std::string>& enumStrings){
  for (int i = 0; i < enums.size(); i++){
    if (value == enums.at(i)){
      return enumStrings.at(i);
    }
  }
  modassert(false, "invalid enum value: " + std::to_string(value));
  return "";
}


void createAutoSerialize(char* structAddress, AutoSerialize& value, GameobjAttributes& attr, ObjectTypeUtil& util){
  AutoSerializeBool* boolValue = std::get_if<AutoSerializeBool>(&value);
  if (boolValue != NULL){
    bool* address = (bool*)(((char*)structAddress) + boolValue -> structOffset);
    attrSet(attr, address, boolValue -> onString, boolValue -> offString, boolValue -> defaultValue, boolValue -> field, true);
    return;
  }

  AutoSerializeRequiredString* strValueRequired = std::get_if<AutoSerializeRequiredString>(&value);
  if (strValueRequired != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValueRequired -> structOffset);
    modassert(attr.stringAttributes.find(strValueRequired -> field) != attr.stringAttributes.end(), std::string("auto serialize - required string field not present ") + strValueRequired -> field);
    attrSet(attr, address, strValueRequired -> field);
    return;
  }

  AutoSerializeString* strValue = std::get_if<AutoSerializeString>(&value);
  if (strValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValue -> structOffset);
    attrSet(attr, address, strValue -> defaultValue, strValue -> field);
    return;
  }

  AutoSerializeFloat* floatValue = std::get_if<AutoSerializeFloat>(&value);
  if (floatValue != NULL){
    float* address = (float*)(((char*)structAddress) + floatValue -> structOffset);
    bool* hasValueAddress = (!floatValue -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + floatValue -> structOffsetFiller.value());
    attrSet(attr, address, hasValueAddress, floatValue -> defaultValue, floatValue -> field);
    return;
  }

  AutoSerializeTextureLoader* textureLoader = std::get_if<AutoSerializeTextureLoader>(&value);
  if (textureLoader != NULL){
    int* address = (int*)(((char*)structAddress) + textureLoader -> structOffset);
    std::string* textureName = (!textureLoader -> structOffsetName.has_value()) ? NULL : (std::string*)(((char*)structAddress) + textureLoader -> structOffsetName.value());
    attrSetLoadTexture(attr, util.ensureTextureLoaded, address, textureName, textureLoader -> defaultValue, textureLoader -> field);
    return;
  }

  AutoSerializeInt* intValue = std::get_if<AutoSerializeInt>(&value);
  if (intValue != NULL){
    int* address = (int*)(((char*)structAddress) + intValue -> structOffset);
    attrSet(attr, address, intValue -> defaultValue, intValue -> field);    
    return;
  }

  AutoSerializeUInt* uintValue = std::get_if<AutoSerializeUInt>(&value);
  if (uintValue != NULL){
    uint* address = (uint*)(((char*)structAddress) + uintValue -> structOffset);
    attrSet(attr, address, uintValue -> defaultValue, uintValue -> field);
    return;
  }

  AutoSerializeVec3* vec3Value = std::get_if<AutoSerializeVec3>(&value);
  if (vec3Value != NULL){
    glm::vec3* address = (glm::vec3*)(((char*)structAddress) + vec3Value -> structOffset);
    bool* hasValueAddress = (!vec3Value -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + vec3Value -> structOffsetFiller.value());
    attrSet(attr, address, hasValueAddress, vec3Value -> defaultValue, vec3Value -> field);
    return;
  }

  AutoSerializeVec4* vec4Value = std::get_if<AutoSerializeVec4>(&value);
  if (vec4Value != NULL){
    glm::vec4* address = (glm::vec4*)(((char*)structAddress) + vec4Value -> structOffset);
    bool* hasValueAddress = (!vec4Value -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + vec4Value -> structOffsetFiller.value());
    attrSet(attr, address, hasValueAddress, vec4Value -> defaultValue, vec4Value -> field);
    return;
  }

  AutoSerializeEnums* enumsValue = std::get_if<AutoSerializeEnums>(&value);
  if (enumsValue != NULL){
    int* address = (int*)(((char*)structAddress) + enumsValue -> structOffset);
    attrSet(attr, address, enumsValue -> enums, enumsValue -> enumStrings, enumsValue -> defaultValue, enumsValue -> field, true);
    return;
  }

  modassert(false, "autoserialize type not found");
}
void createAutoSerialize(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attr, ObjectTypeUtil& util){
  for (auto &value : values){
    createAutoSerialize(structAddress, value, attr, util);
  }
}

void autoserializerSerialize(char* structAddress, AutoSerialize& value, std::vector<std::pair<std::string, std::string>>& _pairs){
  AutoSerializeBool* boolValue = std::get_if<AutoSerializeBool>(&value);
  if (boolValue != NULL){
    bool* address = (bool*)(((char*)structAddress) + boolValue -> structOffset);
    bool value = *address;
    if (value != boolValue -> defaultValue){
      _pairs.push_back({ boolValue -> field, value ? boolValue -> onString : boolValue -> offString });
    }
    return;
  }

  AutoSerializeRequiredString* strValueRequired = std::get_if<AutoSerializeRequiredString>(&value);
  if (strValueRequired != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValueRequired -> structOffset);
    _pairs.push_back({ strValueRequired -> field, *address });
    return;
  }
 
  AutoSerializeString* strValue = std::get_if<AutoSerializeString>(&value);
  if (strValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValue -> structOffset);
    if (*address != strValue -> defaultValue){
      _pairs.push_back({ strValue -> field, *address });
    }
    return;
  }
  
  AutoSerializeFloat* floatValue = std::get_if<AutoSerializeFloat>(&value);
  if (floatValue != NULL){
    float* address = (float*)(((char*)structAddress) + floatValue -> structOffset);
    if (!aboutEqual(*address, floatValue -> defaultValue)){
      std::cout << floatValue -> field << " values: " << std::to_string(*address) << ", " << floatValue -> defaultValue << std::endl;
      _pairs.push_back({ floatValue -> field, std::to_string(*address) });
    }
    return;
  }

  
  AutoSerializeTextureLoader* textureLoader = std::get_if<AutoSerializeTextureLoader>(&value);
  if (textureLoader != NULL){
    //std::string* textureName = (!textureLoader -> structOffsetName.has_value()) ? NULL : (std::string*)(((char*)structAddress) + textureLoader -> structOffsetName.value());
    //_attributes.stringAttributes[textureLoader -> field] = textureName == NULL ? "" : *textureName;
    modassert(false, "textureLoader not yet implemneted");
    return;
  }
  
  AutoSerializeInt* intValue = std::get_if<AutoSerializeInt>(&value);
  if (intValue != NULL){
    int* address = (int*)(((char*)structAddress) + intValue -> structOffset);
    if (*address != intValue -> defaultValue){
      _pairs.push_back({ intValue -> field, std::to_string(*address) });
    }
    return;
  }
  
  AutoSerializeUInt* uintValue = std::get_if<AutoSerializeUInt>(&value);
  if (uintValue != NULL){
    uint* address = (uint*)(((char*)structAddress) + uintValue -> structOffset);
    if (*address != uintValue -> defaultValue){
      _pairs.push_back({ uintValue -> field, std::to_string(*address) });
    }
    return;
  }
  
  AutoSerializeVec3* vec3Value = std::get_if<AutoSerializeVec3>(&value);
  if (vec3Value != NULL){
    glm::vec3* address = (glm::vec3*)(((char*)structAddress) + vec3Value -> structOffset);
    if (!aboutEqual(*address, vec3Value -> defaultValue)){
      _pairs.push_back({ vec3Value -> field, print(*address) });
    }
    return;
  }
  
  AutoSerializeVec4* vec4Value = std::get_if<AutoSerializeVec4>(&value);
  if (vec4Value != NULL){
    glm::vec4* address = (glm::vec4*)(((char*)structAddress) + vec4Value -> structOffset);
    if (!aboutEqual(*address, vec4Value -> defaultValue)){
      _pairs.push_back({ vec4Value -> field, print(*address) });
    }
    return;
  }
  
  AutoSerializeEnums* enumsValue = std::get_if<AutoSerializeEnums>(&value);
  if (enumsValue != NULL){
    int* address = (int*)(((char*)structAddress) + enumsValue -> structOffset);
    auto enumValue = *address;
    if (enumsValue -> defaultValue != enumValue){
      auto enumsString = enumStringFromEnumValue(enumValue, enumsValue -> enums, enumsValue -> enumStrings);
      _pairs.push_back({ enumsValue -> field, enumsString });
    }
    return;
  }

  modassert(false, "autoserialize type not yet implemented");
}

void autoserializerSerialize(char* structAddress, std::vector<AutoSerialize>& values, std::vector<std::pair<std::string, std::string>>& _pairs){
  for (auto &value : values){
    autoserializerSerialize(structAddress, value, _pairs);
  }
}

void autoserializerGetAttr(char* structAddress, AutoSerialize& value, GameobjAttributes& _attributes){
  AutoSerializeBool* boolValue = std::get_if<AutoSerializeBool>(&value);
  if (boolValue != NULL){
    bool* address = (bool*)(((char*)structAddress) + boolValue -> structOffset);
    bool value = *address;
    _attributes.stringAttributes[boolValue -> field] = value ? boolValue -> onString : boolValue -> offString;
    return;
  }


  AutoSerializeRequiredString* strValueRequired = std::get_if<AutoSerializeRequiredString>(&value);
  if (strValueRequired != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValueRequired -> structOffset);
    _attributes.stringAttributes[strValueRequired -> field] = *address;
    return;
  }
 
  AutoSerializeString* strValue = std::get_if<AutoSerializeString>(&value);
  if (strValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValue -> structOffset);
    _attributes.stringAttributes[strValue -> field] = *address;
    return;
  }
  
  AutoSerializeFloat* floatValue = std::get_if<AutoSerializeFloat>(&value);
  if (floatValue != NULL){
    float* address = (float*)(((char*)structAddress) + floatValue -> structOffset);
    _attributes.numAttributes[floatValue -> field] = *address;
    return;
  }

  
  AutoSerializeTextureLoader* textureLoader = std::get_if<AutoSerializeTextureLoader>(&value);
  if (textureLoader != NULL){
    std::string* textureName = (!textureLoader -> structOffsetName.has_value()) ? NULL : (std::string*)(((char*)structAddress) + textureLoader -> structOffsetName.value());
    _attributes.stringAttributes[textureLoader -> field] = textureName == NULL ? "" : *textureName;
    return;
  }

  AutoSerializeInt* intValue = std::get_if<AutoSerializeInt>(&value);
  if (intValue != NULL){
    int* address = (int*)(((char*)structAddress) + intValue -> structOffset);
    _attributes.numAttributes[intValue -> field] = *address;
    return;
  }
  
  AutoSerializeUInt* uintValue = std::get_if<AutoSerializeUInt>(&value);
  if (uintValue != NULL){
    uint* address = (uint*)(((char*)structAddress) + uintValue -> structOffset);
    _attributes.numAttributes[uintValue -> field] = *address;
    return;
  }
  
  AutoSerializeVec3* vec3Value = std::get_if<AutoSerializeVec3>(&value);
  if (vec3Value != NULL){
    glm::vec3* address = (glm::vec3*)(((char*)structAddress) + vec3Value -> structOffset);
    _attributes.vecAttr.vec3[vec3Value -> field] = *address;
    return;
  }
  
  AutoSerializeVec4* vec4Value = std::get_if<AutoSerializeVec4>(&value);
  if (vec4Value != NULL){
    glm::vec4* address = (glm::vec4*)(((char*)structAddress) + vec4Value -> structOffset);
    bool* hasValueAddress = (!vec4Value -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + vec4Value -> structOffsetFiller.value());
    _attributes.vecAttr.vec4[vec4Value -> field] = *address;
    return;
  }
  
  AutoSerializeEnums* enumsValue = std::get_if<AutoSerializeEnums>(&value);
  if (enumsValue != NULL){
    int* address = (int*)(((char*)structAddress) + enumsValue -> structOffset);
    auto enumValue = *address;
    _attributes.stringAttributes[enumsValue -> field] = enumStringFromEnumValue(enumValue, enumsValue -> enums, enumsValue -> enumStrings);
    return;
  }

  modassert(false, "autoserialize type not found");
}

void autoserializerGetAttr(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& _attributes){
  for (auto &value : values){
    autoserializerGetAttr(structAddress, value, _attributes);
  }
}

void autoserializerSetAttr(char* structAddress, AutoSerialize& value, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  AutoSerializeBool* boolValue = std::get_if<AutoSerializeBool>(&value);
  if (boolValue != NULL){
    bool* address = (bool*)(((char*)structAddress) + boolValue -> structOffset);
    attrSet(attributes, address, boolValue -> onString, boolValue -> offString, boolValue -> field, true);
    return;
  }


  AutoSerializeRequiredString* strValueRequired = std::get_if<AutoSerializeRequiredString>(&value);
  if (strValueRequired != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValueRequired -> structOffset);
    attrSet(attributes, address, strValueRequired -> field);
    return;
  }

  AutoSerializeString* strValue = std::get_if<AutoSerializeString>(&value);
  if (strValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValue -> structOffset);
    attrSet(attributes, address, strValue -> field);
    return;
  }

  AutoSerializeFloat* floatValue = std::get_if<AutoSerializeFloat>(&value);
  if (floatValue != NULL){
    float* address = (float*)(((char*)structAddress) + floatValue -> structOffset);
    attrSet(attributes, address, floatValue -> field);
    return;
  }


  AutoSerializeTextureLoader* textureLoader = std::get_if<AutoSerializeTextureLoader>(&value);
  if (textureLoader != NULL){
    int* address = (int*)(((char*)structAddress) + textureLoader -> structOffset);
    std::string* textureName = (!textureLoader -> structOffsetName.has_value()) ? NULL : (std::string*)(((char*)structAddress) + textureLoader -> structOffsetName.value());
    attrSetLoadTexture(attributes, util.ensureTextureLoaded, address, textureName, textureLoader -> defaultValue, textureLoader -> field);
    return;
  }

  AutoSerializeInt* intValue = std::get_if<AutoSerializeInt>(&value);
  if (intValue != NULL){
    int* address = (int*)(((char*)structAddress) + intValue -> structOffset);
    attrSet(attributes, address, intValue -> field);    
    return;
  }
  
  AutoSerializeUInt* uintValue = std::get_if<AutoSerializeUInt>(&value);
  if (uintValue != NULL){
    uint* address = (uint*)(((char*)structAddress) + uintValue -> structOffset);
    attrSet(attributes, address, uintValue -> field);
    return;
  }
//  ///*
  AutoSerializeVec3* vec3Value = std::get_if<AutoSerializeVec3>(&value);
  if (vec3Value != NULL){
    glm::vec3* address = (glm::vec3*)(((char*)structAddress) + vec3Value -> structOffset);
    bool* hasValueAddress = (!vec3Value -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + vec3Value -> structOffsetFiller.value());
    attrSet(attributes, address, hasValueAddress, vec3Value -> field);
    return;
  }

  AutoSerializeVec4* vec4Value = std::get_if<AutoSerializeVec4>(&value);
  if (vec4Value != NULL){
    glm::vec4* address = (glm::vec4*)(((char*)structAddress) + vec4Value -> structOffset);
    bool* hasValueAddress = (!vec4Value -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + vec4Value -> structOffsetFiller.value());
    attrSet(attributes, address, hasValueAddress, vec4Value -> field);
    return;
  }

  AutoSerializeEnums* enumsValue = std::get_if<AutoSerializeEnums>(&value);
  if (enumsValue != NULL){
    int* address = (int*)(((char*)structAddress) + enumsValue -> structOffset);
    attrSet(attributes, address, enumsValue -> enums, enumsValue -> enumStrings, enumsValue -> field, true);
    return;
  }

  modassert(false, "autoserialize type not found");
}

void autoserializerSetAttr(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  for (auto &value : values){
    autoserializerSetAttr(structAddress, value, attributes, util);
  }
}