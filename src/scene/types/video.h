#ifndef MOD_VIDEO
#define MOD_VIDEO

#include <iostream>
#include <assert.h>
#include <functional>

extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libswscale/swscale.h>
  #include <libavutil/imgutils.h>
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
  AVFrame* avFrame2;
  AVPacket *avPacket;
  StreamIndexs streamIndexs;
  StreamCodecs codecs;
  AVPixelFormat format;
};

VideoContent loadVideo(const char* videopath);
AVFrame* nextFrame(VideoContent& content);
void freeVideoContent(VideoContent& content);

#endif
