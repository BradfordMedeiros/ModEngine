#ifndef MOD_MOCAP
#define MOD_MOCAP

#include <vector>
#include <glm/glm.hpp>
#include "./common/util.h"

struct MocapMarker {
  objid id;
  glm::vec3 color;
};

struct MocapDetection {
  objid marker;
  glm::vec3 position;
};

struct MocapDetectionTime {
  float time;
  MocapDetection detection;
};

struct MocapRecording {
  std::vector<MocapDetectionTime> detections;
};

MocapRecording createMocapRecording();
std::vector<MocapDetection> identifyMocapBalls(std::vector<MocapMarker> markers);
void addToMocapRecording(MocapRecording& recording, MocapDetection& detection);
std::string serializeMocapRecording(MocapRecording& recording);

#endif