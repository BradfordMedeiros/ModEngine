#include "./timeplayback.h"

const float maxFrameTime = 0.1f;

TimePlayback::TimePlayback(float currentTime){
  this -> currentTime = currentTime;
  this -> lastTime = currentTime;
  this -> paused = false;
  this -> internalDelay = 0.f;
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
  if (this -> paused){
    return;
  }

  float adjustedTime = time - this -> internalDelay;
  float elapsedTime = adjustedTime - this -> currentTime;

  if (elapsedTime > maxFrameTime){
    float slowdown = elapsedTime - maxFrameTime;
    this -> internalDelay += slowdown;
  }
  
  this -> lastTime = this -> currentTime;
  this -> currentTime = time - this -> internalDelay;
}

float TimePlayback::getCurrentTime(){
  return this -> currentTime;
}
float TimePlayback::getLastTime(){
  return this -> lastTime;
}


float TimePlayback::getDeltaTime(){
  return this -> currentTime - this -> lastTime;
}


