#ifndef MOD_WEBM
#define MOD_WEBM

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <webm/webm_parser.h>
#include <webm/file_reader.h>
#include <webm/webm_parser.h>
#include <webm/callback.h>
#include <webm/istream_reader.h>

#include <webm/mkvparser/mkvparser.h>


#include "./common/util.h"


extern "C" {
  #include <libavcodec/avcodec.h>
  #include <libavformat/avformat.h>
  #include <libswscale/swscale.h>
  #include <libavutil/imgutils.h>
  #include <libswresample/swresample.h>
}

struct StreamIndexs {
  int video;
  int audio;
};

struct StreamCodecs {
  AVCodecContext* videoCodec;
  AVCodecContext* audioCodec;
};

struct VideoContent {
  AVFormatContext* formatContext;
  AVFrame* avFrame;
  AVFrame* avFrame2;
  AVPacket *avPacket;
  StreamIndexs streamIndexs;
  StreamCodecs codecs;
  AVPixelFormat format;
  float videoTimestamp;
};

VideoContent loadVideo(const char* videopath);
int nextFrame(VideoContent& content);
void seekVideo(VideoContent& content);
void pauseVideo(VideoContent& content);
void resumeVideo(VideoContent& content);
void freeVideoContent(VideoContent& content);


void webmTest();

#endif