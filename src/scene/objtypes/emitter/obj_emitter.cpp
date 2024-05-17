#include "./obj_emitter.h"

EmitterSystem emitterSystem { 
  .emitters = {}, 
  .additionalParticlesToRemove = {} 
};

EmitterSystem& getEmitterSystem(){
  return emitterSystem;
}


struct ParsedEmitterKey {
  char fieldIdentifier;
  std::optional<int> frameIndex;
  std::string key;
};

std::optional<ParsedEmitterKey> parseEmitterKey(std::string key){
  if (!(key.at(0) == '+' || key.at(0) == '!')){
    return std::nullopt;
  }
  auto newKey = key.substr(1, key.size());
  auto stringPair = carAndRest(newKey, '-');  // !position:0 0 0 or !0-position:0 0 0
  if (stringPair.first.size() > 0 && stringPair.second.size() > 0){
    int frameIndex = std::atoi(stringPair.first.c_str());
    return ParsedEmitterKey {
      .fieldIdentifier = key.at(0),
      .frameIndex = frameIndex,
      .key = stringPair.second,
    };
  }

  return ParsedEmitterKey {
    .fieldIdentifier = key.at(0),
    .frameIndex = std::nullopt,
    .key = newKey,
  };
}


struct ParsedEmitterValue {
  char fieldIdentifier;
  std::string key;
  std::optional<int> frameIndex;
  AttributeValue value;
};

std::vector<ParsedEmitterValue> parseEmitterValues(GameobjAttributes& attributes){
  std::vector<ParsedEmitterValue> values;
  for (auto &[key, value] : attributes.attr){
    auto parsedEmitterKey = parseEmitterKey(key);
    if (!parsedEmitterKey.has_value()){
      continue;
    }
    values.push_back(ParsedEmitterValue {
      .fieldIdentifier = parsedEmitterKey.value().fieldIdentifier,
      .key = parsedEmitterKey.value().key,
      .frameIndex = parsedEmitterKey.value().frameIndex,
      .value = value,
    });
  }
  return values;
}

GameobjAttributes mainParticleFrame(std::vector<ParsedEmitterValue>& parsedValues, char fieldIdentifier){
  GameobjAttributes attr {};
  for (auto &parsedValue : parsedValues){
    if (parsedValue.fieldIdentifier == fieldIdentifier && !parsedValue.frameIndex.has_value()){
      attr.attr[parsedValue.key] = parsedValue.value;
    }
  }
  return attr;
}

std::map<int, GameobjAttributes> getFrameToAttr(GameobjAttributes& attributes, char fieldIdentifier){
  // todo - intentionally sort this at the end, 
  // dont like depending on the sorting of map, too inexplicit + will be bulk updating this
  std::map<int, GameobjAttributes> frameToAttr;
  auto parsedValues = parseEmitterValues(attributes);
  auto mainAttrs = mainParticleFrame(parsedValues, fieldIdentifier);
  frameToAttr[0] = mainAttrs;
  for (auto &parsedValue : parsedValues){
    if (parsedValue.fieldIdentifier == fieldIdentifier && parsedValue.frameIndex.has_value()){
      frameToAttr[parsedValue.frameIndex.value()] = mainAttrs;      
    }
  }

  for (auto &parsedValue : parsedValues){
    if (parsedValue.fieldIdentifier == fieldIdentifier){
      int frameIndex = parsedValue.frameIndex.has_value() ? parsedValue.frameIndex.value() : 0;
      frameToAttr.at(frameIndex).attr[parsedValue.key] = parsedValue.value;
    }
  }
  return frameToAttr;
}

std::vector<ParticleAttributeFrame> particleFieldFrames(GameobjAttributes& attributes){
  auto frameToAttr = getFrameToAttr(attributes, '+');
  std::vector<ParticleAttributeFrame> frames;
  for (auto &[frame, attr] : frameToAttr){
    frames.push_back(ParticleAttributeFrame {
      .frame = frame,
      .attr = attr,
    });
  }
  return frames;
}

std::vector<EmitterDelta> emitterDeltas(GameobjAttributes& attributes){
  std::vector<EmitterDelta> deltas;
  for (auto &[key, value] : attributes.attr){
    deltas.push_back(EmitterDelta {
      .attributeName = key,
      .value = value,
    });
    
  }
  return deltas;
}

std::vector<EmitterDeltaFrame> emitterDeltasFrames(GameobjAttributes& attributes){
  auto frameToAttr = getFrameToAttr(attributes, '!');
  std::vector<EmitterDeltaFrame> frames;
  for (auto &[frame, attr] : frameToAttr){
    frames.push_back(EmitterDeltaFrame {
      .frame = frame,
      .deltas = emitterDeltas(attr),
    });
  }
  return frames;
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
    .structOffset = offsetof(GameObjectEmitter, numParticlesPerFrame),
    .field = "count",
    .defaultValue = 1,
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
};


bool isEmitterPrefix(char character){
  return character == '!' || character == '+';
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

  for (auto &[key, value] : attributes.attr){
    auto extractedAttribute = extractSpecialAttribute(key);
    if (extractedAttribute.has_value() && extractedAttribute.value().subelement == name){
      attrs[extractedAttribute.value().attribute] = value;
    }
  }
  return GameobjAttributes { .attr = attrs };
}

GameObjectEmitter createEmitter(GameobjAttributes& attributes, ObjectTypeUtil& util){
  GameObjectEmitter obj {};
  createAutoSerializeWithTextureLoading((char*)&obj, emitterAutoserializer, attributes, util);
  assert(obj.limit >= 0);
  
  auto allAttributes = allKeysAndAttributes(attributes);
  auto allSubmodelPaths = emitterSubmodelAttr(allAttributes);
  std::map<std::string, GameobjAttributes> submodelAttributes = {};
  for (auto &submodel : allSubmodelPaths){
    submodelAttributes[submodel] = emitterExtractAttributes(attributes, submodel);
  }

  ParticleConfig particleConfig {
    .particleAttributes = particleFieldFrames(attributes),
    .submodelAttributes = {
      SubmodelAttributeFrame {
        .frame = 0,
        .attr = submodelAttributes,
      },
      SubmodelAttributeFrame {
        .frame = 1,
        .attr = submodelAttributes,
      },
      SubmodelAttributeFrame {
        .frame = 2,
        .attr = submodelAttributes,
      },
      SubmodelAttributeFrame {
        .frame = 3,
        .attr = submodelAttributes,
      },
      SubmodelAttributeFrame {
        .frame = 4,
        .attr = submodelAttributes,
      },
      SubmodelAttributeFrame {
        .frame = 5,
        .attr = submodelAttributes,
      },
    },
    .deltas = emitterDeltasFrames(attributes),
  };
  addEmitter(emitterSystem, util.id, util.getCurrentTime(), obj.limit, obj.rate, obj.duration, obj.numParticlesPerFrame, particleConfig, obj.state, obj.deleteBehavior);
  return obj;
}

void removeEmitterObj(GameObjectEmitter& obj, ObjectRemoveUtil& util){
  removeEmitter(emitterSystem, util.id);
}

bool setEmitterAttribute(GameObjectEmitter& emitterObj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&){
  bool setAnyValue = autoserializerSetAttrWithTextureLoading((char*)&emitterObj, emitterAutoserializer, field, value, util);
  std::string fieldName(field);
  if (fieldName == "state"){
    updateEmitterOptions(emitterSystem, util.id, EmitterUpdateOptions {
      .enabled = emitterObj.state,
    });
    setAnyValue = true;
  }else if (fieldName == "rate"){
    updateEmitterOptions(emitterSystem, util.id, EmitterUpdateOptions {
      .spawnrate = emitterObj.rate,
    });
    setAnyValue = true;
  }else if (fieldName == "limit"){
    updateEmitterOptions(emitterSystem, util.id, EmitterUpdateOptions {
      .targetParticles = emitterObj.limit,
    });
    setAnyValue = true;
  }else if (fieldName == "duration"){
    updateEmitterOptions(emitterSystem, util.id, EmitterUpdateOptions {
      .lifetime = emitterObj.duration,
    });
    setAnyValue = true;
  }else {
    GameobjAttributes attributes {
      .attr = {
        { field, value },
      },
    };
    //auto particleAttributes = particleFields(attributes);
    //if (particleAttributes.attr.size() > 0){
    //  updateEmitterOptions(emitterSystem, util.id, EmitterUpdateOptions {
    //    .particleAttributes = particleAttributes,
    //  });
    //  //setAnyValue = true;
    //  return setAnyValue;
    //}

    auto deltas = emitterDeltas(attributes);
    if (deltas.size() > 0){
      EmitterDelta requestedDelta = deltas.at(0);
      int frameIndex = 0;
      auto newDeltaPtr = getEmitterDelta(emitterSystem, util.id, requestedDelta.attributeName, frameIndex);
      auto newDelta = newDeltaPtr.has_value() ? *(newDeltaPtr.value()) : requestedDelta;
      if (newDeltaPtr.has_value()){
        if (fieldName.at(0) == '!'){
          newDelta.value = requestedDelta.value;
        }
      }
      updateEmitterOptions(emitterSystem, util.id, EmitterUpdateOptions {
        .delta = newDelta,
      });
      return setAnyValue;
    }


  }
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

