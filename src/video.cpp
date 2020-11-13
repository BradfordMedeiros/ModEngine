#include "./video.h"

struct StreamIndexs {
  int video;
  int audio;
};

StreamIndexs getStreamIndexs(AVFormatContext *formatContext) {
  auto numStreams = formatContext -> nb_streams;
  int videoStream = -1;
  int audioStream = -1;
  for (int i = 0; i < numStreams; i++){
    AVCodecParameters* codecParameters = formatContext -> streams[i] -> codecpar;
    AVMediaType mediaType = codecParameters -> codec_type;
    if (mediaType == AVMEDIA_TYPE_VIDEO){
      assert(videoStream == -1);
      videoStream = i;
    }else if (mediaType == AVMEDIA_TYPE_AUDIO){ 
      assert(audioStream == -1);
      audioStream = i;
    }else{
      std::cout << "ERROR: VIDEO: UNHANDLED MEDIA TYPE" << std::endl;
      assert(false);
    }
  }
  StreamIndexs streams {
    .video = videoStream,
    .audio = audioStream,
  };  
  return streams;
}

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
  auto streamIndexs = getStreamIndexs(formatContext);

  std::cout << "video: " << videopath << " is: " << formatContext -> duration << std::endl;
  std::cout << "number video streams: " << numStreams << std::endl;
  std::cout << "video index: " << streamIndexs.video << ", audio index:" << streamIndexs.audio << std::endl;

  for (int i = 0; i < numStreams; i++){
    AVCodecParameters* codecParameters = formatContext -> streams[i] -> codecpar;
    std::cout << "bit rate: " << codecParameters -> bit_rate << std::endl;
    std::cout << "width: " << codecParameters -> width << std::endl;
    std::cout << "height: " << codecParameters -> height << std::endl;

    AVCodec* localCodec = avcodec_find_decoder(codecParameters -> codec_id);
    assert(localCodec != NULL);
    std::cout << "INFO: video: found decoder for codec" << std::endl;

    AVFrame *avFrame = av_frame_alloc();
    assert(avFrame);

    AVPacket *avPacket = av_packet_alloc();
    assert(avPacket);

    AVCodecContext *codecContext = avcodec_alloc_context3(localCodec);
    assert(codecContext);

    auto readValue = av_read_frame(formatContext, avPacket);
    if (readValue < 0){
      std::cout << "INFO: video: error or end of video" << std::endl;
      if (i == streamIndexs.video){
        auto sendValue = avcodec_send_packet(codecContext, avPacket);
        std::cout << "ERROR: VIDEO: sending packet to decoding" << std::endl;
        assert(sendValue >= 0);

        auto receiveValue = avcodec_receive_frame(codecContext, avFrame);
        if (receiveValue != 0){
          if (receiveValue == AVERROR_EOF){ // not actually an error
            std::cout << "ERROR: VIDEO: LAST FRAME" << std::endl;
          }
          assert(false);
        }
      }

    }
    avcodec_free_context(&codecContext);
    av_packet_free(&avPacket);
    av_frame_free(&avFrame);
    std::cout << std::endl;
  }
  avformat_close_input(&formatContext);

/*  
  avformat_close_input(&formatContext);*/

  assert(false);
}

