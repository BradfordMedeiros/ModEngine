#include "./sequencer.h"


Track createTrack(std::function<void()> fns){
  std::vector<std::function<void()>> trackFns;
  Track track {
    .trackFns = trackFns,
  };
  return track;
}
void playbackTrack(Track& track){
  std::cout << "playback track placeholder" << std::endl;
}