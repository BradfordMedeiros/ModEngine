#ifndef MODENGINE_TIMEPLAYBACK
#define MODENGINE_TIMEPLAYBACK

#include <functional>
#include <math.h>
#include <iostream>
#include <assert.h>

// future is + and past is -
class TimePlayback {
  private:
    bool paused;

  public:
    float currentTime;
    TimePlayback(float currentTime); 
    TimePlayback(){ };  // Do not use, just to satisfy maps 
    void play();
    void pause();
    void setTime(float elapsedTime);
    bool isPaused();
};

#endif
