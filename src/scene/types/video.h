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

struct StreamCodecs {
  AVCodecContext *videoCodec;
  AVCodecContext *audioCodec;
};

struct VideoContent {
  AVFormatContext* formatContext;
  AVFrame* avFrame;
  AVPacket *avPacket;
  StreamIndexs streamIndexs;
  StreamCodecs codecs;
};

VideoContent loadVideo(const char* videopath);
AVFrame* nextFrame(VideoContent& content);
void freeVideoContent(VideoContent& content);

#endif
