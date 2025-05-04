#include "./obj_video.h"

void updateTextureData(Texture& texture, unsigned char* data, int textureWidth, int textureHeight);

std::vector<AutoSerialize> videoAutoserializer {
  //AutoSerializeBool {
  //  .structOffset = offsetof(GameObjectCamera, enableDof),
  //  .field = "dof",
  //  .onString = "enabled",
  //  .offString = "disabled",
  //  .defaultValue = false,
  //},

};

GameObjectVideo createVideoObj(GameobjAttributes& attr, ObjectTypeUtil& util){
  std::string videoPath = "../gameresources/video/bigbuck.webm";
	auto videoContent = loadVideo(videoPath.c_str());

  Texture texture = util.loadTextureData(
    "./res/texturevideotexture",
    videoContent.avFrame2 -> data[0], 
    videoContent.avFrame2 -> width, 
    videoContent.avFrame2 -> height, 
    4
  );

	return GameObjectVideo{
		.videoContent = videoContent,
		.sound = createBufferedAudio(),
		.texture = texture,
	};
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

void onVideoObjFrame(GameObjectVideo& videoObj, float currentTime){ // getTotalTimeGame()
  VideoContent& video = videoObj.videoContent;
  if (video.videoTimestamp < currentTime){
      // perhaps i should catch up if i'm running behind quicker
      int stream = nextFrame(video);
      if (stream == video.streamIndexs.video){
        updateTextureData( 
          videoObj.texture,
          video.avFrame2 -> data[0], 
          video.avFrame2 -> width, 
          video.avFrame2 -> height
        );
      }else if (stream == video.streamIndexs.audio){
          auto audioCodec = video.codecs.audioCodec;
          auto bufferSize = av_samples_get_buffer_size(NULL, audioCodec -> channels, video.avFrame -> nb_samples, audioCodec -> sample_fmt, 0);
          auto numChannels = audioCodec -> channels;
          //// @TODO process all channels
          //// @TODO chandle more formats to eliminate assertion below 
          //// | int outputSamples = swr_convert(p_swrContext,  p_destBuffer, p_destLinesize,  (const uint8_t**)p_frame->extended_data, p_frame->nb_samples);
          std::cout << "fmt name: " << av_get_sample_fmt_name(audioCodec -> sample_fmt) << std::endl;;
         // modassert(audioCodec -> sample_fmt == AV_SAMPLE_FMT_S16, "unexpected audio code format");
          playBufferedAudio(videoObj.sound, (uint8_t*)video.avFrame -> data[0], bufferSize, audioCodec -> sample_rate);
    }
  }
}