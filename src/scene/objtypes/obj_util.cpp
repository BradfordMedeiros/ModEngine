#include "./obj_util.h"

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
void attrForceSet(GameobjAttributes& attr, std::string* value, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    *value = attr.stringAttributes.at(field);
  }else if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *value = serializeFloat(attr.numAttributes.at(field));
  }else if (attr.vecAttr.vec3.find(field) != attr.vecAttr.vec3.end()){
    *value = serializeVec(attr.vecAttr.vec3.at(field));
  }else if (attr.vecAttr.vec4.find(field) != attr.vecAttr.vec4.end()){
    *value = serializeVec(attr.vecAttr.vec4.at(field));
  }
}

void attrSet(GameobjAttributes& attr, std::string* value, std::string defaultValue, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    *value = attr.stringAttributes.at(field);
  }else{
    *value = defaultValue;
  } 
}

void attrForceSet(GameobjAttributes& attr, std::string* value, std::string defaultValue, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    *value = attr.stringAttributes.at(field);
  }else if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *value = serializeFloat(attr.numAttributes.at(field));
  }else if (attr.vecAttr.vec3.find(field) != attr.vecAttr.vec3.end()){
    *value = serializeVec(attr.vecAttr.vec3.at(field));
  }else if (attr.vecAttr.vec4.find(field) != attr.vecAttr.vec4.end()){
    *value = serializeVec(attr.vecAttr.vec4.at(field));
  }else{
    *value = defaultValue;
  } 
}

void attrSet(GameobjAttributes& attr, float* value, const char* field){
  if (attr.numAttributes.find(field) != attr.numAttributes.end()){
    *value = attr.numAttributes.at(field);
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

void attrSet(GameobjAttributes& attr, glm::vec2* _value, bool* _hasValue, glm::vec2 defaultValue, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    auto value = attr.stringAttributes.at(field);
    *_value = parseVec2(value);
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

void attrSet(GameobjAttributes& attr, glm::vec2* _value, bool* _hasValue, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    auto value = attr.stringAttributes.at(field);
    *_value = parseVec2(value);
    if (_hasValue != NULL){
      *_hasValue = true;
    }
  }else{
    if (_hasValue != NULL){
      *_hasValue = false;
    }
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

void attrSet(GameobjAttributes& attr, glm::quat* _value, glm::quat defaultValue, const char* field){
  if (attr.vecAttr.vec4.find(field) != attr.vecAttr.vec4.end()){
    *_value = parseQuat(attr.vecAttr.vec4.at(field));
  }else{
    *_value = defaultValue;
  }  
}
void attrSet(GameobjAttributes& attr, glm::quat* _value, const char* field){
  if (attr.vecAttr.vec4.find(field) != attr.vecAttr.vec4.end()){
    *_value = parseQuat(attr.vecAttr.vec4.at(field));
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

void attrSetLoadTextureManual(GameobjAttributes& attr, TextureLoadingData* _textureLoading, const char* field){
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    std::string textureToLoad = attr.stringAttributes.at(field);
    //std::cout << "attr set load texture manual: tex to load: " << textureToLoad << std::endl;
    //std::cout << "texture string: " << _textureLoading -> textureString << std::endl;
    //std::cout << "isloaded: " << _textureLoading -> isLoaded << std::endl;

    if (textureToLoad != _textureLoading -> textureString){
      _textureLoading -> isLoaded = false;
      _textureLoading -> textureId = -1;
      _textureLoading -> textureString = textureToLoad;
    }
  }
}

void attrSetLoadTextureManual(GameobjAttributes& attr, TextureLoadingData* _textureLoading, std::string defaultTexture, const char* field){
  std::string textureToLoad = defaultTexture;
  if (attr.stringAttributes.find(field) != attr.stringAttributes.end()){
    textureToLoad = attr.stringAttributes.at(field);
  }
  if (textureToLoad != _textureLoading -> textureString){
    _textureLoading -> isLoaded = false;
    _textureLoading -> textureId = -1;
    _textureLoading -> textureString = textureToLoad;
  }
  //std::cout << "texture to load: " << textureToLoad << " isloaded: " << _textureLoading -> isLoaded << std::endl;
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

void autoserializeHandleTextureLoading(char* structAddress, std::vector<AutoSerialize>& values, std::function<Texture(std::string)> ensureTextureLoaded, std::function<void(int)> releaseTexture){
  for (auto &value : values){
    AutoSerializeTextureLoaderManual* textureLoaderManual = std::get_if<AutoSerializeTextureLoaderManual>(&value);
    if (textureLoaderManual != NULL){
      TextureLoadingData* _textureLoading = (TextureLoadingData*)(((char*)structAddress) + textureLoaderManual -> structOffset);
      if (_textureLoading -> isLoaded){
        // do nothing
      }else if (_textureLoading -> textureString != ""){
        auto texture = ensureTextureLoaded(_textureLoading -> textureString);
        _textureLoading -> textureId = texture.textureId;
        _textureLoading -> isLoaded = true;
      }else{
        _textureLoading -> textureId = -1;
        _textureLoading -> isLoaded = true;
      }
      
      //std::cout << "texture to load: " << _textureLoading -> textureString << " isloaded: " << _textureLoading -> isLoaded << std::endl;
    }
  }
}

void createAutoSerialize(char* structAddress, AutoSerialize& value, GameobjAttributes& attr){
  AutoSerializeBool* boolValue = std::get_if<AutoSerializeBool>(&value);
  if (boolValue != NULL){
    bool* address = (bool*)(((char*)structAddress) + boolValue -> structOffset);
    attrSet(attr, address, boolValue -> onString, boolValue -> offString, boolValue -> defaultValue, boolValue -> field, true);
    return;
  }

  AutoSerializeString* strValue = std::get_if<AutoSerializeString>(&value);
  if (strValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValue -> structOffset);
    attrSet(attr, address, strValue -> defaultValue, strValue -> field);
    return;
  }

  AutoSerializeForceString* strForcedValue = std::get_if<AutoSerializeForceString>(&value);
  if (strForcedValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strForcedValue -> structOffset);
    attrForceSet(attr, address, strForcedValue -> defaultValue, strForcedValue -> field);
    return;
  }

  AutoSerializeFloat* floatValue = std::get_if<AutoSerializeFloat>(&value);
  if (floatValue != NULL){
    float* address = (float*)(((char*)structAddress) + floatValue -> structOffset);
    bool* hasValueAddress = (!floatValue -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + floatValue -> structOffsetFiller.value());
    attrSet(attr, address, hasValueAddress, floatValue -> defaultValue, floatValue -> field);
    return;
  }

  AutoSerializeTextureLoaderManual* textureLoaderManual = std::get_if<AutoSerializeTextureLoaderManual>(&value);
  if (textureLoaderManual != NULL){
    TextureLoadingData* address = (TextureLoadingData*)(((char*)structAddress) + textureLoaderManual -> structOffset);
    attrSetLoadTextureManual(attr, address, textureLoaderManual -> defaultValue, textureLoaderManual -> field);
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

  AutoSerializeVec2* vec2Value = std::get_if<AutoSerializeVec2>(&value);
  if (vec2Value != NULL){
    glm::vec2* address = (glm::vec2*)(((char*)structAddress) + vec2Value -> structOffset);
    bool* hasValueAddress = (!vec2Value -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + vec2Value -> structOffsetFiller.value());
    attrSet(attr, address, hasValueAddress, vec2Value -> defaultValue, vec2Value -> field);
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

  AutoSerializeRotation* rotValue = std::get_if<AutoSerializeRotation>(&value);
  if (rotValue != NULL){
    glm::quat* address = (glm::quat*)(((char*)structAddress) + rotValue -> structOffset);
    attrSet(attr, address, rotValue -> defaultValue, rotValue -> field);
    return;
  }

  AutoSerializeEnums* enumsValue = std::get_if<AutoSerializeEnums>(&value);
  if (enumsValue != NULL){
    int* address = (int*)(((char*)structAddress) + enumsValue -> structOffset);
    attrSet(attr, address, enumsValue -> enums, enumsValue -> enumStrings, enumsValue -> defaultValue, enumsValue -> field, true);
    return;
  }

  AutoSerializeCustom* customValue = std::get_if<AutoSerializeCustom>(&value);
  if (customValue != NULL){
    int* address = (int*)(((char*)structAddress) + customValue -> structOffset);
    if (customValue -> fieldType == ATTRIBUTE_VEC3){
      if (attr.vecAttr.vec3.find(customValue -> field) == attr.vecAttr.vec3.end()){
        customValue -> deserialize(address, NULL);
      }else{
        customValue -> deserialize(address, &(attr.vecAttr.vec3.at(customValue -> field)));
      }
    }else if (customValue -> fieldType == ATTRIBUTE_VEC4){
      if (attr.vecAttr.vec4.find(customValue -> field) == attr.vecAttr.vec4.end()){
        customValue -> deserialize(address, NULL);
      }else{
        customValue -> deserialize(address, &(attr.vecAttr.vec4.at(customValue -> field)));
      }
    }else if (customValue -> fieldType == ATTRIBUTE_STRING){
      if (attr.stringAttributes.find(customValue -> field) == attr.stringAttributes.end()){
        customValue -> deserialize(address, NULL);
      }else{
        customValue -> deserialize(address, &(attr.stringAttributes.at(customValue -> field)));
      }
    }else if (customValue -> fieldType == ATTRIBUTE_FLOAT){
      if (attr.numAttributes.find(customValue -> field) == attr.numAttributes.end()){
        customValue -> deserialize(address, NULL);
      }else{
        std::cout << "Custom value: " << customValue -> field << ", " << attr.numAttributes.at(customValue -> field) << std::endl;
        float value = attr.numAttributes.at(customValue -> field);
        customValue -> deserialize(address, &value);
      }
    }else{
      modassert(false, "custom value -> invalid field type");
    }
    return;
  }
  AutoserializeReservedField* reservedField = std::get_if<AutoserializeReservedField>(&value);
  if (reservedField != NULL){
    // do nothing
    return;
  }

  modassert(false, "autoserialize type not found");
}
void createAutoSerialize(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attr){
  for (auto &value : values){
    createAutoSerialize(structAddress, value, attr);
  }
}
void createAutoSerializeWithTextureLoading(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attr, ObjectTypeUtil& util){
  createAutoSerialize(structAddress, values, attr);
  autoserializeHandleTextureLoading(structAddress, values, util.ensureTextureLoaded, util.releaseTexture);
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
 
  AutoSerializeString* strValue = std::get_if<AutoSerializeString>(&value);
  if (strValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValue -> structOffset);
    if (*address != strValue -> defaultValue){
      _pairs.push_back({ strValue -> field, *address });
    }
    return;
  }
  
  AutoSerializeForceString* strForceValue = std::get_if<AutoSerializeForceString>(&value);
  if (strForceValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strForceValue -> structOffset);
    if (*address != strForceValue -> defaultValue){
      _pairs.push_back({ strForceValue -> field, *address });
    }
    return;
  }  

  AutoSerializeFloat* floatValue = std::get_if<AutoSerializeFloat>(&value);
  if (floatValue != NULL){
    float* address = (float*)(((char*)structAddress) + floatValue -> structOffset);
    if (!aboutEqual(*address, floatValue -> defaultValue)){
      std::cout << floatValue -> field << " values: " << std::to_string(*address) << ", " << floatValue -> defaultValue << std::endl;
      _pairs.push_back({ floatValue -> field, serializeFloat(*address) });
    }
    return;
  }

  
  AutoSerializeTextureLoaderManual* textureLoaderManual = std::get_if<AutoSerializeTextureLoaderManual>(&value);
  if (textureLoaderManual != NULL){
    TextureLoadingData* textureLoadingInfo = (TextureLoadingData*)(((char*)structAddress) + textureLoaderManual -> structOffset);
    if (textureLoadingInfo -> textureString != textureLoaderManual -> defaultValue){
      _pairs.push_back({ textureLoaderManual -> field, textureLoadingInfo -> textureString });
    }
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
  
  AutoSerializeVec2* vec2Value = std::get_if<AutoSerializeVec2>(&value);
  if (vec2Value != NULL){
    glm::vec2* address = (glm::vec2*)(((char*)structAddress) + vec2Value -> structOffset);
    if (!aboutEqual(*address, vec2Value -> defaultValue)){
      _pairs.push_back({ vec2Value -> field, serializeVec(*address) });
    }
    return;
  }

  AutoSerializeVec3* vec3Value = std::get_if<AutoSerializeVec3>(&value);
  if (vec3Value != NULL){
    glm::vec3* address = (glm::vec3*)(((char*)structAddress) + vec3Value -> structOffset);
    if (!aboutEqual(*address, vec3Value -> defaultValue)){
      _pairs.push_back({ vec3Value -> field, serializeVec(*address) });
    }
    return;
  }
  
  AutoSerializeVec4* vec4Value = std::get_if<AutoSerializeVec4>(&value);
  if (vec4Value != NULL){
    glm::vec4* address = (glm::vec4*)(((char*)structAddress) + vec4Value -> structOffset);
    if (!aboutEqual(*address, vec4Value -> defaultValue)){
      _pairs.push_back({ vec4Value -> field, serializeVec(*address) });
    }
    return;
  }

  AutoSerializeRotation* rotValue = std::get_if<AutoSerializeRotation>(&value);
  if (rotValue != NULL){
    glm::quat* address = (glm::quat*)(((char*)structAddress) + rotValue -> structOffset);
    glm::vec4 quat4Rot = serializeQuatToVec4(*address);
    if (!aboutEqual(quat4Rot, serializeQuatToVec4(rotValue -> defaultValue))){
      _pairs.push_back({ rotValue -> field, serializeVec(quat4Rot) });
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

  AutoSerializeCustom* customValue = std::get_if<AutoSerializeCustom>(&value);
  if (customValue != NULL){
    int* address = (int*)(((char*)structAddress) + customValue -> structOffset);
    auto attribute = customValue -> getAttribute(address);

    auto vec3Attr = std::get_if<glm::vec3>(&attribute);
    if (vec3Attr != NULL){
      _pairs.push_back({ customValue -> field, serializeVec(*vec3Attr) });
      return;
    }
    auto vec4Attr = std::get_if<glm::vec4>(&attribute);
    if (vec4Attr != NULL){
      _pairs.push_back({ customValue -> field, serializeVec(*vec4Attr) });
      return;
    }

    auto strAttr = std::get_if<std::string>(&attribute);
    if (strAttr != NULL){
      _pairs.push_back({ customValue -> field, *strAttr });
      return;
    }

    auto floatAttr = std::get_if<float>(&attribute);
    if (floatAttr != NULL){
      _pairs.push_back({ customValue -> field, serializeFloat(*floatAttr) });
      return;
    }

    modassert(false, "invalid get attribute custom type");
    return;
  }

  AutoserializeReservedField* reservedField = std::get_if<AutoserializeReservedField>(&value);
  if (reservedField != NULL){
    // do nothing
    return;
  }

  modassert(false, "autoserialize type not yet implemented");
}

void autoserializerSerialize(char* structAddress, std::vector<AutoSerialize>& values, std::vector<std::pair<std::string, std::string>>& _pairs){
  for (auto &value : values){
    autoserializerSerialize(structAddress, value, _pairs);
  }
}

std::optional<AutoSerialize*> getAutoserializeByField(std::vector<AutoSerialize>& values, const char* field){
  for (auto &value : values){
    if (serializerName(value) == field){
      return &value;
    }
  }
  return std::nullopt;
}


std::optional<AttributeValuePtr> autoserializerGetAttrPtr(char* structAddress, AutoSerialize& value){
  AutoSerializeBool* boolValue = std::get_if<AutoSerializeBool>(&value);
  if (boolValue != NULL){
    bool* address = (bool*)(((char*)structAddress) + boolValue -> structOffset);
    return address;
  }

  AutoSerializeString* strValue = std::get_if<AutoSerializeString>(&value);
  if (strValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValue -> structOffset);
    return address;
  }
  
  AutoSerializeForceString* strForceValue = std::get_if<AutoSerializeForceString>(&value);
  if (strForceValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strForceValue -> structOffset);
    return address;
  }
 
  AutoSerializeFloat* floatValue = std::get_if<AutoSerializeFloat>(&value);
  if (floatValue != NULL){
    //float* address = (float*)(((char*)structAddress) + floatValue -> structOffset);
    //return address;
    modassert(false, "objtypes cannot get a AutoSerializeFloat field");
    return std::nullopt;
  }
 
  AutoSerializeTextureLoaderManual* textureLoaderManual = std::get_if<AutoSerializeTextureLoaderManual>(&value);
  if (textureLoaderManual != NULL){
    //TextureLoadingData* address = (TextureLoadingData*)(((char*)structAddress) + textureLoaderManual -> structOffset);
    //return;
    modassert(false, "objtypes cannot get a AutoSerializeTextureLoaderManual field");
    return std::nullopt;
  }

  AutoSerializeInt* intValue = std::get_if<AutoSerializeInt>(&value);
  if (intValue != NULL){
    int* address = (int*)(((char*)structAddress) + intValue -> structOffset);
    return address;
  }

  AutoSerializeUInt* uintValue = std::get_if<AutoSerializeUInt>(&value);
  if (uintValue != NULL){
    uint* address = (uint*)(((char*)structAddress) + uintValue -> structOffset);
    return address;
  }

  AutoSerializeVec2* vec2Value = std::get_if<AutoSerializeVec2>(&value);
  if (vec2Value != NULL){
    glm::vec2* address = (glm::vec2*)(((char*)structAddress) + vec2Value -> structOffset);
    return address;
  }

  AutoSerializeVec3* vec3Value = std::get_if<AutoSerializeVec3>(&value);
  if (vec3Value != NULL){
    glm::vec3* address = (glm::vec3*)(((char*)structAddress) + vec3Value -> structOffset);
    return address;
  }

  AutoSerializeVec4* vec4Value = std::get_if<AutoSerializeVec4>(&value);
  if (vec4Value != NULL){
    glm::vec4* address = (glm::vec4*)(((char*)structAddress) + vec4Value -> structOffset);
    return address;
  }
  
  AutoSerializeRotation* rotValue = std::get_if<AutoSerializeRotation>(&value);
  if (rotValue != NULL){
    glm::quat* address = (glm::quat*)(((char*)structAddress) + rotValue -> structOffset);
    return address;
  }

  AutoSerializeEnums* enumsValue = std::get_if<AutoSerializeEnums>(&value);
  if (enumsValue != NULL){
    int* address = (int*)(((char*)structAddress) + enumsValue -> structOffset);
    return address;
  }

  AutoSerializeCustom* customValue = std::get_if<AutoSerializeCustom>(&value);
  if (customValue != NULL){
    modassert(false, "objtypes cannot get a AutoSerializeCustom field");
    return std::nullopt;
  }
  AutoserializeReservedField* reservedField = std::get_if<AutoserializeReservedField>(&value);
  if (reservedField != NULL){
    // do nothing
    modassert(false, "objtypes cannot get a AutoserializeReservedField field");
    return std::nullopt;
  }
  modassert(false, "autoserialize type not found");
  return std::nullopt;
}

void autoserializerGetAttr(char* structAddress, AutoSerialize& value, GameobjAttributes& _attributes){
  AutoSerializeBool* boolValue = std::get_if<AutoSerializeBool>(&value);
  if (boolValue != NULL){
    bool valueAtPr = *(getTypeFromAttr<bool>(autoserializerGetAttrPtr(structAddress, value)).value());
    _attributes.stringAttributes[boolValue -> field] = valueAtPr ? boolValue -> onString : boolValue -> offString;
    return;
  }
 
  AutoSerializeString* strValue = std::get_if<AutoSerializeString>(&value);
  if (strValue != NULL){
    std::string* address = (getTypeFromAttr<std::string>(autoserializerGetAttrPtr(structAddress, value)).value());
    _attributes.stringAttributes[strValue -> field] = *address;
    return;
  }
  AutoSerializeForceString* strForceValue = std::get_if<AutoSerializeForceString>(&value);
  if (strForceValue != NULL){
    std::string* address = (getTypeFromAttr<std::string>(autoserializerGetAttrPtr(structAddress, value)).value());
    _attributes.stringAttributes[strForceValue -> field] = *address;
    return;
  }
  
  AutoSerializeFloat* floatValue = std::get_if<AutoSerializeFloat>(&value);
  if (floatValue != NULL){
    float* address = (float*)(((char*)structAddress) + floatValue -> structOffset);
    _attributes.numAttributes[floatValue -> field] = *address;
    return;
  }

  AutoSerializeTextureLoaderManual* textureLoaderManual = std::get_if<AutoSerializeTextureLoaderManual>(&value);
  if (textureLoaderManual != NULL){
    TextureLoadingData* address = (TextureLoadingData*)(((char*)structAddress) + textureLoaderManual -> structOffset);
    _attributes.stringAttributes[textureLoaderManual -> field] = address -> textureString;
    return;
  }

  AutoSerializeInt* intValue = std::get_if<AutoSerializeInt>(&value);
  if (intValue != NULL){
    int* address = (getTypeFromAttr<int>(autoserializerGetAttrPtr(structAddress, value)).value());
    _attributes.numAttributes[intValue -> field] = *address;
    return;
  }
  
  AutoSerializeUInt* uintValue = std::get_if<AutoSerializeUInt>(&value);
  if (uintValue != NULL){
    uint* address = (getTypeFromAttr<uint>(autoserializerGetAttrPtr(structAddress, value)).value());
    _attributes.numAttributes[uintValue -> field] = *address;
    return;
  }
  
  AutoSerializeVec2* vec2Value = std::get_if<AutoSerializeVec2>(&value);
  if (vec2Value != NULL){
    glm::vec2* address = (getTypeFromAttr<glm::vec2>(autoserializerGetAttrPtr(structAddress, value)).value());
    _attributes.stringAttributes[vec2Value -> field] = serializeVec(*address);
    return;
  }

  AutoSerializeVec3* vec3Value = std::get_if<AutoSerializeVec3>(&value);
  if (vec3Value != NULL){
    glm::vec3* address = (getTypeFromAttr<glm::vec3>(autoserializerGetAttrPtr(structAddress, value)).value());
    _attributes.vecAttr.vec3[vec3Value -> field] = *address;
    return;
  }
  
  AutoSerializeVec4* vec4Value = std::get_if<AutoSerializeVec4>(&value);
  if (vec4Value != NULL){
    glm::vec4* address = (getTypeFromAttr<glm::vec4>(autoserializerGetAttrPtr(structAddress, value)).value());
    _attributes.vecAttr.vec4[vec4Value -> field] = *address;
    return;
  }

  AutoSerializeRotation* rotValue = std::get_if<AutoSerializeRotation>(&value);
  if (rotValue != NULL){
    glm::quat* address = (getTypeFromAttr<glm::quat>(autoserializerGetAttrPtr(structAddress, value)).value());
    _attributes.vecAttr.vec4[rotValue -> field] = serializeQuatToVec4(*address);
    return;
  }
  
  AutoSerializeEnums* enumsValue = std::get_if<AutoSerializeEnums>(&value);
  if (enumsValue != NULL){
    int* address = (getTypeFromAttr<int>(autoserializerGetAttrPtr(structAddress, value)).value());
    auto enumValue = *address;
    _attributes.stringAttributes[enumsValue -> field] = enumStringFromEnumValue(enumValue, enumsValue -> enums, enumsValue -> enumStrings);
    return;
  }

  AutoSerializeCustom* customValue = std::get_if<AutoSerializeCustom>(&value);
  if (customValue != NULL){
    int* address = (int*)(((char*)structAddress) + customValue -> structOffset);
    auto attribute = customValue -> getAttribute(address);

    auto vec3Attr = std::get_if<glm::vec3>(&attribute);
    if (vec3Attr != NULL){
      _attributes.vecAttr.vec3[customValue -> field] = *vec3Attr;
      return;
    }
    auto vec4Attr = std::get_if<glm::vec4>(&attribute);
    if (vec4Attr != NULL){
      _attributes.vecAttr.vec4[customValue -> field] = *vec4Attr;
      return;
    }

    auto strAttr = std::get_if<std::string>(&attribute);
    if (strAttr != NULL){
      _attributes.stringAttributes[customValue -> field] = *strAttr;
      return;
    }

    auto floatAttr = std::get_if<float>(&attribute);
    if (floatAttr != NULL){
      _attributes.numAttributes[customValue -> field] = *floatAttr;
      return;
    }

    modassert(false, "invalid get attribute custom type");
    return;
  }

  AutoserializeReservedField* reservedField = std::get_if<AutoserializeReservedField>(&value);
  if (reservedField != NULL){
    // do nothing
    return;
  }


  modassert(false, "autoserialize type not found");
}

void autoserializerGetAttr(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& _attributes){
  for (auto &value : values){
    autoserializerGetAttr(structAddress, value, _attributes);
  }
}

void autoserializerSetAttr(char* structAddress, AutoSerialize& value, GameobjAttributes& attributes){
  AutoSerializeBool* boolValue = std::get_if<AutoSerializeBool>(&value);
  if (boolValue != NULL){
    bool* address = (bool*)(((char*)structAddress) + boolValue -> structOffset);
    attrSet(attributes, address, boolValue -> onString, boolValue -> offString, boolValue -> field, true);
    return;
  }

  AutoSerializeString* strValue = std::get_if<AutoSerializeString>(&value);
  if (strValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValue -> structOffset);
    attrSet(attributes, address, strValue -> field);
    return;
  }

  AutoSerializeForceString* strForcedValue = std::get_if<AutoSerializeForceString>(&value);
  if (strForcedValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strForcedValue -> structOffset);
    attrForceSet(attributes, address, strForcedValue -> field);
    return;
  }
  

  AutoSerializeFloat* floatValue = std::get_if<AutoSerializeFloat>(&value);
  if (floatValue != NULL){
    float* address = (float*)(((char*)structAddress) + floatValue -> structOffset);
    attrSet(attributes, address, floatValue -> field);
    return;
  }

  AutoSerializeTextureLoaderManual* textureLoaderManual = std::get_if<AutoSerializeTextureLoaderManual>(&value);
  if (textureLoaderManual != NULL){
    TextureLoadingData* address = (TextureLoadingData*)(((char*)structAddress) + textureLoaderManual -> structOffset);
    attrSetLoadTextureManual(attributes, address, textureLoaderManual -> field);
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

  AutoSerializeVec2* vec2Value = std::get_if<AutoSerializeVec2>(&value);
  if (vec2Value != NULL){
    glm::vec2* address = (glm::vec2*)(((char*)structAddress) + vec2Value -> structOffset);
    bool* hasValueAddress = (!vec2Value -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + vec2Value -> structOffsetFiller.value());
    attrSet(attributes, address, hasValueAddress, vec2Value -> field);
    return;
  }

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

  AutoSerializeRotation* rotValue = std::get_if<AutoSerializeRotation>(&value);
  if (rotValue != NULL){
    glm::quat* address = (glm::quat*)(((char*)structAddress) + rotValue -> structOffset);
    attrSet(attributes, address, rotValue -> field);
    return;
  }

  AutoSerializeEnums* enumsValue = std::get_if<AutoSerializeEnums>(&value);
  if (enumsValue != NULL){
    int* address = (int*)(((char*)structAddress) + enumsValue -> structOffset);
    attrSet(attributes, address, enumsValue -> enums, enumsValue -> enumStrings, enumsValue -> field, true);
    return;
  }

  AutoSerializeCustom* customValue = std::get_if<AutoSerializeCustom>(&value);
  if (customValue != NULL){
    int* address = (int*)(((char*)structAddress) + customValue -> structOffset);
    if (customValue -> fieldType == ATTRIBUTE_VEC3){
      if (attributes.vecAttr.vec3.find(customValue -> field) == attributes.vecAttr.vec3.end()){
        customValue -> setAttributes(address, NULL);
      }else{
        customValue -> setAttributes(address, &(attributes.vecAttr.vec3.at(customValue -> field)));
      }
    }else if (customValue -> fieldType == ATTRIBUTE_VEC4){
      if (attributes.vecAttr.vec4.find(customValue -> field) == attributes.vecAttr.vec4.end()){
        customValue -> setAttributes(address, NULL);
      }else{
        customValue -> setAttributes(address, &(attributes.vecAttr.vec4.at(customValue -> field)));
      }
    }else if (customValue -> fieldType == ATTRIBUTE_STRING){
      if (attributes.stringAttributes.find(customValue -> field) == attributes.stringAttributes.end()){
        customValue -> setAttributes(address, NULL);
      }else{
        customValue -> setAttributes(address, &(attributes.stringAttributes.at(customValue -> field)));
      }
    }else if (customValue -> fieldType == ATTRIBUTE_FLOAT){
      if (attributes.numAttributes.find(customValue -> field) == attributes.numAttributes.end()){
        customValue -> setAttributes(address, NULL);
      }else{
        // why is num attributes a double?  Should unify
        float value = static_cast<float>(attributes.numAttributes.at(customValue -> field));
        customValue -> setAttributes(address, &value);
      }
    }else{
      modassert(false, "custom value -> invalid field type");
    }
    return;
  }

  AutoserializeReservedField* reservedField = std::get_if<AutoserializeReservedField>(&value);
  if (reservedField != NULL){
    // do nothing
    return;
  }

  modassert(false, "autoserialize type not found");
}

void autoserializerSetAttr(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attributes){
  for (auto &value : values){
    autoserializerSetAttr(structAddress, value, attributes);
  }
}

void autoserializerSetAttrWithTextureLoading(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  autoserializerSetAttr(structAddress, values, attributes);
  autoserializeHandleTextureLoading(structAddress, values, util.ensureTextureLoaded, util.releaseTexture);
}

std::string serializerName(AutoSerialize& serializer){
  AutoSerializeBool* boolSerializer = std::get_if<AutoSerializeBool>(&serializer);
  if (boolSerializer != NULL){
    return boolSerializer -> field;
  }
  AutoSerializeString* stringSerializer = std::get_if<AutoSerializeString>(&serializer);
  if (stringSerializer != NULL){
    return stringSerializer -> field;
  }

  AutoSerializeForceString* forcedStringSerializer = std::get_if<AutoSerializeForceString>(&serializer);
  if (forcedStringSerializer != NULL){
    return forcedStringSerializer -> field;
  }

  AutoSerializeTextureLoaderManual* textureSerializer = std::get_if<AutoSerializeTextureLoaderManual>(&serializer);
  if (textureSerializer != NULL){
    return textureSerializer -> field;
  }
  AutoSerializeEnums* enumsSerializer = std::get_if<AutoSerializeEnums>(&serializer);
  if (enumsSerializer != NULL){
    return enumsSerializer -> field;
  }
  AutoSerializeVec2* vec2Serializer = std::get_if<AutoSerializeVec2>(&serializer);
  if (vec2Serializer != NULL){
    return vec2Serializer -> field;
  }
  AutoSerializeFloat* floatSerializer = std::get_if<AutoSerializeFloat>(&serializer);
  if (floatSerializer != NULL){
    return floatSerializer -> field;
  }
  AutoSerializeInt* intSerializer = std::get_if<AutoSerializeInt>(&serializer);
  if (intSerializer != NULL){
    return intSerializer -> field;
  }
  AutoSerializeUInt* uintSerializer = std::get_if<AutoSerializeUInt>(&serializer);
  if (uintSerializer != NULL){
    return uintSerializer -> field;
  }
  AutoSerializeVec3* vec3Serializer = std::get_if<AutoSerializeVec3>(&serializer);
  if (vec3Serializer != NULL){
    return vec3Serializer -> field;
  }
  AutoSerializeVec4* vec4Serializer = std::get_if<AutoSerializeVec4>(&serializer);
  if (vec4Serializer != NULL){
    return vec4Serializer -> field;
  }

  AutoSerializeRotation* rotSerializer = std::get_if<AutoSerializeRotation>(&serializer);
  if (rotSerializer != NULL){
    return rotSerializer -> field;
  }

  AutoSerializeCustom* customSerializer = std::get_if<AutoSerializeCustom>(&serializer);
  if (customSerializer != NULL){
    return customSerializer -> field;
  }
  AutoserializeReservedField* reservedField = std::get_if<AutoserializeReservedField>(&serializer);
  if (reservedField != NULL){
    // do nothing
    return reservedField -> field;
  }
  modassert(false, "could not find serializer");
  return "";
}

std::optional<AutoSerialize> serializerByName(std::vector<AutoSerialize>& serializer, std::string& name){
  for (auto &value : serializer){
    if (serializerName(value) == name){
      return value;
    }
  }
  return std::nullopt;
}

std::set<std::string> serializerFieldNames(std::vector<AutoSerialize>& serializers){
  std::set<std::string> fields;
  for (auto &autoserializer : serializers){
    fields.insert(serializerName(autoserializer));
  }
  return fields;
}

AttributeValueType typeForSerializer(AutoSerialize& serializer){
  AutoSerializeBool* boolSerializer = std::get_if<AutoSerializeBool>(&serializer);
  AutoSerializeString* stringSerializer = std::get_if<AutoSerializeString>(&serializer);
  AutoSerializeForceString* forcedStringSerializer = std::get_if<AutoSerializeForceString>(&serializer);
  AutoSerializeTextureLoaderManual* textureSerializer = std::get_if<AutoSerializeTextureLoaderManual>(&serializer);
  AutoSerializeEnums* enumsSerializer = std::get_if<AutoSerializeEnums>(&serializer);
  AutoSerializeVec2* vec2Serializer = std::get_if<AutoSerializeVec2>(&serializer);
  if (boolSerializer != NULL || stringSerializer != NULL || forcedStringSerializer != NULL || textureSerializer != NULL || enumsSerializer != NULL || vec2Serializer != NULL){
    return ATTRIBUTE_STRING;
  }
  AutoSerializeFloat* floatSerializer = std::get_if<AutoSerializeFloat>(&serializer);
  AutoSerializeInt* intSerializer = std::get_if<AutoSerializeInt>(&serializer);
  AutoSerializeUInt* uintSerializer = std::get_if<AutoSerializeUInt>(&serializer);
  if (floatSerializer != NULL || intSerializer != NULL || uintSerializer != NULL){
    return ATTRIBUTE_FLOAT;
  }
  AutoSerializeVec3* vec3Serializer = std::get_if<AutoSerializeVec3>(&serializer);
  if (vec3Serializer != NULL){
    return ATTRIBUTE_VEC3;
  }
  AutoSerializeVec4* vec4Serializer = std::get_if<AutoSerializeVec4>(&serializer);
  if (vec3Serializer != NULL){
    return ATTRIBUTE_VEC4;
  }
  AutoSerializeRotation* rotSerializer = std::get_if<AutoSerializeRotation>(&serializer);
  if (rotSerializer != NULL){
    return ATTRIBUTE_VEC4;
  }
  AutoSerializeCustom* customSerializer = std::get_if<AutoSerializeCustom>(&serializer);
  if (customSerializer != NULL){
    return customSerializer -> fieldType;
  }
  AutoserializeReservedField* reservedField = std::get_if<AutoserializeReservedField>(&serializer);
  if (reservedField != NULL){
    // do nothing
    return reservedField -> fieldType;
  }
  modassert(false, "type for serializer invalid type");
  return ATTRIBUTE_STRING;
}

std::string printTextureDebug(TextureInformation& info){
  std::string debug = "";
  debug += "offset = " + print(info.textureoffset);
  debug += ", tiling = " + print(info.texturetiling);
  debug += ", size = " + print(info.texturesize);
  debug += ", id = " + std::to_string(info.loadingInfo.textureId);
  debug += ", texstring = " + info.loadingInfo.textureString;
  return debug;
}