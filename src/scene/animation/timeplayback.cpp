#include "./timeplayback.h"

TimePlayback::TimePlayback(float currentTime){
  this -> currentTime = currentTime;
  this -> paused = false;
}

void TimePlayback::play(){
  this -> paused = false;
}
void TimePlayback::pause(){
  this -> paused = true;
}
bool TimePlayback::isPaused() {
  return this -> paused;
}

void TimePlayback::setTime(float time){ 
  if (!this -> paused){
    this -> currentTime = time;
  }
}


