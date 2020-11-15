#ifndef MOD_VIDEO
#define MOD_VIDEO

#include <iostream>
#include <assert.h>
#include <functional>

extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
}

struct StreamIndexs {
  int video;
  int audio;
};

struct VideoContent {
  AVFormatContext* formatContext;
  AVFrame* avFrame;
  StreamIndexs streamIndexs;
  AVCodecContext *videoCodec;
  AVCodecContext *audioCodec;
};

void loadVideo(std::function<bool(AVFrame* frame)> onFrame);
AVFrame* nextFrame(VideoContent* content);
void freeVideoContent(VideoContent& content);

#endif
