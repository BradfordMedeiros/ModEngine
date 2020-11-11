#ifndef MOD_VIDEO
#define MOD_VIDEO

#include <iostream>
#include <assert.h>

extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
}

void testvideo();

#endif
