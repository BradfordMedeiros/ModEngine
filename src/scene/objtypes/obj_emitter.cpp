#include "./obj_emitter.h"

GameobjAttributes particleFields(GameobjAttributes& attributes){
  GameobjAttributes attr {
    .stringAttributes = {},
    .numAttributes = {},
    .vecAttr = { .vec3 = {}, .vec4 = {} },
  };
  for (auto [key, value] : attributes.stringAttributes){
    if (key.at(0) == '+' && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      attr.stringAttributes[newKey] = value;
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
        .value = glm::vec4(0.f, 0.f, 0.f, 0.f),
        .variance = glm::vec4(0.f, 0.f, 0.f, 0.f),
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

GameObjectEmitter createEmitter(GameobjAttributes& attributes, ObjectTypeUtil& util){
  GameObjectEmitter obj {};
  createAutoSerializeWithTextureLoading((char*)&obj, emitterAutoserializer, attributes, util);
  assert(obj.limit >= 0);
  
  auto emitterAttr = particleFields(attributes);
  util.addEmitter(obj.templateName, obj.rate, obj.duration, obj.limit, emitterAttr, emitterDeltas(attributes), obj.state, obj.deleteBehavior);
  return obj;
}

void removeEmitterObj(GameObjectEmitter& obj, ObjectRemoveUtil& util){
  util.rmEmitter();
}

bool setEmitterAttributes(GameObjectEmitter& emitterObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  autoserializerSetAttrWithTextureLoading((char*)&emitterObj, emitterAutoserializer, attributes, util);
  util.setEmitterEnabled(emitterObj.state);
  return false;
}

void emitterObjAttr(GameObjectEmitter& emitterObj, GameobjAttributes& _attributes){
  autoserializerGetAttr((char*)&emitterObj, emitterAutoserializer, _attributes);
}

std::vector<std::pair<std::string, std::string>> serializeEmitter(GameObjectEmitter& emitterObj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&emitterObj, emitterAutoserializer, pairs);
  return pairs;
}