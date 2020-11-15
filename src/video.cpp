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

  assert(videoStream != -1);
  assert(audioStream != -1);
  StreamIndexs streams {
    .video = videoStream,
    .audio = audioStream,
  };  
  return streams;
}

void printFrameInfo(AVFrame* avFrame){
  std::cout << "------------------------------" << std::endl;
  std::cout << "frame width: " << avFrame -> width << std::endl;
  std::cout << "frame height: " << avFrame -> height << std::endl;
  std::cout << "frame format: " << avFrame -> format << std::endl;
  std::cout << "frame pict type: " << av_get_picture_type_char(avFrame -> pict_type) << std::endl;;
  std::cout << "frame key frame: " << avFrame -> key_frame << std::endl;
  std::cout << "frame line size: " << avFrame -> linesize[0] << std::endl;
  std::cout << "------------------------------" << std::endl;
}

void save_frame(unsigned char *buf, int wrap, int xsize, int ysize, const char *filename){
    FILE *f;
    int i;
    f = fopen(filename,"w");
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);

    // writing line by line
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}

void readFrame(AVFormatContext* formatContext, AVPacket* avPacket, AVCodecContext* codecContext, AVFrame* avFrame, StreamIndexs& streams){
  auto readValue = av_read_frame(formatContext, avPacket);
  std::cout << "INFO: VIDEO: read frame: " << readValue << std::endl;
  if (readValue == 0 && avPacket -> stream_index == streams.video){
    std::cout << "INFO: VIDEO: stream index: " << avPacket -> stream_index << std::endl;
    auto sendValue = avcodec_send_packet(codecContext, avPacket);
    if (sendValue ==  AVERROR_EOF){
      std::cout << "send packet: end of field" << std::endl;
    }else if (sendValue ==  AVERROR(EAGAIN)){
      std::cout << "send packet: averror(eagain)" << std::endl;
    }else if (sendValue == AVERROR(EINVAL)){
      std::cout << "send packet: averror(einval)" << std::endl;
    }else if (sendValue ==  AVERROR(ENOMEM)){
      std::cout << "send packet: averror(ENOMEM)" << std::endl;
    }
    std::cout << "send value: " << sendValue << std::endl;
    assert(sendValue == 0);
    auto receiveValue = avcodec_receive_frame(codecContext, avFrame);
    if (receiveValue != 0){
      if (receiveValue == AVERROR_EOF){ // not actually an error
        std::cout << "ERROR: VIDEO: LAST FRAME" << std::endl;
      }
      assert(false);
    }
    printFrameInfo(avFrame);
    std::cout << "video: frame index: " << codecContext -> frame_number << std::endl;  
  }else if (avPacket -> stream_index != streams.video){
    std::cout << "INFO: video: not video, skipping (" << avPacket -> stream_index << ")" << std::endl;
  }else{
    std::cout << "INFO: video: READ error or end of video" << std::endl;
    assert(false);
  } 
  av_packet_unref(avPacket);
}

void testvideo(std::function<bool(AVFrame* frame)> onFrame){
  av_register_all();

  // https://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
  AVFormatContext *formatContext = avformat_alloc_context();
  if (!formatContext){
    std::cout << "ERROR: Could not allocate memeory for avformatcontext" << std::endl;
    assert(false);
  }
  
  auto videopath = "./res/videos/bunny.avi";
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

  av_dump_format(formatContext, 0, videopath, 0);

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
    std::cout << "INFO: video: found decoder for codec: " << codecParameters -> codec_id << std::endl;

    AVFrame *avFrame = av_frame_alloc();
    assert(avFrame);

    AVPacket *avPacket = av_packet_alloc();
    assert(avPacket);

    AVCodecContext *codecContext = avcodec_alloc_context3(localCodec);
    assert(codecContext);

    auto paramsValue = avcodec_parameters_to_context(codecContext, codecParameters);
    assert(paramsValue >= 0);


    auto codecValue = avcodec_open2(codecContext, localCodec, NULL);
    assert(codecValue == 0);
    if (i == streamIndexs.video){
      for (int j = 0; j < 100; j++){
        readFrame(formatContext, avPacket, codecContext, avFrame, streamIndexs);
        bool keepReading = onFrame(avFrame);
        if (!keepReading){
          i = numStreams + 1;
          j = 101;
          break;
        }
      }
    }
    avcodec_free_context(&codecContext);
    av_packet_free(&avPacket);
    av_frame_free(&avFrame);
    std::cout << std::endl;
  }
  avformat_close_input(&formatContext);
  std::cout << "INFO: VIDEO: FINISHED VIDEO" << std::endl;
}

