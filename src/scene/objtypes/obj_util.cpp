#include "./obj_util.h"

void attrSetString(std::optional<std::string> strValue, std::string* value, std::string defaultValue){
  if (strValue.has_value()){
    *value = strValue.value();
  }else{
    *value = defaultValue;
  } 
}

void attrForceSet(std::optional<AttributeValue> attrValue, std::string* value, std::string defaultValue){
  if (attrValue.has_value()){
    auto strValue = std::get_if<std::string>(&attrValue.value());
    auto floatValue = std::get_if<float>(&attrValue.value());
    auto vec3Value = std::get_if<glm::vec3>(&attrValue.value());
    auto vec4Value = std::get_if<glm::vec4>(&attrValue.value());
    if (strValue){
      *value = *strValue;
    }else if (floatValue){
      *value = serializeFloat(*floatValue);
    }else if (vec3Value){
      *value = serializeVec(*vec3Value);
    }else if (vec4Value){
      *value = serializeVec(*vec4Value);
    }
    modassert(strValue || floatValue || vec3Value || vec4Value, "invalid value type attrForceSet");
  }else{
    *value = defaultValue;
  }
}

void attrSetFloat(std::optional<float> floatValue, float* _value, bool* _hasValue, float defaultValue){
  //modassert(!floatValue.has_value(), std::string("value is: ") + std::string(field));
  if (floatValue.has_value()){
    *_value = floatValue.value();
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

void attrSetUint(std::optional<float> floatValue, unsigned int* value, unsigned int defaultValue){
  if (floatValue.has_value()){
    *value = static_cast<unsigned int>(floatValue.value());
  }else{
    *value = defaultValue;
  }
}

void attrSetInt(std::optional<float> floatValue, int* _value, int defaultValue){
  if (floatValue.has_value()){
    *_value = static_cast<int>(floatValue.value());
  }else{
    *_value = defaultValue;
  }
}

void attrSetVec3(std::optional<glm::vec3> vec3Value, glm::vec3* _value, bool* _hasValue, glm::vec3 defaultValue){
  if (vec3Value.has_value()){
    *_value = vec3Value.value();
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

void attrSetVec2(std::optional<glm::vec2> vec2Value, glm::vec2* _value, bool* _hasValue, glm::vec2 defaultValue){
  if (vec2Value.has_value()){
    *_value = vec2Value.value();
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

void attrSetVec4(std::optional<glm::vec4> vec4Value, glm::vec4* _value, bool* _hasValue, glm::vec4 defaultValue){
  if (vec4Value.has_value()){
    *_value = vec4Value.value();
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

void attrSetCreateQuat(std::optional<glm::vec4> vec4Value, glm::quat* _value, glm::quat defaultValue){
  if (vec4Value.has_value()){
    *_value = parseQuat(vec4Value.value());
  }else{
    *_value = defaultValue;
  }  
}

void attrSetCreate(std::optional<std::string> attrValue, bool* _value, const char* onString, const char* offString, bool defaultValue){
  if (attrValue.has_value()){
    auto stringValue = attrValue.value();
    if (stringValue == onString){
      *_value = true;
    }else if (stringValue == offString){
      *_value = false;
    }else{
      modassert(false, "Invalid string value in attrSet - " + stringValue);
    }
  }else{
    *_value = defaultValue;
  }
}

void attrSetLoadTextureManual(std::optional<std::string> strValue, TextureLoadingData* _textureLoading, std::string defaultTexture){
  std::string textureToLoad = defaultTexture;
  if (strValue.has_value()){
    textureToLoad = strValue.value();
  }
  if (textureToLoad != _textureLoading -> textureString){
    _textureLoading -> isLoaded = false;
    _textureLoading -> textureId = -1;
    _textureLoading -> textureString = textureToLoad;
  }
  //std::cout << "texture to load: " << textureToLoad << " isloaded: " << _textureLoading -> isLoaded << std::endl;
}

void attrSetEnums(std::optional<std::string> strValue, int* _value, std::vector<int> enums, std::vector<std::string> enumStrings, int defaultValue, const char* field){
  if (strValue.has_value()){
    auto value = strValue.value();
    bool foundEnum = false;
    for (int i = 0; i < enumStrings.size(); i++){
      if (enumStrings.at(i) == value){
        *_value = enums.at(i);
        foundEnum = true;
        break;
      }
    }
    modassert(foundEnum, std::string("invalid enum type for ") + std::string(field) + " - " + value);
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


void autoserializeHandleTextureLoading(char* structAddress, AutoSerialize& value, std::function<Texture(std::string)> ensureTextureLoaded, std::function<void(int)> releaseTexture){
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

void createAutoSerializeAttr(char* structAddress, AutoSerialize& value, std::optional<AttributeValue>&& attrValue){
  AutoSerializeBool* boolValue = std::get_if<AutoSerializeBool>(&value);
  if (boolValue != NULL){
    bool* address = (bool*)(((char*)structAddress) + boolValue -> structOffset);
    attrSetCreate(unwrapAttrOpt<std::string>(attrValue), address, boolValue -> onString, boolValue -> offString, boolValue -> defaultValue);
    return;
  }

  AutoSerializeString* strValue = std::get_if<AutoSerializeString>(&value);
  if (strValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValue -> structOffset);
    attrSetString(unwrapAttrOpt<std::string>(attrValue), address, strValue -> defaultValue);
    return;
  }

  AutoSerializeForceString* strForcedValue = std::get_if<AutoSerializeForceString>(&value);
  if (strForcedValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strForcedValue -> structOffset);
    attrForceSet(attrValue, address, strForcedValue -> defaultValue);
    return;
  }

  AutoSerializeFloat* floatValue = std::get_if<AutoSerializeFloat>(&value);
  if (floatValue != NULL){
    float* address = (float*)(((char*)structAddress) + floatValue -> structOffset);
    bool* hasValueAddress = (!floatValue -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + floatValue -> structOffsetFiller.value());
    attrSetFloat(unwrapAttrOpt<float>(attrValue), address, hasValueAddress, floatValue -> defaultValue);
    return;
  }

  AutoSerializeTextureLoaderManual* textureLoaderManual = std::get_if<AutoSerializeTextureLoaderManual>(&value);
  if (textureLoaderManual != NULL){
    TextureLoadingData* address = (TextureLoadingData*)(((char*)structAddress) + textureLoaderManual -> structOffset);
    attrSetLoadTextureManual(unwrapAttrOpt<std::string>(attrValue), address, textureLoaderManual -> defaultValue);
    return;
  }

  AutoSerializeInt* intValue = std::get_if<AutoSerializeInt>(&value);
  if (intValue != NULL){
    int* address = (int*)(((char*)structAddress) + intValue -> structOffset);
    attrSetInt(unwrapAttrOpt<float>(attrValue), address, intValue -> defaultValue);    
    return;
  }

  AutoSerializeUInt* uintValue = std::get_if<AutoSerializeUInt>(&value);
  if (uintValue != NULL){
    uint* address = (uint*)(((char*)structAddress) + uintValue -> structOffset);
    attrSetUint(unwrapAttrOpt<float>(attrValue), address, uintValue -> defaultValue);
    return;
  }

  AutoSerializeVec2* vec2Value = std::get_if<AutoSerializeVec2>(&value);
  if (vec2Value != NULL){
    glm::vec2* address = (glm::vec2*)(((char*)structAddress) + vec2Value -> structOffset);
    bool* hasValueAddress = (!vec2Value -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + vec2Value -> structOffsetFiller.value());
    attrSetVec2(unwrapAttrOpt<glm::vec2>(attrValue), address, hasValueAddress, vec2Value -> defaultValue);
    return;
  }

  AutoSerializeVec3* vec3Value = std::get_if<AutoSerializeVec3>(&value);
  if (vec3Value != NULL){
    glm::vec3* address = (glm::vec3*)(((char*)structAddress) + vec3Value -> structOffset);
    bool* hasValueAddress = (!vec3Value -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + vec3Value -> structOffsetFiller.value());
    attrSetVec3(unwrapAttrOpt<glm::vec3>(attrValue), address, hasValueAddress, vec3Value -> defaultValue);
    return;
  }

  AutoSerializeVec4* vec4Value = std::get_if<AutoSerializeVec4>(&value);
  if (vec4Value != NULL){
    glm::vec4* address = (glm::vec4*)(((char*)structAddress) + vec4Value -> structOffset);
    bool* hasValueAddress = (!vec4Value -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + vec4Value -> structOffsetFiller.value());
    attrSetVec4(unwrapAttrOpt<glm::vec4>(attrValue), address, hasValueAddress, vec4Value -> defaultValue);
    return;
  }

  AutoSerializeRotation* rotValue = std::get_if<AutoSerializeRotation>(&value);
  if (rotValue != NULL){
    glm::quat* address = (glm::quat*)(((char*)structAddress) + rotValue -> structOffset);
    attrSetCreateQuat(unwrapAttrOpt<glm::vec4>(attrValue), address, rotValue -> defaultValue);
    return;
  }

  AutoSerializeEnums* enumsValue = std::get_if<AutoSerializeEnums>(&value);
  if (enumsValue != NULL){
    int* address = (int*)(((char*)structAddress) + enumsValue -> structOffset);
    attrSetEnums(unwrapAttrOpt<std::string>(attrValue), address, enumsValue -> enums, enumsValue -> enumStrings, enumsValue -> defaultValue, enumsValue -> field);
    return;
  }

  AutoSerializeCustom* customValue = std::get_if<AutoSerializeCustom>(&value);
  if (customValue != NULL){
    int* address = (int*)(((char*)structAddress) + customValue -> structOffset);

    if (customValue -> fieldType == ATTRIBUTE_VEC3){
      auto vec3Value = maybeUnwrapAttrOpt<glm::vec3>(attrValue);
      if (!vec3Value.has_value()){
        customValue -> deserialize(address, NULL);
      }else{
        customValue -> deserialize(address, &vec3Value.value());
      }
    }else if (customValue -> fieldType == ATTRIBUTE_VEC4){
      auto vec4Value = maybeUnwrapAttrOpt<glm::vec4>(attrValue);
      if (!vec4Value.has_value()){
        customValue -> deserialize(address, NULL);
      }else{
        customValue -> deserialize(address, &vec4Value.value());
      }
    }else if (customValue -> fieldType == ATTRIBUTE_STRING){
      auto strValue = maybeUnwrapAttrOpt<std::string>(attrValue);
      if (!strValue.has_value()){
        customValue -> deserialize(address, NULL);
      }else{
        customValue -> deserialize(address, &strValue.value());
      }
    }else if (customValue -> fieldType == ATTRIBUTE_FLOAT){
      auto floatValue = maybeUnwrapAttrOpt<float>(attrValue);
      if (!floatValue.has_value()){
        customValue -> deserialize(address, NULL);
      }else{
        customValue -> deserialize(address, &floatValue.value());
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
    createAutoSerializeAttr(structAddress, value, getAttributeValue(attr, serializerFieldName(value).c_str()));
  }
}
void createAutoSerializeWithTextureLoading(char* structAddress, std::vector<AutoSerialize>& values, GameobjAttributes& attr, ObjectTypeUtil& util){
  createAutoSerialize(structAddress, values, attr);
  for (auto &value : values){
    autoserializeHandleTextureLoading(structAddress, value, util.ensureTextureLoaded, util.releaseTexture);
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
    if (serializerFieldName(value) == field){
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
    float* address = (float*)(((char*)structAddress) + floatValue -> structOffset);
    return address;
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

std::optional<AttributeValuePtr> getAttributePtr(char* structAddress, std::vector<AutoSerialize>& autoserializerConfig, const char* field){
  auto autoserializer = getAutoserializeByField(autoserializerConfig, field);
  if (autoserializer.has_value()){
    auto attrPtrValue = autoserializerGetAttrPtr(structAddress, *autoserializer.value());
    if (attrPtrValue.has_value()){
      return attrPtrValue;
    }
  }
  return std::nullopt;
}

bool autoserializerSetAttr(char* structAddress, AutoSerialize& value, const char* fieldName, std::optional<AttributeValue> attributeValue){
  AutoSerializeBool* boolValue = std::get_if<AutoSerializeBool>(&value);
  if (boolValue != NULL){
    bool* address = (bool*)(((char*)structAddress) + boolValue -> structOffset);
    if (attributeValue.has_value()){
      bool* boolPtr = std::get_if<bool>(&attributeValue.value());
      if (boolPtr){
        *address = *boolPtr;
        return true;
      }


      auto value = unwrapAttr<std::string>(attributeValue.value());
      if (value == boolValue -> onString){
         *address = true;
      }else if (value == boolValue -> offString){
         *address = false;
      }else {
         modassert(false, "invalid on/off string");
      }
      return true;
    }
    return false;
  }

  AutoSerializeString* strValue = std::get_if<AutoSerializeString>(&value);
  if (strValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strValue -> structOffset);
    if (attributeValue.has_value()){
      *address = unwrapAttr<std::string>(attributeValue.value());
      return true;
    }
    return false;
  }

  AutoSerializeForceString* strForcedValue = std::get_if<AutoSerializeForceString>(&value);
  if (strForcedValue != NULL){
    std::string* address = (std::string*)(((char*)structAddress) + strForcedValue -> structOffset);

    if (attributeValue.has_value()){
      bool isStringAttr = std::get_if<std::string>(&attributeValue.value());
      bool isFloatAttr = std::get_if<float>(&attributeValue.value());
      bool isVec3Attr = std::get_if<glm::vec3>(&attributeValue.value());
      bool isVec4Attr = std::get_if<glm::vec4>(&attributeValue.value());

      if (isStringAttr){
        *address = unwrapAttr<std::string>(attributeValue.value());
        return true;
      }else if (isFloatAttr){
        *address = serializeFloat(unwrapAttr<float>(attributeValue.value()));
        return true;
      }else if (isVec3Attr){
        *address = serializeVec(unwrapAttr<glm::vec3>(attributeValue.value()));
        return true;
      }else if (isVec4Attr){
        *address = serializeVec(unwrapAttr<glm::vec4>(attributeValue.value()));
        return true;
      }else{
        modassert(false, "invalid type strForcedValue");
      }
    }

    return false;
  }
  
  AutoSerializeFloat* floatValue = std::get_if<AutoSerializeFloat>(&value);
  if (floatValue != NULL){
    float* address = (float*)(((char*)structAddress) + floatValue -> structOffset);
    if (attributeValue.has_value()){
      *address = unwrapAttr<float>(attributeValue.value());
      return true;
    }
    return false;
  }

  AutoSerializeTextureLoaderManual* textureLoaderManual = std::get_if<AutoSerializeTextureLoaderManual>(&value);
  if (textureLoaderManual != NULL){
    TextureLoadingData* address = (TextureLoadingData*)(((char*)structAddress) + textureLoaderManual -> structOffset);
    if (attributeValue.has_value()){
      std::string textureToLoad = unwrapAttr<std::string>(attributeValue.value());

      //std::cout << "attr set load texture manual: tex to load: " << textureToLoad << std::endl;
      //std::cout << "texture string: " << _textureLoading -> textureString << std::endl;
      //std::cout << "isloaded: " << _textureLoading -> isLoaded << std::endl;
      if (textureToLoad != address -> textureString){
        address -> isLoaded = false;
        address -> textureId = -1;
        address -> textureString = textureToLoad;
        return true;
      }
    }
    return false;
  }

  AutoSerializeInt* intValue = std::get_if<AutoSerializeInt>(&value);
  if (intValue != NULL){
    int* address = (int*)(((char*)structAddress) + intValue -> structOffset);
    if (attributeValue.has_value()){
      float floatValue = unwrapAttr<float>(attributeValue.value());
      *address = static_cast<int>(floatValue);
      return true;
    }
    return false;
  }
  
  AutoSerializeUInt* uintValue = std::get_if<AutoSerializeUInt>(&value);
  if (uintValue != NULL){
    uint* address = (uint*)(((char*)structAddress) + uintValue -> structOffset);
    if (attributeValue.has_value()){
      float floatValue = unwrapAttr<float>(attributeValue.value());
      *address = static_cast<unsigned int>(floatValue);
      return true;
    }
    return false;
  }

  AutoSerializeVec2* vec2Value = std::get_if<AutoSerializeVec2>(&value);
  if (vec2Value != NULL){
    glm::vec2* address = (glm::vec2*)(((char*)structAddress) + vec2Value -> structOffset);
    bool* hasValueAddress = (!vec2Value -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + vec2Value -> structOffsetFiller.value());
    if (attributeValue.has_value()){
      glm::vec2 vec2Value = unwrapAttr<glm::vec2>(attributeValue.value());
      *address = vec2Value;
      if (hasValueAddress != NULL){
        *hasValueAddress = true;
      }
      return true;
    }else{
      if (hasValueAddress != NULL){
        *hasValueAddress = false;
      }
    }  
    return false;
  }

  AutoSerializeVec3* vec3Value = std::get_if<AutoSerializeVec3>(&value);
  if (vec3Value != NULL){
    glm::vec3* address = (glm::vec3*)(((char*)structAddress) + vec3Value -> structOffset);
    bool* hasValueAddress = (!vec3Value -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + vec3Value -> structOffsetFiller.value());
    if (attributeValue.has_value()){
      *address = unwrapAttr<glm::vec3>(attributeValue.value());
      if (hasValueAddress != NULL){
        *hasValueAddress = true;
      }
      return true;
    }
    return false;
  }

  AutoSerializeVec4* vec4Value = std::get_if<AutoSerializeVec4>(&value);
  if (vec4Value != NULL){
    glm::vec4* address = (glm::vec4*)(((char*)structAddress) + vec4Value -> structOffset);
    bool* hasValueAddress = (!vec4Value -> structOffsetFiller.has_value()) ? NULL : (bool*)(((char*)structAddress) + vec4Value -> structOffsetFiller.value());
    if (attributeValue.has_value()){
      *address = unwrapAttr<glm::vec4>(attributeValue.value());
      if (hasValueAddress != NULL){
        *hasValueAddress = true;
      }
      return true;
    }
    return false;
  }

  AutoSerializeRotation* rotValue = std::get_if<AutoSerializeRotation>(&value);
  if (rotValue != NULL){
    glm::quat* address = (glm::quat*)(((char*)structAddress) + rotValue -> structOffset);
    if (attributeValue.has_value()){
      glm::vec4 vec4Value = unwrapAttr<glm::vec4>(attributeValue.value());
      *address = parseQuat(vec4Value);
      return true;
    } 
    return false;
  }

  AutoSerializeEnums* enumsValue = std::get_if<AutoSerializeEnums>(&value);
  if (enumsValue != NULL){
    int* address = (int*)(((char*)structAddress) + enumsValue -> structOffset);
    if (attributeValue.has_value()){
      std::string value = unwrapAttr<std::string>(attributeValue.value());
      bool foundEnum = false;
      for (int i = 0; i < enumsValue -> enumStrings.size(); i++){
        if (enumsValue -> enumStrings.at(i) == value){
          *address = enumsValue -> enums.at(i);
          foundEnum = true;
          break;
        }
      }
      return true;
      modassert(foundEnum, std::string("invalid enum type for ") + enumsValue -> field + " - " + value);
    }
    return false;
  }

  AutoSerializeCustom* customValue = std::get_if<AutoSerializeCustom>(&value);
  if (customValue != NULL && attributeValue.has_value()){
    int* address = (int*)(((char*)structAddress) + customValue -> structOffset);
    if (customValue -> fieldType == ATTRIBUTE_VEC3){
      auto vec3Type = std::get_if<glm::vec3>(&attributeValue.value());
      if (vec3Type){
        customValue -> setAttributes(address, vec3Type);
        return true;
      }
    }else if (customValue -> fieldType == ATTRIBUTE_VEC4){
      auto vec4Type = std::get_if<glm::vec4>(&attributeValue.value());
      if (vec4Type){
        customValue -> setAttributes(address, vec4Type);
        return true;
      }
    }else if (customValue -> fieldType == ATTRIBUTE_STRING){
      auto strType = std::get_if<std::string>(&attributeValue.value());
      if (strType){
        customValue -> setAttributes(address, strType);
        return true;
      }
    }else if (customValue -> fieldType == ATTRIBUTE_FLOAT){
      auto floatType = std::get_if<float>(&attributeValue.value());
      if (floatType){
        float value = static_cast<float>(*floatType);
        customValue -> setAttributes(address, &value);
        return true;
      }
    }else{
      modassert(false, "custom value -> invalid field type");
    }
    return false;
  }

  AutoserializeReservedField* reservedField = std::get_if<AutoserializeReservedField>(&value);
  if (reservedField != NULL){
    // do nothing
    return false;
  }

  modassert(false, "autoserialize type not found");
  return false;
}

bool autoserializerSetAttrWithTextureLoading(char* structAddress, std::vector<AutoSerialize>& values, const char* field, AttributeValue attrValue, ObjectSetAttribUtil& util){
  auto serializer = getAutoserializeByField(values, field);
  if (!serializer.has_value()){
    return false;
  }
  bool setAttr = autoserializerSetAttr(structAddress, *(serializer.value()), field, attrValue);
  autoserializeHandleTextureLoading(structAddress, *(serializer.value()), util.ensureTextureLoaded, util.releaseTexture);
  return setAttr;
}

std::string serializerFieldName(AutoSerialize& serializer){
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

std::set<std::string> serializerFieldNames(std::vector<AutoSerialize>& serializers){
  std::set<std::string> fields;
  for (auto &autoserializer : serializers){
    fields.insert(serializerFieldName(autoserializer));
  }
  return fields;
}

AttributeValueType typeForSerializer(AutoSerialize& serializer){
  AutoSerializeBool* boolSerializer = std::get_if<AutoSerializeBool>(&serializer);
  AutoSerializeString* stringSerializer = std::get_if<AutoSerializeString>(&serializer);
  AutoSerializeForceString* forcedStringSerializer = std::get_if<AutoSerializeForceString>(&serializer);
  AutoSerializeTextureLoaderManual* textureSerializer = std::get_if<AutoSerializeTextureLoaderManual>(&serializer);
  AutoSerializeEnums* enumsSerializer = std::get_if<AutoSerializeEnums>(&serializer);
  if (boolSerializer != NULL || stringSerializer != NULL || forcedStringSerializer != NULL || textureSerializer != NULL || enumsSerializer != NULL){
    return ATTRIBUTE_STRING;
  }
  AutoSerializeFloat* floatSerializer = std::get_if<AutoSerializeFloat>(&serializer);
  AutoSerializeInt* intSerializer = std::get_if<AutoSerializeInt>(&serializer);
  AutoSerializeUInt* uintSerializer = std::get_if<AutoSerializeUInt>(&serializer);
  if (floatSerializer != NULL || intSerializer != NULL || uintSerializer != NULL){
    return ATTRIBUTE_FLOAT;
  }
  AutoSerializeVec2* vec2Serializer = std::get_if<AutoSerializeVec2>(&serializer);
  if (vec2Serializer != NULL){
    return ATTRIBUTE_VEC2;
  }
  AutoSerializeVec3* vec3Serializer = std::get_if<AutoSerializeVec3>(&serializer);
  if (vec3Serializer != NULL){
    return ATTRIBUTE_VEC3;
  }
  AutoSerializeVec4* vec4Serializer = std::get_if<AutoSerializeVec4>(&serializer);
  if (vec4Serializer != NULL){
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
  modassert(false, std::string("type for serializer invalid type: ") + serializerFieldName(serializer));
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