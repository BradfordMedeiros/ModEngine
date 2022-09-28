#ifndef MODENGINE_TIMEPLAYBACK
#define MODENGINE_TIMEPLAYBACK

#include <functional>
#include <math.h>
#include <iostream>
#include <assert.h>

enum EndBehavior {
    PAUSE,         
    RESTART  
};
    
// future is + and past is -
class TimePlayback {
  private:
    float beginTime;
    float duration;
    bool paused;
    EndBehavior endBehavior;

    std::function<void(float, float)> onFrame;
    std::function<void()> onFinish;
    bool hasRemainingTime();
    
  public:
    float currentTime;
    TimePlayback(float currentTime, std::function<void(float, float)> onFrame,  std::function<void()> onFinish, float duration = INFINITY, EndBehavior behavior = RESTART); 
    TimePlayback(){ };  // Do not use, just to satisfy maps 
    void play();
    void pause();
    void setElapsedTime(float elapsedTime);
    bool isPaused();
};

#endif
