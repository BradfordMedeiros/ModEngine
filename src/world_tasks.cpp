#include "./world_tasks.h"

extern TimePlayback timePlayback; // kind of janky, probably move camera to should call into this in a more separated way
extern Stats statistics;
extern engineState state;
extern World world;
extern SysInterface interface;

std::unordered_map<objid, RequestMovingObject> requestMovingObjects; // TODO STATIC
std::vector<ScheduledTask> scheduledTasks;  // TODO STATIC
std::vector<ScheduledTask> tasksToSchedule; // // TODO STATIC taks to schedule is sepearate since want enqueue only in the tick, since task.fn can modify 

std::unordered_map<objid, ActiveRecording> activeRecordings;
std::unordered_map<objid, PlayingRecording> playingRecordings;



// moving objects 
void moveCameraTo(objid cameraId, glm::vec3 position, std::optional<float> duration){
  if (!duration.has_value()){
    setGameObjectPosition(cameraId, position, true, Hint { .hint = "moveCameraTo" });
    return;
  }
  requestMovingObjects[cameraId] = RequestMovingObject {
    .initialPos = getGameObjectPosition(cameraId, true, "moveCameraTo"),
    .finalPos = position,
    .initialTime = timePlayback.currentTime,
    .duration = duration.value(),
  };
}
void handleMovingObjects(float currTime){
  std::vector<objid> idsToRemove;
  for (auto &[id, movingObject] : requestMovingObjects){
    float elapsedTime = currTime - movingObject.initialTime;
    float percentage = elapsedTime / movingObject.duration;
    bool finished = percentage >= 1.f;
    percentage = percentage > 1.f ? 1.f : percentage;
    auto distance = percentage * (movingObject.finalPos - movingObject.initialPos);
    auto newPosition = movingObject.initialPos + distance;
    setGameObjectPosition(id, newPosition, true, Hint { .hint = "handleMovingObjects" });
    if (finished){
      idsToRemove.push_back(id);
    }
  }
  for (auto id : idsToRemove){
    requestMovingObjects.erase(id);
  }
}


/////// scheduled tasks
void schedule(objid id, bool realtime, float delayTimeMs, void* data, std::function<void(void*)> fn) {
  tasksToSchedule.push_back(ScheduledTask { 
    .ownerId = id,
    .fn = fn,
    .realtime = realtime,
    .time = (realtime ? (statistics.now * 1000 + delayTimeMs) : (timePlayback.currentTime * 1000 + delayTimeMs)),
    .data = data,
  });
}

void removeScheduledTask(std::set<objid> ids){
  std::vector<ScheduledTask> newScheduledTasks;
  for (int i = 0; i < scheduledTasks.size(); i++){
    if (ids.find(i) == ids.end()){
      newScheduledTasks.push_back(scheduledTasks.at(i));
    }
  }
  scheduledTasks = newScheduledTasks;
}
void removeScheduledTaskByOwner(std::set<objid> ids){
  std::vector<ScheduledTask> newScheduledTasks;
  for (int i = 0; i < scheduledTasks.size(); i++){
    if (ids.find(scheduledTasks.at(i).ownerId) == ids.end()){
      newScheduledTasks.push_back(scheduledTasks.at(i));
    }
  }
  scheduledTasks = newScheduledTasks; 
}

void tickScheduledTasks(){
  registerStat(statistics.numScheduledTasks, static_cast<int>(scheduledTasks.size()));
  for (auto &task : tasksToSchedule){
    scheduledTasks.push_back(task);
  }
  tasksToSchedule = {};

  std::set<objid> idsToRemove;
  float currTime = statistics.now * 1000;
  //std::cout << "num tasks: " << scheduledTasks.size() << std::endl;
  for (int i = 0; i < scheduledTasks.size(); i++){
    ScheduledTask& task = scheduledTasks.at(i);
    //std::cout << "task time: " << task.time << ", currTime = " << currTime << std::endl;
    auto shouldExecuteTask = task.realtime ? (currTime > task.time) : ((timePlayback.currentTime * 1000) > task.time);
    if (!shouldExecuteTask){
      continue;
    }
    modlog("SCHEDULER", "executing scheduled task, owner = " + std::to_string(task.ownerId));
    task.fn(task.data); // if this wasn't copied, this could screw up the loop
    idsToRemove.insert(i);  
  }
  removeScheduledTask(idsToRemove);
  //modlog("scheduled tasks", "num tasks: " + std::to_string(scheduledTasks.size()));
}

// recordings

objid createRecording(objid id){  
  auto recordingId = getUniqueObjId();
  assert(activeRecordings.find(recordingId) == activeRecordings.end());
  activeRecordings[recordingId] = ActiveRecording{
    .targetObj = id,
    .recording = createRecording(),
  };
  return recordingId;
}
void saveRecording(objid recordingId, std::string filepath){
  auto recording = activeRecordings.at(recordingId);
  std::cout << "SAVING RECORDING STARTED - " << filepath << std::endl;
  saveRecording(filepath, recording.recording, serializePropertySuffix);
  activeRecordings.erase(recordingId);
  std::cout << "SAVING RECORDING COMPLETE - " << filepath << std::endl;
}


/*
struct RecordingOptionResume{};
struct RecordingOptionResumeAtTime{
  float elapsedTime;
};
typedef std::variant<RecordingOptionResume, RecordingOptionResumeAtTime> PlayRecordingOption;*/

void playRecording(objid id, std::string recordingPath, std::optional<RecordingPlaybackType> type,  std::optional<PlayRecordingOption> recordingOption){
  bool resumeFromCurrent = false;
  std::optional<float> elapsedTime;
  if (recordingOption.has_value()){
    resumeFromCurrent = std::get_if<RecordingOptionResume>(&recordingOption.value()) != NULL;
 
    RecordingOptionResumeAtTime* resumeAtTime = std::get_if<RecordingOptionResumeAtTime>(&recordingOption.value());
    if (resumeAtTime){
      elapsedTime = resumeAtTime -> elapsedTime;
    }
  }

  auto recordingType = type.has_value() ? type.value() : RECORDING_PLAY_ONCE;
  auto isReverseRecording = recordingType == RECORDING_PLAY_ONCE_REVERSE;

  std::optional<PlayingRecording> oldRecording;
  if (playingRecordings.find(id) != playingRecordings.end() && resumeFromCurrent){
    oldRecording = playingRecordings.at(id);
  }

  stopRecording(id);

  float startTime = getTotalTime();
  if (oldRecording.has_value()){
    bool reverseTime = false;
    reverseTime = oldRecording.has_value() && oldRecording.value().playInReverse != isReverseRecording;
    if (!reverseTime){
      startTime = oldRecording.value().startTime;
    }else{
      float elapsedTime = getTotalTime() - oldRecording.value().startTime;
      float remainingTime = recordingLength(recordingPath) - elapsedTime;
      startTime = getTotalTime() - remainingTime;
    }
  }
  if (elapsedTime.has_value()){
    startTime -= elapsedTime.value();
  }

  playingRecordings[id] = PlayingRecording {
    .recordingPath = recordingPath,
    .startTime = startTime,
    .type = recordingType,
    .recording = loadRecording(recordingPath, parsePropertySuffix, interface.readFile),
    .playInReverse = isReverseRecording,
  };
}
void stopRecording(objid id){
  if (playingRecordings.find(id) != playingRecordings.end()){
    playingRecordings.erase(id);
  }
}

std::optional<RecordingState> recordingState(objid id){
  if (playingRecordings.find(id) != playingRecordings.end()){
    float timeElapsed = getTotalTime() - playingRecordings.at(id).startTime;
    return RecordingState {
      .timeElapsed = timeElapsed,
      .length = maxTimeForRecording(playingRecordings.at(id).recording),
    };
  }
  return std::nullopt;
}

float recordingLength(std::string recordingPath){
  auto recording = loadRecording(recordingPath, parsePropertySuffix, interface.readFile);
  return maxTimeForRecording(recording);
}

void tickRecordings(float time){
  for (auto &[id, activeRecording] : activeRecordings){
    auto localTransform = gameobjectTransformation(world, activeRecording.targetObj, false, "tick recordings");
    saveRecordingIndex(activeRecording.recording, "position", localTransform.position, time);
  } 

  std::vector<objid> recordingsToRemove;
  for (auto &[id, recording] : playingRecordings){
    bool isComplete = false;
    auto interpolatedProperties = recordingPropertiesInterpolated(recording.recording, time, interpolateAttribute, recording.startTime, recording.type, recording.playInReverse, &isComplete);
    if (isComplete || recording.type == RECORDING_SETONLY){
      modassert(recording.type != RECORDING_PLAY_LOOP, "recording playback - got complete loop type");
      modassert(recording.type != RECORDING_PLAY_LOOP_REVERSE, "recording playback - got complete loop type");
      recordingsToRemove.push_back(id);
    }
    for (auto &property: interpolatedProperties){
      setSingleGameObjectAttr(world, id, property.propertyName.c_str(), property.value);
    }
  }
  for (auto id : recordingsToRemove){
    stopRecording(id);
  }
}


void handleChangedResourceFiles(std::set<std::string> changedFiles){
  for (auto &file : changedFiles){
    if (getFileType(file) == IMAGE_EXTENSION){
      maybeReloadTextureWorld(world, file);
    }
  }
}
