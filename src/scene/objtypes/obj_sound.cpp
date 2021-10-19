#include "./obj_sound.h"

GameObjectSound createSound(GameobjAttributes& attr){
  auto clip = attr.stringAttributes.at("clip");
  auto loop = (attr.stringAttributes.find("loop") != attr.stringAttributes.end()) && (attr.stringAttributes.at("loop") == "true");
  auto source = loadSoundState(clip, loop);
  GameObjectSound obj {
    .clip = clip,
    .source = source,
    .loop = loop,
  };
  return obj;
}

void soundObjAttr(GameObjectSound& soundObj, GameobjAttributes& _attributes){
  _attributes.stringAttributes["clip"] = soundObj.clip;
  _attributes.stringAttributes["loop"] = soundObj.loop ? "true" : "false";
}