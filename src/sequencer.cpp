#include "./sequencer.h"

Track createTrack(std::string name, std::vector<std::function<void()>> fns){
  Track track {
    .name = name,
    .trackFns = fns,
  };
  return track;
}
void playbackTrack(Track& track){
  std::cout << "playback track name: " << track.name << std::endl;
  std::cout << "playback track, number fns: " << track.trackFns.size() << std::endl; 
  for (auto trackFn : track.trackFns){
    trackFn();
  }
}

StateMachine createStateMachine(std::vector<State> states){
  assert(states.size() > 0);
  std::map<std::string, State> stateMapping;
  for (auto state : states){
    stateMapping[state.name] = state;
  }
  StateMachine machine {
    .initialState = states.at(0).name,
    .currentState = states.at(0).name,
    .states = stateMapping,
  };
  return machine;
}
