#ifndef MODENGINE_TIMEPLAYBACK
#define MODENGINE_TIMEPLAYBACK

#include <functional>

class TimePlayback {
  private:
    float pausedTime;
    float currentTime;
    bool isPaused;
    std::function<void(float, float)> onFrame;
  public:
    TimePlayback(float currentTime, std::function<void(float, float)> onFrame); 
    void play();
    void pause();
    void setElapsedTime(float elapsedTime);

};

#endif
