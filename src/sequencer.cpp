#include "./sequencer.h"


Track createTrack(std::string name, std::vector<std::function<void()>> fns){
  std::vector<std::function<void()>> trackFns;
  Track track {
    .name = name,
    .trackFns = trackFns,
  };
  return track;
}
void playbackTrack(Track& track){
  std::cout << "playback track, number fns: " << track.trackFns.size() << std::endl; 
  for (auto trackFn : track.trackFns){
    trackFn();
  }
}