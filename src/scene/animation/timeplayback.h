#ifndef MODENGINE_TIMEPLAYBACK
#define MODENGINE_TIMEPLAYBACK

#include <functional>
#include <math.h>

enum EndBehavior {
    PAUSE,         
    RESTART  
};
    
// future is + and past is -
class TimePlayback {
  private:
    float currentTime;
    float beginTime;
    float duration;
    bool paused;
    EndBehavior endBehavior;

    std::function<void(float, float)> onFrame;
    bool hasRemainingTime();
    
  public:
    TimePlayback(float currentTime, std::function<void(float, float)> onFrame, float duration = INFINITY, EndBehavior behavior = RESTART); 
    TimePlayback(){ };  // Do not use, just to satisfy maps 
    void play();
    void pause();
    void setElapsedTime(float elapsedTime);
    bool isPaused();
};

#endif
