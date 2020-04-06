#include "./timeplayback.h"

TimePlayback::TimePlayback(float currentTime, std::function<void(float, float)> onFrame, float duration, EndBehavior behavior){
  this -> beginTime = currentTime;
  this -> currentTime = currentTime;
  this -> paused = false;
  this -> onFrame = onFrame;
  this -> duration = duration;
  this -> endBehavior = behavior;
}

bool TimePlayback::hasRemainingTime(){
  float totalElapsedTime = this -> currentTime - this -> beginTime;
  float remainingTime = this -> duration - totalElapsedTime;
  return remainingTime > 0;
}

void TimePlayback::play(){
  if (this -> hasRemainingTime()){
    this -> paused = false;
  }
}
void TimePlayback::pause(){
  this -> paused = true;
}
bool TimePlayback::isPaused() {
  return this -> paused;
}

void TimePlayback::setElapsedTime(float elapsedTime){
  if (!this -> paused){
    this -> currentTime = this -> currentTime + elapsedTime;
    this -> onFrame(this -> currentTime, elapsedTime); 
  
    float totalElapsedTime = this -> currentTime - this -> beginTime;
    float remainingTime = this -> duration - totalElapsedTime;

    if (!this -> hasRemainingTime()){
      if (this ->endBehavior == PAUSE){
        this -> pause();
      }else{
        this -> currentTime = this -> beginTime;
      }
    }
  }
}

