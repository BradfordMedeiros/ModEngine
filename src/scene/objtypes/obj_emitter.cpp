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

struct ValueVariance {
  glm::vec3 value;
  glm::vec3 variance;
  std::vector<float> lifetimeEffect;
};
std::vector<EmitterDelta> emitterDeltas(GameobjAttributes& attributes){
  std::map<std::string, ValueVariance> values;
  ///////////////// Same code for diff types consolidate
  for (auto [key, _] : attributes.vecAttr.vec3){
    if ((key.at(0) == '!' || key.at(0) == '?' || key.at(0) == '%') && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      values[newKey] = ValueVariance {
        .value = glm::vec4(0.f, 0.f, 0.f, 0.f),
        .variance = glm::vec4(0.f, 0.f, 0.f, 0.f),
        .lifetimeEffect = {},
      };
    }
  }
  for (auto [key, _] : attributes.vecAttr.vec4){
    if ((key.at(0) == '!' || key.at(0) == '?' || key.at(0) == '%') && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      values[newKey] = ValueVariance {
        .value = glm::vec4(0.f, 0.f, 0.f, 0.f),
        .variance = glm::vec4(0.f, 0.f, 0.f, 0.f),
        .lifetimeEffect = {},
      };
    }
  }
  ////////////////////////////////////////////////////////////
  
  ///////////////// Same code for diff types
  for (auto [key, value] : attributes.vecAttr.vec3){
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
  return deltas;
}

GameObjectEmitter createEmitter(GameobjAttributes& attributes, ObjectTypeUtil& util){
  GameObjectEmitter obj {};
  float spawnrate = attributes.numAttributes.find("rate") != attributes.numAttributes.end() ? attributes.numAttributes.at("rate") : 1.f;
  float lifetime = attributes.numAttributes.find("duration") != attributes.numAttributes.end() ? attributes.numAttributes.at("duration") : 10.f;
  int limit = attributes.numAttributes.find("limit") != attributes.numAttributes.end() ? attributes.numAttributes.at("limit") : 10;
  auto enabled = attributes.stringAttributes.find("state") != attributes.stringAttributes.end() ? !(attributes.stringAttributes.at("state") == "disabled") : true;
  assert(limit >= 0);
  
  auto deleteValueStr = attributes.stringAttributes.find("onremove") != attributes.stringAttributes.end() ? attributes.stringAttributes.at("onremove") : "delete";
  auto deleteType = EMITTER_DELETE;
  if (deleteValueStr == "orphan"){
    deleteType = EMITTER_ORPHAN;
  }
  if (deleteValueStr == "finish"){
    deleteType = EMITTER_FINISH;
  }

  auto emitterAttr = particleFields(attributes);
  util.addEmitter(spawnrate, lifetime, limit, emitterAttr, emitterDeltas(attributes), enabled, deleteType);
  return obj;
}

void removeEmitter(GameObjectEmitter& heightmapObj, ObjectRemoveUtil& util){
  util.rmEmitter();
}

void setEmitterAttributes(GameObjectEmitter& emitterObj, GameobjAttributes& attributes, ObjectSetAttribUtil& util){
  auto enabled = attributes.stringAttributes.find("state") != attributes.stringAttributes.end() ? !(attributes.stringAttributes.at("state") == "disabled") : true;
  util.setEmitterEnabled(enabled);
}