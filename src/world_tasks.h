#ifndef MOD_WORLDTASKS
#define MOD_WORLDTASKS

#include <glm/glm.hpp>
#include "./main_api.h"  // change this for the scene layer instead


struct RequestMovingObject {
  glm::vec3 initialPos;
  glm::vec3 finalPos;
  float initialTime;
  float duration;
};

void moveCameraTo(objid cameraId, glm::vec3 position, std::optional<float> duration);
void handleMovingObjects(float currTime);


//////////////////////////
struct ScheduledTask {
  objid ownerId;
  std::function<void(void*)> fn;
  bool realtime;
  float time;  // delay time + now 
  void* data;
};

void schedule(objid id, bool realtime, float delayTimeMs, void* data, std::function<void(void*)> fn);
void removeScheduledTask(std::set<objid> ids);
void removeScheduledTaskByOwner(std::set<objid> ids);
void tickScheduledTasks();



struct ActiveRecording {
  objid targetObj;
  Recording recording;
};
struct PlayingRecording {
  std::string recordingPath;
  float startTime;
  RecordingPlaybackType type;
  Recording recording;
  bool playInReverse;
};

void playRecording(objid id, std::string recordingPath, std::optional<RecordingPlaybackType> type, std::optional<PlayRecordingOption> recordingOption);
void stopRecording(objid id);
std::optional<RecordingState> recordingState(objid id);
float recordingLength(std::string recordingPath);
void tickRecording(float time, GameObject& gameobject);
void tickRecordings(float time);

objid createRecording(objid id);
void saveRecording(objid recordingId, std::string filepath);

#endif