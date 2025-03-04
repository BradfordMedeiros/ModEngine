#include "./world_tasks.h"

extern TimePlayback timePlayback; // kind of janky, probably move camera to should call into this in a more separated way

std::unordered_map<objid, RequestMovingObject> requestMovingObjects; // TODO STATIC
void moveCameraTo(objid cameraId, glm::vec3 position, std::optional<float> duration){
  if (!duration.has_value()){
    setGameObjectPosition(cameraId, position, true);
    return;
  }
  requestMovingObjects[cameraId] = RequestMovingObject {
    .initialPos = getGameObjectPosition(cameraId, true),
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
    setGameObjectPosition(id, newPosition, true);
    if (finished){
      idsToRemove.push_back(id);
    }
  }
  for (auto id : idsToRemove){
    requestMovingObjects.erase(id);
  }
}

