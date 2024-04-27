void autoserializerGetAttr(char* structAddress, AutoSerialize& value, GameobjAttributes& _attributes){
  AutoSerializeBool* boolValue = std::get_if<AutoSerializeBool>(&value);
  if (boolValue != NULL){
    bool valueAtPtr = *(getTypeFromAttr<bool>(autoserializerGetAttrPtr(structAddress, value)).value());
    _attributes.stringAttributes[boolValue -> field] = valueAtPtr ? boolValue -> onString : boolValue -> offString;
    return;
  }
 
  AutoSerializeString* strValue = std::get_if<AutoSerializeString>(&value);
  AutoSerializeForceString* strForceValue = std::get_if<AutoSerializeForceString>(&value);
  if (strValue != NULL || strForceValue != NULL){
    std::string* address = (getTypeFromAttr<std::string>(autoserializerGetAttrPtr(structAddress, value)).value());
    _attributes.stringAttributes[strValue -> field] = *address;
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