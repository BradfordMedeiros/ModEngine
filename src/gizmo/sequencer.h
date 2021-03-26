#ifndef MOD_SEQUENCER
#define MOD_SEQUENCER

#include <string>
#include <iostream>
#include <vector>
#include <map>
#include <functional>
#include <variant>
#include <assert.h>
#include "../common/util.h"

struct Track {
  std::string name;
  std::vector<std::function<void()>> trackFns;
};
struct ExitCallback {
  std::vector<std::function<void()>> exitFns;
};

struct State {
  std::string name;
  std::string defaultTrack;
  std::map<std::string, std::string> attributes;
  std::map<std::string, Track> tracks;
  ExitCallback onExit;
};

struct StateMachine {
  std::string currentState;
  std::string currentTrack;
  int trackIndex;
  std::map<std::string, State> states;
};

typedef std::variant<Track, std::function<void()>> StateMachineItem;

Track createTrack(std::string name, std::vector<std::function<void()>> fns);
void playbackTrack(Track& track);

StateMachine createStateMachine(std::vector<State> states);
void playStateMachine(StateMachine* machine, objid id);
void removeStateMachines(objid id);
void setStateMachine(StateMachine* machine, std::string newState);
void processStateMachines();


/*


(machine (list 
  (state "open (list 
    (track "sometrack" (list + + +))
  ))
  (state "closed")
))
 

  (define mainmachine 
    (create-machine
      (state closed 
        (attributes (color "blue"))     ; this should set the color attribute to blue, but then set it back when not in closed state
        (track "opening door" 
          (display "opening door")
          (play-animation open-door-animation)     
          (trigger "door-opened")
        )
        (track "play shake head"
          (wait-trigger "door-opened") 
          (display "player shaking head")
          (play-animation player-shake-head)
          (go-state open)
        )
        (track "music"
          (play-sound-clip "hello")
          (play-sound-clip "world")   ; this can get preempted, if go-state open happens before. 
        )
      )
      (state open

      )
    )
  )

  (play-machine mainmachine)
*/

#endif