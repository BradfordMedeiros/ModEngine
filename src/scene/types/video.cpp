#include "./video.h"

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

AVCodecContext* alloc_codec(AVFormatContext* formatContext, int index){
  AVCodecParameters* codecParameters = formatContext -> streams[index] -> codecpar;
  std::cout << "bit rate: " << codecParameters -> bit_rate << std::endl;
  std::cout << "width: " << codecParameters -> width << std::endl;
  std::cout << "height: " << codecParameters -> height << std::endl;
  AVCodec* localCodec = avcodec_find_decoder(codecParameters -> codec_id);
  assert(localCodec != NULL);
  std::cout << "INFO: video: found decoder for codec: " << codecParameters -> codec_id << std::endl;
  AVCodecContext *codecContext = avcodec_alloc_context3(localCodec);
  assert(codecContext);
  auto paramsValue = avcodec_parameters_to_context(codecContext, codecParameters);
  assert(paramsValue >= 0);
  auto codecValue = avcodec_open2(codecContext, localCodec, NULL);
  assert(codecValue == 0);
  return codecContext;
}

StreamCodecs getCodecs(AVFormatContext* formatContext, StreamIndexs& streamIndexs){
  StreamCodecs codecs {
    .videoCodec = alloc_codec(formatContext, streamIndexs.video),
    .audioCodec = alloc_codec(formatContext, streamIndexs.audio),
  };
  return codecs;
}
void freeCodecs(StreamCodecs& codecs){
  avcodec_free_context(&codecs.videoCodec);
  avcodec_free_context(&codecs.audioCodec);
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

void printAudioFrameInfo(AVFrame* avFrame,  AVCodecContext *audioCodec){
  std::cout  << "number of samples: " << avFrame -> nb_samples << std::endl;
  std::cout << "duration: " << avFrame -> pkt_duration << std::endl;
  std::cout << "timestamp: " << avFrame -> pkt_dts << std::endl;
  std::cout << "channels: " << avFrame -> channels << std::endl;
}

void readFrame(AVFormatContext* formatContext, AVPacket* avPacket, AVCodecContext* codecContext, AVCodecContext* audioCodec, AVFrame* avFrame, AVFrame* avFrame2, StreamIndexs& streams, AVPixelFormat destFormat){
  auto readValue = av_read_frame(formatContext, avPacket);
  if (readValue == 0 && avPacket -> stream_index == streams.video){
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
    assert(sendValue == 0);
    auto receiveValue = avcodec_receive_frame(codecContext, avFrame);
    if (receiveValue != 0){
      if (receiveValue == AVERROR_EOF){ // not actually an error
        std::cout << "ERROR: VIDEO: LAST FRAME" << std::endl;
      }
      assert(false);
    }

    auto swsContext = sws_getContext(avFrame -> width, avFrame -> height, codecContext -> pix_fmt, avFrame2 -> width, avFrame2 -> height, destFormat,  SWS_FAST_BILINEAR, NULL, NULL, NULL);
    assert(swsContext != NULL);
    sws_scale(swsContext, avFrame -> data, avFrame -> linesize, 0, avFrame -> height, avFrame2-> data, avFrame2 -> linesize);

    assert(avFrame2 -> width != 0);
    assert(avFrame2 -> height != 0);
    assert(avFrame2 -> linesize[0] != 0);
    //printFrameInfo(avFrame);
  }else if (avPacket -> stream_index == streams.audio){
    // move this to the top video decoding
    return;
    std::cout << "INFO: VIDEO: processing audio frame" << std::endl;

    auto sendValue = avcodec_send_packet(audioCodec, avPacket);
    if (sendValue ==  AVERROR_EOF){
      std::cout << "send packet: end of field" << std::endl;
    }else if (sendValue ==  AVERROR(EAGAIN)){
      std::cout << "send packet: averror(eagain)" << std::endl;
    }else if (sendValue == AVERROR(EINVAL)){
      std::cout << "send packet: averror(einval)" << std::endl;
    }else if (sendValue ==  AVERROR(ENOMEM)){
      std::cout << "send packet: averror(ENOMEM)" << std::endl;
    }
    assert(sendValue == 0);
    auto receiveValue = avcodec_receive_frame(audioCodec, avFrame);
    if (receiveValue != 0){
      if (receiveValue == AVERROR_EOF){ // not actually an error
        std::cout << "ERROR: AUDIO: LAST FRAME" << std::endl;
      }
      assert(false);
    }

    printAudioFrameInfo(avFrame, audioCodec);
  }else{
    std::cout << "INFO: video: READ error or end of video" << std::endl;
    assert(false);
  } 
  av_packet_unref(avPacket);
}

bool initialized = false;
VideoContent loadVideo(const char* videopath){
  if (!initialized){
    av_register_all();
    initialized = true;
  }

  // https://ffmpeg.org/doxygen/trunk/structAVFormatContext.html
  AVFormatContext* formatContext = avformat_alloc_context();
  if (!formatContext){
    std::cout << "ERROR: Could not allocate memory for avformatcontext" << std::endl;
    assert(false);
  }
  
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
  AVFrame* avFrame = av_frame_alloc();
  assert(avFrame);

  AVFrame* avFrame2 = av_frame_alloc();
  assert(avFrame2);

  auto format = AV_PIX_FMT_RGBA;

  // These determine the output scaling of AVFrame2 (see usage in readFrame)
  avFrame2 -> width = 200;
  avFrame2 -> height = 100;
  avFrame2 -> linesize[0] = 100;

  av_image_alloc(avFrame2 -> data, avFrame2 -> linesize, avFrame2 -> width, avFrame2 -> height,  format, 1); 

  auto streamIndexs = getStreamIndexs(formatContext);

  AVPacket *avPacket = av_packet_alloc();
  assert(avPacket);

  VideoContent videoContent {
    .formatContext = formatContext,
    .avFrame = avFrame,
    .avFrame2 = avFrame2,
    .avPacket = avPacket,
    .streamIndexs = streamIndexs,
    .codecs = getCodecs(formatContext, streamIndexs),
    .format = format,
  };
  std::cout << "video: " << videopath << " is: " << videoContent.formatContext -> duration << std::endl;
  std::cout << "number video streams: " << numStreams << std::endl;
  std::cout << "video index: " << videoContent.streamIndexs.video << ", audio index:" << videoContent.streamIndexs.audio << std::endl;
  nextFrame(videoContent);
  return videoContent;
}

AVFrame* nextFrame(VideoContent& content){
  readFrame(content.formatContext, content.avPacket, content.codecs.videoCodec, content.codecs.audioCodec, content.avFrame, content.avFrame2, content.streamIndexs, content.format);
  return content.avFrame;
}

void seekVideo(VideoContent& content){
  assert(false);
}
void pauseVideo(VideoContent& content){
  assert(false);
}
void resumeVideo(VideoContent& content){
  assert(false);
}

void freeVideoContent(VideoContent& content){
  av_packet_free(&content.avPacket);
  freeCodecs(content.codecs);
  av_frame_free(&content.avFrame);
  av_freep(content.avFrame2 -> data);
  av_frame_free(&content.avFrame2);
  avformat_close_input(&content.formatContext);
}