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

  auto streamOpenValue = avformat_find_stream_info(formatContext, NULL);
  if (streamOpenValue < 0){
    std::cout << "ERROR: could not find stream info" << std::endl;
    assert(false);
  }

  auto numStreams = formatContext -> nb_streams;

  std::cout << "video: " << videopath << " is: " << formatContext -> duration << std::endl;
  std::cout << "number video streams: " << numStreams << std::endl;

  for (int i = 0; i < numStreams; i++){
    std::cout << "stream number: " << i << std::endl;
    AVCodecParameters* codecParameters = formatContext -> streams[i] -> codecpar;
    std::cout << "bit rate: " << codecParameters -> bit_rate << std::endl;
    std::cout << "width: " << codecParameters -> width << std::endl;
    std::cout << "height: " << codecParameters -> height << std::endl;

    AVMediaType mediaType = codecParameters -> codec_type;
    if (mediaType == AVMEDIA_TYPE_VIDEO){
      std::cout << "media type: video" << std::endl;
    }else if (mediaType == AVMEDIA_TYPE_AUDIO){ 
      std::cout << "media type: audio" << std::endl;
    }else{
      std::cout << "ERROR: VIDEO: UNHANDLED MEDIA TYPE" << std::endl;
      assert(false);
    }

    AVCodec* localCodec = avcodec_find_decoder(codecParameters -> codec_id);
    assert(localCodec != NULL);
    std::cout << "INFO: video: found decoder for codec" << std::endl;

    std::cout << std::endl;
  }
/*  
  avformat_close_input(&formatContext);*/

  assert(false);
}

