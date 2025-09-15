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
    float internalDelay;

    float currentTime;
    float lastTime;

  public:

    TimePlayback(float currentTime); 
    TimePlayback(){ };  // Do not use, just to satisfy maps 
    void play();
    void pause();
    void setTime(float elapsedTime);
    bool isPaused();

    float getCurrentTime();
    float getLastTime();
    float getDeltaTime();
};

#endif
