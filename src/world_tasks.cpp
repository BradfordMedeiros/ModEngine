#include "./world_tasks.h"

extern TimePlayback timePlayback; // kind of janky, probably move camera to should call into this in a more separated way
extern Stats statistics;
extern engineState state;
extern World world;
extern SysInterface interface;

extern std::unordered_map<objid, RequestMovingObject> requestMovingObjects;
extern std::vector<ScheduledTask> scheduledTasks; 
extern std::vector<ScheduledTask> tasksToSchedule; // tasks to schedule is sepearate since want enqueue only in the tick, since task.fn can modify 


// moving objects 
void moveCameraTo(objid cameraId, glm::vec3 position, std::optional<float> duration){
  if (!duration.has_value()){
    setGameObjectPosition(cameraId, position, true, Hint { .hint = "moveCameraTo" });
    return;
  }
  requestMovingObjects[cameraId] = RequestMovingObject {
    .initialPos = getGameObjectPosition(cameraId, true, "moveCameraTo"),
    .finalPos = position,
    .initialTime = timePlayback.getCurrentTime(),
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
void removeMovingObjects(objid id){
  requestMovingObjects.erase(id);
}


/////// scheduled tasks
void schedule(objid id, bool realtime, float delayTimeMs, void* data, std::function<void(void*)> fn) {
  tasksToSchedule.push_back(ScheduledTask { 
    .ownerId = id,
    .fn = fn,
    .realtime = realtime,
    .time = (realtime ? (statistics.now * 1000 + delayTimeMs) : (timePlayback.getCurrentTime() * 1000 + delayTimeMs)),
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
    auto shouldExecuteTask = task.realtime ? (currTime > task.time) : ((timePlayback.getCurrentTime() * 1000) > task.time);
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


void handleChangedResourceFiles(std::set<std::string> changedFiles){
  for (auto &file : changedFiles){
    std::cout << "watch handleChangedResourceFiles: " << file << std::endl;
    auto fileType = getFileType(file);
    if (fileType == IMAGE_EXTENSION){
      maybeReloadTextureWorld(world, file);
    }
    if (fileType == EFFEKSEEKER_EXTENSION){
      // this needs to be a relative path
      reloadEffect(file);
    }
    if (fileType == UNKNOWN_EXTENSION){
      std::cout << "watch handleChangedResourceFiles unknown file type: " << file << ", extension = " << fileType << std::endl;
    }
    //
  }
}
