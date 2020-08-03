#include "./sequencer.h"

Track createTrack(std::string name, std::vector<std::function<void()>> fns){
  Track track {
    .name = name,
    .trackFns = fns,
  };
  return track;
}
void playbackTrack(Track& track){
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

void playStateMachine(StateMachine& machine){
  std::cout << "states in machine: " << machine.states.size() << std::endl;
  for (auto [name, state] : machine.states){
    std::cout << "state name:  " << name << std::endl;
  }
}

void setStateMachine(StateMachine& machine, std::string newState){
  machine.currentState = newState;
}


std::vector<StateMachine> activeMachines;
void processStateMachines(){
  for (auto machine : activeMachines){
    State& activeState = machine.states.at(machine.currentState);
  }
} 