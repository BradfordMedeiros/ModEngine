#ifndef MOD_VIDEO
#define MOD_VIDEO

#include <iostream>
#include <assert.h>
#include <functional>

extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
}

void save_frame(unsigned char *buf, int wrap, int xsize, int ysize, const char *filename);
void testvideo(std::function<bool(AVFrame* frame)> onFrame);

#endif
