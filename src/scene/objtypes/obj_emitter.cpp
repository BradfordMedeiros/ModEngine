#include "./obj_emitter.h"

GameobjAttributes particleFields(GameobjAttributes& attributes){
  GameobjAttributes attr {
    .attr = {},
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  for (auto [key, value] : attributes.attr){
    auto strValue = std::get_if<std::string>(&value);
    if (!strValue){
      modassert(false, "invalid type particleFields");
    }
    if (key.at(0) == '+' && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      attr.attr[newKey] = *strValue;
    }
  }
  for (auto [key, value] : attributes.numAttributes){
    if (key.at(0) == '+' && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      attr.numAttributes[newKey] = value;
    }
  }
  for (auto [key, value] : attributes.vecAttr.vec3){
    if (key.at(0) == '+' && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      attr.vecAttr.vec3[newKey] = value;
    }
  }
  for (auto [key, value] : attributes.vecAttr.vec4){
    if (key.at(0) == '+' && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      attr.vecAttr.vec4[newKey] = value;
    }
  }
  return attr;
}

std::vector<EmitterDelta> emitterDeltas(GameobjAttributes& attributes){
  std::map<std::string, EmitterDelta> values;
  ///////////////// Same code for diff types consolidate
  for (auto [key, _] : attributes.vecAttr.vec3){
    if ((key.at(0) == '!' || key.at(0) == '?' || key.at(0) == '%') && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      values[newKey] = EmitterDelta {
        .attributeName = newKey,
        .value = glm::vec3(0.f, 0.f, 0.f),
        .variance = glm::vec3(0.f, 0.f, 0.f),
        .lifetimeEffect = {},
      };
    }
  }
  for (auto [key, _] : attributes.vecAttr.vec4){
    if ((key.at(0) == '!' || key.at(0) == '?' || key.at(0) == '%') && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      values[newKey] = EmitterDelta {
        .attributeName = newKey,
        .value = glm::vec4(0.f, 0.f, 0.f, 0.f),
        .variance = glm::vec4(0.f, 0.f, 0.f, 0.f),
        .lifetimeEffect = {},
      };
    }
  }
  ////////////////////////////////////////////////////////////
  
  ///////////////// Same code for diff types
  for (auto [key, value] : attributes.vecAttr.vec3){
    std::cout <<  "emitter: adding vec3 type for: " << key << std::endl;
    if (key.size() > 1){
      auto newKey = key.substr(1, key.size());
      if (key.at(0) == '!'){
        values.at(newKey).value = value;
      }else if (key.at(0) == '?'){
        values.at(newKey).variance = value;
      }
      /*else if (key.at(0) == '%'){
        values.at(newKey).lifetimeEffect = parseFloatVec(value);
      }*/
    }
  }
  for (auto [key, value] : attributes.vecAttr.vec4){
    std::cout <<  "emitter: adding vec4 type for: " << key << std::endl;
    if (key.size() > 1){
      auto newKey = key.substr(1, key.size());
      if (key.at(0) == '!'){
        values.at(newKey).value = value;
      }else if (key.at(0) == '?'){
        values.at(newKey).variance = value;
      }
      /*else if (key.at(0) == '%'){
        values.at(newKey).lifetimeEffect = parseFloatVec(value);
      }*/
    }
  }
  ////////////////////////////////////////////////////////////////

  std::vector<EmitterDelta> deltas;
  for (auto [key, value] : values){
    deltas.push_back(EmitterDelta{
      .attributeName = key,
      .value = value.value,
      .variance = value.variance,
      .lifetimeEffect = value.lifetimeEffect,
    });
  }

  for (auto &delta : deltas){
    std::cout << "create emitter - " << delta.attributeName << " - " << print(delta.value) << " = " << print(delta.variance) << std::endl;
  }

  return deltas;
}

std::vector<AutoSerialize> emitterAutoserializer {
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectEmitter, rate),
    .structOffsetFiller = std::nullopt,
    .field = "rate",
    .defaultValue = 1.f,
  },
  AutoSerializeFloat {
    .structOffset = offsetof(GameObjectEmitter, duration),
    .structOffsetFiller = std::nullopt,
    .field = "duration",
    .defaultValue = 10.f,
  },
  AutoSerializeUInt {
    .structOffset = offsetof(GameObjectEmitter, limit),
    .field = "limit",
    .defaultValue = 10,
  },
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectEmitter, state),
    .field = "state", 
    .onString = "enabled",
    .offString = "disabled",
    .defaultValue = true,
  },
  AutoSerializeEnums {
    .structOffset = offsetof(GameObjectEmitter, deleteBehavior),
    .enums = { EMITTER_NOTYPE, EMITTER_DELETE, EMITTER_ORPHAN, EMITTER_FINISH },
    .enumStrings = { "notype", "delete", "orphan", "finish" },
    .field = "onremove",
    .defaultValue = EMITTER_DELETE,
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObjectEmitter, templateName),
    .field = "template",
    .defaultValue = "obj",
  }
};


bool isEmitterPrefix(char character){
  return character == '!' || character == '?' || character == '%' || character == '+';
}
std::set<std::string> emitterSubmodelAttr(std::vector<GameobjAttribute>& attr){
  std::set<std::string> submodelNames;
  //std::cout << "submodel attr: [ ";
  for (auto &val : attr){
    if (isEmitterPrefix(val.field.at(0))){
      auto strippedAttribute = val.field.substr(1, val.field.size());
      auto subelementVal = subelementTargetName(strippedAttribute);
      auto mainname = mainTargetElement(strippedAttribute);
      //std::cout << val.attribute << "( " << subelementVal.has_value() << " ) [ " << (subelementVal.has_value() ? mainname : "- no name -") << " ], ";
      if (subelementVal.has_value()){
        submodelNames.insert(mainname);
      }
    }
  }
  //std::cout << "]" << std::endl;
  return submodelNames;
}

struct EmitterSpecialAttribute {
  std::string subelement;
  std::string attribute;
};

std::optional<EmitterSpecialAttribute> extractSpecialAttribute(std::string key){
  if (key.at(0) == '+'){
    auto strippedAttribute = key.substr(1, key.size());
    auto subelementVal = subelementTargetName(strippedAttribute);
    auto mainname = mainTargetElement(strippedAttribute);
    if (subelementVal.has_value()){
      return EmitterSpecialAttribute {
        .subelement = mainname,
        .attribute = subelementVal.value(),
      };
    }
  }
  return std::nullopt;
}

GameobjAttributes emitterExtractAttributes(GameobjAttributes& attributes, std::string name){
  std::map<std::string, AttributeValue> attrs;
  std::map<std::string, float> numAttributes;
  std::map<std::string, glm::vec3> vec3;
  std::map<std::string, glm::vec4> vec4;

  for (auto &[key, value] : attributes.attr){
    auto extractedAttribute = extractSpecialAttribute(key);
    if (extractedAttribute.has_value() && extractedAttribute.value().subelement == name){
      attrs[extractedAttribute.value().attribute] = value;
    }
  }
  for (auto &[key, value] : attributes.numAttributes){
    auto extractedAttribute = extractSpecialAttribute(key);
    if (extractedAttribute.has_value() && extractedAttribute.value().subelement == name){
      numAttributes[extractedAttribute.value().attribute] = value;
    }
  }
  for (auto &[key, value] : attributes.vecAttr.vec3){
    auto extractedAttribute = extractSpecialAttribute(key);
    if (extractedAttribute.has_value() && extractedAttribute.value().subelement == name){
      vec3[extractedAttribute.value().attribute] = value;
    }  
  }
  for (auto &[key, value] : attributes.vecAttr.vec4){
    auto extractedAttribute = extractSpecialAttribute(key);
    if (extractedAttribute.has_value() && extractedAttribute.value().subelement == name){
      vec4[extractedAttribute.value().attribute] = value;
    }   
  }
  return GameobjAttributes {
    .attr = attrs,
    .numAttributes = numAttributes,
    .vecAttr { .vec3 = vec3, .vec4 = vec4 },
  };
}

GameObjectEmitter createEmitter(GameobjAttributes& attributes, ObjectTypeUtil& util){
  GameObjectEmitter obj {};
  createAutoSerializeWithTextureLoading((char*)&obj, emitterAutoserializer, attributes, util);
  assert(obj.limit >= 0);
  
  auto emitterAttr = particleFields(attributes);
  auto allAttributes = allKeysAndAttributes(attributes);
  auto allSubmodelPaths = emitterSubmodelAttr(allAttributes);
  std::map<std::string, GameobjAttributes> submodelAttributes = {};
  for (auto &submodel : allSubmodelPaths){
    submodelAttributes[submodel] = emitterExtractAttributes(attributes, submodel);
  }
  util.addEmitter(obj.templateName, obj.rate, obj.duration, obj.limit, emitterAttr, submodelAttributes, emitterDeltas(attributes), obj.state, obj.deleteBehavior);
  return obj;
}

void removeEmitterObj(GameObjectEmitter& obj, ObjectRemoveUtil& util){
  util.rmEmitter();
}

bool setEmitterAttribute(GameObjectEmitter& emitterObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&){
  bool setAnyValue = autoserializerSetAttrWithTextureLoading((char*)&emitterObj, emitterAutoserializer, field, value, util);
  util.setEmitterEnabled(emitterObj.state);
  return setAnyValue;
}


std::optional<AttributeValuePtr> getEmitterAttribute(GameObjectEmitter& obj, const char* field){
  return getAttributePtr((char*)&obj, emitterAutoserializer, field);
}

std::vector<std::pair<std::string, std::string>> serializeEmitter(GameObjectEmitter& emitterObj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&emitterObj, emitterAutoserializer, pairs);
  return pairs;
}