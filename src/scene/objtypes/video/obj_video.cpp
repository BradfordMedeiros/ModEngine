#include "./obj_video.h"

void updateTextureData(Texture& texture, unsigned char* data, int textureWidth, int textureHeight);
bool textureLoaded(std::string& texturepath);

std::vector<AutoSerialize> videoAutoserializer {
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectVideo, drawMesh),
    .field = "show",
    .onString = "enabled",
    .offString = "disabled",
    .defaultValue = false,
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObjectVideo, texturePath),
    .field = "texturepath",
    .defaultValue = "./res/texturevideotexture",
  },
  AutoSerializeString {
    .structOffset = offsetof(GameObjectVideo, video),
    .field = "video",
    .defaultValue = "../gameresources/video/bigbuck.webm",
  },
  AutoSerializeBool {
    .structOffset = offsetof(GameObjectVideo, playing),
    .field = "playing",
    .onString = "enabled",
    .offString = "disabled",
    .defaultValue = true,
  },
};

GameObjectVideo createVideoObj(GameobjAttributes& attr, ObjectTypeUtil& util){
	GameObjectVideo videoObj {};
  createAutoSerializeWithTextureLoading((char*)&videoObj, videoAutoserializer, attr, util);
	modassert(!textureLoaded(videoObj.texturePath), "texture already loaded for video");

	auto videoContent = loadVideo(videoObj.video.c_str());

  Texture texture = util.loadTextureData(
    videoObj.texturePath,
    videoContent.avFrame2 -> data[0], 
    videoContent.avFrame2 -> width, 
    videoContent.avFrame2 -> height, 
    4
  );

	videoObj.texture = texture;
	videoObj.videoContent = videoContent;
	videoObj.sound = createBufferedAudio();

	videoObj.playing = true;

	return videoObj;
}

void removeVideoObj(GameObjectVideo& videoObj, ObjectRemoveUtil& util){
	freeVideoContent(videoObj.videoContent);
	freeBufferedAudio(videoObj.sound);
}

std::vector<std::pair<std::string, std::string>> serializeVideo(GameObjectVideo& obj, ObjectSerializeUtil& util){
  std::vector<std::pair<std::string, std::string>> pairs;
  autoserializerSerialize((char*)&obj, videoAutoserializer, pairs);
  return pairs;
}

std::optional<AttributeValuePtr> getVideoAttribute(GameObjectVideo& obj, const char* field){
  return getAttributePtr((char*)&obj, videoAutoserializer, field);
}

bool setVideoAttribute(GameObjectVideo& obj, const char* field, AttributeValue value, ObjectSetAttribUtil& util, SetAttrFlags&){
  return autoserializerSetAttrWithTextureLoading((char*)&obj, videoAutoserializer, field, value, util);
}

void onVideoObjFrame(GameObjectVideo& videoObj, float currentTime, Transformation& viewTransform){
  setSoundPosition(videoObj.sound.source, viewTransform.position.x, viewTransform.position.y, viewTransform.position.z);

	std::cout << "onVideoObjFrame length: " << getVideoLength(videoObj) << std::endl;
  VideoContent& video = videoObj.videoContent;
  if (!videoObj.playing){
  	return;
  }

  std::cout << "video latestTimestamp: " << video.latestTimestamp << ", current time = " << currentTime << std::endl;

  if (!videoObj.startTime.has_value()){
    videoObj.startTime = currentTime;
  }

  bool shouldUpdateTexture = false;
  int numFramesDecoded = 0;
  while(true){ // probably should have a max iterations here 
    numFramesDecoded++;
    if (numFramesDecoded > 10){  // TODO relate this to framerate better
      break;
    }
    float timeIntoVideo = currentTime - videoObj.startTime.value();
    float timeDifference = timeIntoVideo - video.latestTimestamp;
    modlog("video lag", std::to_string(timeDifference));
    modlog("video desired", std::to_string(timeIntoVideo));
    modlog("video actual", std::to_string(video.latestTimestamp));

    if (video.latestTimestamp <= timeIntoVideo){
        // perhaps i should catch up if i'm running behind quicker
        bool videoEnd = false;
        int stream = nextFrame(video, &videoEnd);\
        modlog("video", "reading frame");
        if (videoEnd){
          //videoObj.playing = false;
          float time = 15.f;
          seekVideo(videoObj, time);
          videoObj.startTime = currentTime - time;
          videoObj.videoContent.latestTimestamp = time;
          videoObj.videoContent.audioTimestamp = time;

          modlog("video", "stopped playing");
          return;
        }

        if (stream == video.streamIndexs.video){
          shouldUpdateTexture = true;
        }else if (stream == video.streamIndexs.audio){
          auto audioCodec = video.codecs.audioCodec;
          auto bufferSize = av_samples_get_buffer_size(NULL, audioCodec -> channels, video.avFrame -> nb_samples, audioCodec -> sample_fmt, 0);
          auto numChannels = audioCodec -> channels;
          //// @TODO process all channels
          //// @TODO chandle more formats to eliminate assertion below 
          //// | int outputSamples = swr_convert(p_swrContext,  p_destBuffer, p_destLinesize,  (const uint8_t**)p_frame->extended_data, p_frame->nb_samples);
          if (video.avFrame -> nb_samples == 0){
            continue;
          }
          std::cout << "avFrame buffered audio: " << std::to_string(audioCodec -> sample_rate) << ", nb_samples = " << video.avFrame -> nb_samples << ", fmt = " << av_get_sample_fmt_name(audioCodec -> sample_fmt) << ", channels = " << numChannels <<  std::endl;
          //if (audioCodec -> sample_fmt != AV_SAMPLE_FMT_S16){
          //  const char* sample_fmt_str = av_get_sample_fmt_name(audioCodec -> sample_fmt);
          //  modassert(false, std::string("unexpected audio code format: ") + std::string(sample_fmt_str));
          //}
          playBufferedAudio(videoObj.sound, (void*)video.avFrame -> data[0], bufferSize, audioCodec -> sample_rate);
      }
    }else{
      break;
    }
  }

  modlog("video frames", std::to_string(numFramesDecoded));
  updateTextureData(videoObj.texture, video.avFrame2 -> data[0], video.avFrame2 -> width, video.avFrame2 -> height);
}

void seekVideo(GameObjectVideo& obj, float time){
	seekVideo(obj.videoContent, time);
}

float getVideoLength(GameObjectVideo& obj){
	if (obj.videoContent.formatContext -> duration == AV_NOPTS_VALUE){
		return 0.f;
	}
  int64_t duration_microseconds = obj.videoContent.formatContext -> duration;
  double duration_seconds = static_cast<double>(duration_microseconds) / AV_TIME_BASE;
	return duration_seconds;
}