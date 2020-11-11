#include "./video.h"

void testvideo(){
  av_register_all();

  // https://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
  AVFormatContext *formatContext = avformat_alloc_context();
  if (!formatContext){
    std::cout << "ERROR: Could not allocate memeory for avformatcontext" << std::endl;
    assert(false);
  }
  
  auto videopath = "./res/videos/bunny.mkv";
  auto openValue = avformat_open_input(&formatContext, videopath, NULL, NULL);
  if (openValue != 0){
    std::cout << "ERROR: could not open video: " << videopath << std::endl;
    assert(false);
  }

  std::cout << "video: " << videopath << " is: " << formatContext -> duration << std::endl;
/*
  avformat_close_input(&formatContext);*/

}

