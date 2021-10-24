#include "./obj_emitter.h"

GameobjAttributes particleFields(GameobjAttributes& attributes){
  GameobjAttributes attr {
    .stringAttributes = {},
    .numAttributes = {},
    .vecAttributes = {},
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
  for (auto [key, value] : attributes.vecAttributes){
    if (key.at(0) == '+' && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      attr.vecAttributes[newKey] = value;
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
  for (auto [key, _] : attributes.vecAttributes){
    if ((key.at(0) == '!' || key.at(0) == '?' || key.at(0) == '%') && key.size() > 1){
      auto newKey = key.substr(1, key.size());
      values[newKey] = ValueVariance {
        .value = glm::vec3(0.f, 0.f, 0.f),
        .variance = glm::vec3(0.f, 0.f, 0.f),
        .lifetimeEffect = {},
      };
    }
  }
  
  for (auto [key, value] : attributes.vecAttributes){
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