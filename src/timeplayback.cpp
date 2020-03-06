#include "./timeplayback.h"

TimePlayback::TimePlayback(float currentTime, std::function<void(float, float)> onFrame){
  this -> currentTime = currentTime;
  this -> isPaused = false;
  this -> onFrame = onFrame;
}

void TimePlayback::play(){
  this -> isPaused = false;
}

void TimePlayback::pause(){
  this -> isPaused = true;
}

void TimePlayback::setElapsedTime(float elapsedTime){
  if (!this -> isPaused){
    this -> currentTime = this -> currentTime + elapsedTime;
    this -> onFrame(this -> currentTime, elapsedTime); 
  }
}

